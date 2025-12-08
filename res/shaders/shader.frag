#version 450 core

layout(location = 0) in vec3 position;
layout(location = 1) in vec2 texCoord;

layout(location = 0) out vec4 FragColor;

layout(binding = 0) uniform sampler2D rockTexture;

void main() {
    // vec3 col = position;
    // float v = col.y;
    // FragColor = vec4(v, v, v, 1.0);

    vec4 rockCol1 = texture(rockTexture, texCoord / 32);
    vec4 rockCol2 = texture(rockTexture, texCoord / 1024);
    vec4 col = mix(rockCol1, rockCol2, 0.5);

    col *= position.y;

    FragColor = col;

    // if (v < 0.35) {
    //     // water
    //     col.rgb = vec3(0.1, 0.1, 0.7);
    // } else if (v < 0.45) {
    //     // beach
    //     col.rgb = vec3(0.98, 0.92, 0.47);
    // } else if (v < 0.65) {
    //     // grass
    //     col.rgb = vec3(0.33, 0.8, 0.32);
    // } else {
    //     // stone
    //     col.rgb = vec3(0.33, 0.33, 0.33);
    // }
    // col *= (1.0 - v);
    // FragColor = vec4(col, 1.0);

}
