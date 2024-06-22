#version 460 core

layout(location = 0) out vec4 FragColor;

layout(location = 0) in vec3 inWorldPos;

uniform sampler2D equirectangularMap;

const vec2 invAtan = vec2(0.1591, 0.3183);
vec2 SampleSphericalMap(vec3 v) {
    vec2 uv = vec2(atan(v.z, v.x), asin(v.y));
    uv *= invAtan;
    uv += 0.5;
    return uv;
}

void main() {
    vec2 uv = SampleSphericalMap(normalize(inWorldPos));
    vec3 color = texture(equirectangularMap, uv).rgb;

    color = color / (color + vec3(1.0));    // HDR tonemapping
    FragColor = vec4(color, 1.0);
}
