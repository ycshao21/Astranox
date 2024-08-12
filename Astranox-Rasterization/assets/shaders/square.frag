#version 450 core

// layout(binding = 1) uniform sampler2D u_Texture;

layout(location = 0) in vec4 v_Color;
layout(location = 1) in vec2 v_TexCoord;

layout(location = 0) out vec4 outColor;

void main() {
    // vec4 tex = texture(u_Texture, v_TexCoord);
    // outColor = vec4(v_Color * tex);
    outColor = v_Color;
}

