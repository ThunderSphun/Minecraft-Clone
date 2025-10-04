#version 460

uniform float time = 0;

in vec4 color;
out vec4 fragColor;

void main() {
	vec4 c = color;

	c *= (sin(time) + 1.) / 2;

	fragColor = c;
}
