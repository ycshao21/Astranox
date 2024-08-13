#version 450 core

layout(binding = 0) uniform CameraData {
	mat4 viewProjection;
} u_Camera;

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec4 a_Color;

layout(location = 0) out vec4 v_Color;

void main() {
    gl_Position = u_Camera.viewProjection * vec4(a_Position, 1.0);
    v_Color = a_Color;
}

