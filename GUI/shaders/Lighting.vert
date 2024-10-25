#version 460 core

layout(location = 0) in vec3 inPos;
layout(location = 1) in vec2 inTexCoords;

layout(location = 0) out vec2 outTexCoords;

void main() {
    outTexCoords = inTexCoords;
    gl_Position = vec4(inPos, 1.0);
}
