#version 450 core

out vec4 FragColor;

layout (location = 0) in vec2 texCoord;

layout (binding = 0) uniform sampler2D ourTexture;

void main() {
   FragColor = texture(ourTexture, texCoord);
}
