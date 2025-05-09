# cg_hw6_Q3 readme

## What you need
You need Visual Studio 2022 and window 11 OS.

And C/C++ should be available in VS2022.

## About
This project is about Phong Shading.

To view the result image, open Q3_result.png, 

and if you want an explanation of the code, scroll down below.

## How to run

1. Click code and download as zip file.
   
![image](https://github.com/user-attachments/assets/7afcd647-0e1a-4451-a316-2d0ec97c7674)



2. Unzip a download file

![image](https://github.com/user-attachments/assets/0abd2e1f-fbc0-4bbe-9e34-6639cf18e3cd)



3. Open cg_hw6_Q3-master. Double click cg_hw6_Q3-master and opne OpenglViewer.sln

![image](https://github.com/user-attachments/assets/8fb753d6-e097-4155-a83e-b58d85e332a1)




4. click "F5" on your keybord. Then you will get the result.

![image](https://github.com/user-attachments/assets/b1c9efa8-7538-4a84-a34a-dea56acdb5bf)



## Code explanation

I have only included explanations for the parts that were modified and added in HW5.

For other parts of the code, please refer to the HW5 README.

```
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
```
![image](https://github.com/user-attachments/assets/f4858440-856e-4c25-8725-80eb9d1d917e)

Implemented the compute_phong_lighting function to calculate lighting according to the instructions for HW6.

Only lightPos was hardcoded to fit the coordinate system of my code.

-------------


```

void rasterize_triangle(vec4 v0, vec4 v1, vec4 v2, vec3 n0, vec3 n1, vec3 n2) {

       ...

            if ((a >= 0 && b >= 0 && c >= 0) || (a <= 0 && b <= 0 && c <= 0)) {

                    ...
                    // Interpolated normal
                    vec3 normal = normalize(w0 * n0 + w1 * n1 + w2 * n2);

                    // Phong shading color at the pixel
                    vec3 color = compute_phong_lighting(vec3(p0), normal);                
                     ...
        }
    }
}

```
![image](https://github.com/user-attachments/assets/03b16ff6-e208-4aea-b7e0-f769dfaa79d8)


Phong Shading is implemented using per-fragment normal. Rasterization get normal.

Other code is same as Hw6_Q2 Gouraud Shading, and two attribute above was added.

-----------
```
void render_scene() {
   ...

    
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
```
![image](https://github.com/user-attachments/assets/3736402d-f590-4563-8fd4-590e9d33f592)

Computes the vertex normal.

Additional parameters were added to the rasterize_triangle

--------------
