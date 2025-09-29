#version 330 core

in vec3 in_pos;
in vec3 in_col;

out vec3 color;

void main(void) {
	gl_Position = vec4(in_pos, 1.0);
	color = in_col;
}
