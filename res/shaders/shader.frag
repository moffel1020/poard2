#version 460 core

out vec4 FragColor;

layout (location = 0) in vec2 texCoord;

uniform sampler2D ourTexture;

void main() {
   FragColor = texture(ourTexture, texCoord);
}
