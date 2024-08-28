#type vertex
#version 450 core

layout(set = 0, binding = 0) uniform CameraData {
	mat4 viewProjection;
} u_Camera;


layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec4 a_Color;
layout(location = 2) in vec2 a_TexCoord;
layout(location = 3) in float a_TexIndex;
layout(location = 4) in float a_TilingFactor;

struct VertexOutput {
	vec4 color;
	vec2 texCoord;
	float texIndex;
	float tilingFactor;
};

layout(location = 0) out VertexOutput vertOut;

void main() {
    gl_Position = u_Camera.viewProjection * vec4(a_Position, 1.0);
	vertOut.color = a_Color;
	vertOut.texCoord = a_TexCoord;
	vertOut.texIndex = a_TexIndex;
	vertOut.tilingFactor = a_TilingFactor;
}


#type fragment
#version 450 core

layout(set = 0, binding = 1) uniform sampler2D u_Textures[32];

struct VertexOutput {
	vec4 color;
	vec2 texCoord;
	float texIndex;
	float tilingFactor;
};

layout(location = 0) in VertexOutput vertIn;

layout(location = 0) out vec4 o_Color;


void main() {
    o_Color = texture(u_Textures[int(vertIn.texIndex)], vertIn.texCoord * vertIn.tilingFactor) * vertIn.color;
}


