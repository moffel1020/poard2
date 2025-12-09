#version 450 core

layout(location = 0) in vec3 aPos;

layout(location = 0) out vec3 texCoord;

layout(location = 1) uniform mat4 view;
layout(location = 2) uniform mat4 proj;

void main() {
    texCoord = aPos;
    vec4 pos = proj * view * vec4(aPos, 1.0);
    // gl_Position = pos;
    gl_Position = pos.xyww;
}
