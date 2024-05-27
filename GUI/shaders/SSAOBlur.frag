#version 460 core

layout(location = 0) out float FragColor;

layout(location = 0) in vec2 inTexCoords;

uniform sampler2D ssaoMap;

void main() {
    vec2 texelSize = 1.0 / vec2(textureSize(ssaoMap, 0));
    float result = 0.0;

    for (int x = -2; x < 2; ++x) {
        for (int y = -2; y < 2; ++y) {
            vec2 offset = vec2(float(x), float(y)) * texelSize;
            result += texture(ssaoMap, inTexCoords + offset).r;
        }
    }

    FragColor = result / (4.0 * 4.0);
}
