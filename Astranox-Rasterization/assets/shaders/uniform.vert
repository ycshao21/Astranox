#version 450 core

layout(binding = 0) uniform UniformBufferObject {
	mat4 model;
	mat4 view;
	mat4 projection;
} ubo;

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec4 a_Color;

layout(location = 0) out vec4 v_Color;

void main() {
    gl_Position = ubo.projection * ubo.view * ubo.model * vec4(a_Position, 1.0);
    v_Color = a_Color;
}
