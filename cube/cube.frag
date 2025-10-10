#version 330 core

in vec3 v_normal;
in vec3 v_world_pos;

uniform vec3 u_light_dir;  // should be normalized
uniform vec3 u_color;

layout(location = 0) out vec4 frag_color;

void main() {
	vec3 N = normalize(v_normal);
	vec3 L = normalize(-u_light_dir); // assuming dir *toward* the surface
	float diff = max(dot(N, L), 0.0);
	vec3 color = u_color * diff;
	frag_color = vec4(color, 1.0);
}

