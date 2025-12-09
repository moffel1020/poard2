#version 450 core

layout(location = 0) in vec3 position;
layout(location = 1) in vec2 texCoord;

layout(location = 0) out vec4 FragColor;

layout(binding = 0) uniform sampler2D rockTexture;
layout(binding = 1) uniform sampler2D grassTexture;

layout(location = 5) uniform vec3 camPos;
layout(location = 6) uniform vec2 fogDistance;

// Fog parameters, could make them uniforms and pass them into the fragment shader
vec4 applyFog(in vec4 color) {
    float maxDist = fogDistance.y;
    float minDist = fogDistance.x;
    vec4  fogColor = vec4(0.9, 0.8, 0.7, 1.0);

    float dist = length(camPos - position);
    float factor = (maxDist - dist) / (maxDist - minDist);
    factor = clamp(factor, 0.0, 1.0);

    return mix(fogColor, color, factor);
}

void main() {
    vec4 grassCol = texture(grassTexture, texCoord / 1024);
    vec4 rockCol = texture(rockTexture, texCoord / 512);
    vec4 col = mix(grassCol, rockCol, 1 - position.y);

    col *= position.y;
    col = applyFog(col);

    FragColor = col;
}
