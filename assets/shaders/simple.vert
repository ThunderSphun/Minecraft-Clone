#version 460

layout (location = 0) in vec3 a_position;
layout (location = 1) in vec4 a_color;
layout (location = 2) in vec2 a_texcoord;

uniform mat4 modelMatrix = mat4(1.0);
uniform mat4 viewMatrix = mat4(1.0);
uniform mat4 projectionMatrix = mat4(1.0);
uniform float time = 0;

out vec4 color;
out vec2 texCoord;

void main() {
	vec3 position = a_position;

	//position.y += sin(a_position.x * a_position.z * time) / 3;

	color = a_color;
	texCoord = a_texcoord;
	gl_Position = projectionMatrix * viewMatrix * modelMatrix  * vec4(position, 1);
}
