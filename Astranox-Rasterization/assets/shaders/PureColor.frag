#version 450 core

layout(location = 0) in vec4 v_Color;
layout(location = 1) in vec2 v_TexCoord;

layout(location = 0) out vec4 outColor;


void main() {
    outColor = v_Color;
}

