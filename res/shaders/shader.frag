#version 450 core

layout(location = 0) in vec2 texCoord;

layout(location = 0) out vec4 FragColor;

layout(binding = 0) uniform sampler2D myTexture;

void main() {
    vec4 col = texture(myTexture, texCoord);
    float v = col.r;

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

    col = vec4(v, v, v, 1.0);

    FragColor = col;
}
