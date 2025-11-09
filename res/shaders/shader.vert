#version 450 core

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec2 aTexCoord;

layout(location = 0) out vec3 position;

uniform mat4 model;
uniform mat4 view;
uniform mat4 proj;

void main() {
    vec3 vertPos = aPos;
    vertPos.y *= 200;
    
    gl_Position = proj * view * model * vec4(vertPos, 1.0);
    position = aPos;
}
