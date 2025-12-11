#version 450 core
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec3 aColor;

uniform mat4 model;
uniform mat4 view;
uniform mat4 proj;
uniform vec3 u_color;
uniform int u_useUniformColor;

out vec3 vColor;

void main()
{
    if (u_useUniformColor == 1) {
        vColor = u_color;
    } else {
        vColor = aColor;
    }
    gl_Position = proj * view * model * vec4(aPos, 1.0);
}
