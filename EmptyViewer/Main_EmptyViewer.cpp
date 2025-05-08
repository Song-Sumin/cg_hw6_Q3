#define _USE_MATH_DEFINES
#include <Windows.h>
#include <iostream>
#include <GL/glew.h>
#include <GL/GL.h>
#include <GL/freeglut.h>

#define GLFW_INCLUDE_GLU
#define GLFW_DLL
#include <GLFW/glfw3.h>
#include <vector>

#define GLM_SWIZZLE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/string_cast.hpp>

#include <iostream>
#include <vector>
#include <cmath>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <GLFW/glfw3.h>

using namespace glm;

const int WIDTH = 512;
const int HEIGHT = 512;

std::vector<vec3> gVertices;
std::vector<int> gIndexBuffer;
std::vector<float> DepthBuffer(WIDTH* HEIGHT, 1e9f);
std::vector<float> OutputImage(WIDTH* HEIGHT * 3, 0.0f);

void create_scene() {
    int width = 32;
    int height = 16;
    float theta, phi;

    gVertices.resize((height - 2) * width + 2);
    gIndexBuffer.resize((height - 2) * (width - 1) * 6 + 6 * (width - 1));

    int t = 0;
    for (int j = 1; j < height - 1; ++j) {
        for (int i = 0; i < width; ++i) {
            theta = (float)j / (height - 1) * M_PI;
            phi = (float)i / (width - 1) * 2 * M_PI;
            float x = sinf(theta) * cosf(phi);
            float y = cosf(theta);
            float z = -sinf(theta) * sinf(phi);
            gVertices[t++] = vec3(x, y, z);
        }
    }
    gVertices[t++] = vec3(0, 1, 0);
    gVertices[t++] = vec3(0, -1, 0);

    t = 0;
    for (int j = 0; j < height - 3; ++j) {
        for (int i = 0; i < width - 1; ++i) {
            gIndexBuffer[t++] = j * width + i;
            gIndexBuffer[t++] = (j + 1) * width + (i + 1);
            gIndexBuffer[t++] = j * width + (i + 1);
            gIndexBuffer[t++] = j * width + i;
            gIndexBuffer[t++] = (j + 1) * width + i;
            gIndexBuffer[t++] = (j + 1) * width + (i + 1);
        }
    }
    for (int i = 0; i < width - 1; ++i) {
        gIndexBuffer[t++] = (height - 2) * width;
        gIndexBuffer[t++] = i;
        gIndexBuffer[t++] = i + 1;
        gIndexBuffer[t++] = (height - 2) * width + 1;
        gIndexBuffer[t++] = (height - 3) * width + (i + 1);
        gIndexBuffer[t++] = (height - 3) * width + i;
    }
}

vec3 compute_phong_lighting(vec3 pos, vec3 normal) {
    // Material properties
    vec3 ka(0.0f, 1.0f, 0.0f);
    vec3 kd(0.0f, 0.5f, 0.0f);
    vec3 ks(0.5f, 0.5f, 0.5f);
    float p = 32.0f;

    // Light properties
    vec3 lightPos(4.0f, -4.0f, 3.0f);
    vec3 viewPos(0.0f, 0.0f, 0.0f);
    float Ia = 0.2f;

    // Lighting calculations
    vec3 L = normalize(lightPos - pos);
    vec3 V = normalize(viewPos - pos);
    vec3 R = reflect(-L, normal);

    vec3 ambient = Ia * ka;
    vec3 diffuse = kd * std::max(dot(normal, L), 0.0f);
    vec3 specular = ks * pow(std::max(dot(R, V), 0.0f), p);

    vec3 color = ambient + diffuse + specular;

    // Gamma correction
    color.r = pow(color.r, 1.0f / 2.2f);
    color.g = pow(color.g, 1.0f / 2.2f);
    color.b = pow(color.b, 1.0f / 2.2f);

    return clamp(color, 0.0f, 1.0f);
}

void rasterize_triangle(vec4 v0, vec4 v1, vec4 v2, vec3 n0, vec3 n1, vec3 n2) {
    // Homogeneous divide
    vec3 p0 = vec3(v0) / v0.w;
    vec3 p1 = vec3(v1) / v1.w;
    vec3 p2 = vec3(v2) / v2.w;

    // Viewport transform
    auto to_screen = [](vec3 p) -> vec2 {
        return vec2((p.x + 0.5f) * WIDTH, (p.y + 0.5f) * HEIGHT);
        };

    vec2 s0 = to_screen(p0);
    vec2 s1 = to_screen(p1);
    vec2 s2 = to_screen(p2);

    float minX = std::max(0.0f, floor(std::min({ s0.x, s1.x, s2.x })));
    float maxX = std::min((float)WIDTH - 1, ceil(std::max({ s0.x, s1.x, s2.x })));
    float minY = std::max(0.0f, floor(std::min({ s0.y, s1.y, s2.y })));
    float maxY = std::min((float)HEIGHT - 1, ceil(std::max({ s0.y, s1.y, s2.y })));

    for (int y = (int)minY; y <= (int)maxY; ++y) {
        for (int x = (int)minX; x <= (int)maxX; ++x) {
            vec2 p(x + 0.5f, y + 0.5f);
            vec2 v0v1 = s1 - s0, v0p = p - s0;
            vec2 v1v2 = s2 - s1, v1p = p - s1;
            vec2 v2v0 = s0 - s2, v2p = p - s2;

            float a = v0v1.x * v0p.y - v0v1.y * v0p.x;
            float b = v1v2.x * v1p.y - v1v2.y * v1p.x;
            float c = v2v0.x * v2p.y - v2v0.y * v2p.x;

            if ((a >= 0 && b >= 0 && c >= 0) || (a <= 0 && b <= 0 && c <= 0)) {
                // Barycentric interpolation
                float area = std::abs((s1.x - s0.x) * (s2.y - s0.y) - (s2.x - s0.x) * (s1.y - s0.y));
                float w0 = std::abs((s1.x - p.x) * (s2.y - p.y) - (s2.x - p.x) * (s1.y - p.y)) / area;
                float w1 = std::abs((s2.x - p.x) * (s0.y - p.y) - (s0.x - p.x) * (s2.y - p.y)) / area;
                float w2 = 1.0f - w0 - w1;

                // Depth interpolation
                float depth = w0 * p0.z + w1 * p1.z + w2 * p2.z;
                int idx = y * WIDTH + x;
                if (depth < DepthBuffer[idx]) {
                    DepthBuffer[idx] = depth;

                    // Interpolated normal
                    vec3 normal = normalize(w0 * n0 + w1 * n1 + w2 * n2);

                    // Phong shading color at the pixel
                    vec3 color = compute_phong_lighting(vec3(p0), normal);

                    OutputImage[3 * idx + 0] = color.r;
                    OutputImage[3 * idx + 1] = color.g;
                    OutputImage[3 * idx + 2] = color.b;
                }
            }
        }
    }
}

void render_scene() {
    create_scene();

    mat4 model = glm::scale(mat4(1.0f), vec3(2.0f));
    model = glm::translate(model, vec3(0, 0, -7));

    mat4 view = lookAt(vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 0.0f, -1.0f), vec3(0.0f, 1.0f, 0.0f));
    mat4 persp = glm::frustum(-0.1f, 0.1f, -0.1f, 0.1f, -0.1f, -1000.0f);
    mat4 ortho = glm::scale(mat4(1.0f), vec3(1, 1, -1));
    mat4 MVP = ortho * persp * view * model;

    for (int i = 0; i < gIndexBuffer.size(); i += 3) {
        int i0 = gIndexBuffer[i];
        int i1 = gIndexBuffer[i + 1];
        int i2 = gIndexBuffer[i + 2];

        vec3 p0 = gVertices[i0];
        vec3 p1 = gVertices[i1];
        vec3 p2 = gVertices[i2];

        vec3 n0 = normalize(p0);  // Sphere: position = normal
        vec3 n1 = normalize(p1);
        vec3 n2 = normalize(p2);

        vec4 v0 = MVP * vec4(p0, 1.0f);
        vec4 v1 = MVP * vec4(p1, 1.0f);
        vec4 v2 = MVP * vec4(p2, 1.0f);

        rasterize_triangle(v0, v1, v2, n0, n1, n2);
    }
}

int main() {
    if (!glfwInit()) return -1;
    GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "Software Rasterizer with Phong Shading", NULL, NULL);
    if (!window) return -1;
    glfwMakeContextCurrent(window);

    render_scene();

    while (!glfwWindowShouldClose(window)) {
        glClear(GL_COLOR_BUFFER_BIT);
        glDrawPixels(WIDTH, HEIGHT, GL_RGB, GL_FLOAT, &OutputImage[0]);
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
