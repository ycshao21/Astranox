#version 450 core

layout(set = 0, binding = 1) uniform sampler2D u_Textures[32];

layout(location = 0) in vec4 v_Color;
layout(location = 1) in vec2 v_TexCoord;
layout(location = 2) in float v_TexIndex;
layout(location = 3) in float v_TilingFactor;

layout(location = 0) out vec4 outColor;


void main() {
    outColor = texture(u_Textures[int(v_TexIndex)], v_TexCoord * v_TilingFactor) * v_Color;
}

