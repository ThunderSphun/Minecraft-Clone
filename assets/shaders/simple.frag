#version 460

uniform float time = 0;
uniform sampler2D s_texture;

in vec4 color;
in vec2 texCoord;

out vec4 fragColor;

void main() {
	vec4 c = color;

	c *= texture(s_texture, texCoord);
	//c *= (sin(time) / 4) + 0.75;

	fragColor = c;
}
