#version 460

uniform float time = 0;

in vec3 color;
out vec3 fragColor;

void main() {
	vec3 c = color;

	c *= .5 * (sin(time) + 1.);

	fragColor = c;
}
