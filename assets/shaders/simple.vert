#version 460

layout (location = 0) in vec3 a_position;
layout (location = 1) in vec3 a_color;

uniform mat4 mvpMatrix;
uniform float time = 0;

out vec3 color;

void main() {
	vec3 position = a_position;

	position.y += sin(a_position.x * a_position.z * time / 3);

	color = a_color;
	gl_Position = mvpMatrix * vec4(position, 1);
}
