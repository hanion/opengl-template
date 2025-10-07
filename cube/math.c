#include <math.h>
#include <stddef.h>
#include <stdint.h>
#include "glad.h"

typedef struct {
	float x,y,z;
} V3f;

typedef struct {
	float x,y,z,w;
} V4f;

typedef struct {
	float m[4][4];
} M4f;

typedef struct {
	V3f position, rotation,scale;
} Transform;

typedef struct {
	V3f pos;
} Vertex;

typedef uint32_t Index;
typedef struct {
	Vertex* vertices;
	Index* indices;
	size_t vertices_count;
	size_t indices_count;
	GLuint vao, vbo, ebo;
	GLint mvp_location, color_location;
} Mesh;


typedef struct {
	Transform transform;
	M4f view_projection_matrix;
	M4f perspective_projection;
	float fov, aspect;
} Camera;

V4f v4f_add(V4f a, V4f b) {
	V4f r;
	r.x = a.x + b.x;
	r.y = a.y + b.y;
	r.z = a.z + b.z;
	r.w = a.w + b.w;
	return r;
}


V4f v4f_mulf(V4f v, float f) {
	V4f r;
	r.x = v.x * f;
	r.y = v.y * f;
	r.z = v.z * f;
	r.w = v.w * f;
	return r;
}

M4f m4f_identity(void) {
	M4f r = {0};
	r.m[0][0] = 1.0f;
	r.m[1][1] = 1.0f;
	r.m[2][2] = 1.0f;
	r.m[3][3] = 1.0f;
	return r;
}
M4f m4f_mul_m4f(M4f a, M4f b) {
	M4f r = {0};
	for (int i = 0; i < 4; ++i)
		for (int j = 0; j < 4; ++j)
			for (int k = 0; k < 4; ++k)
				r.m[i][j] += a.m[i][k] * b.m[k][j];
	return r;
}

V4f m4f_mul_vec4(const M4f *m, const V4f *v) {
	V4f result;
	result.x = m->m[0][0] * v->x + m->m[0][1] * v->y + m->m[0][2] * v->z + m->m[0][3] * v->w;
	result.y = m->m[1][0] * v->x + m->m[1][1] * v->y + m->m[1][2] * v->z + m->m[1][3] * v->w;
	result.z = m->m[2][0] * v->x + m->m[2][1] * v->y + m->m[2][2] * v->z + m->m[2][3] * v->w;
	result.w = m->m[3][0] * v->x + m->m[3][1] * v->y + m->m[3][2] * v->z + m->m[3][3] * v->w;
	return result;
}


M4f m4f_translate(V3f v) {
	M4f r = m4f_identity();
	r.m[3][0] = v.x;
	r.m[3][1] = v.y;
	r.m[3][2] = v.z;
	return r;
}

M4f m4f_rotate_y(float angle) {
	M4f r = m4f_identity();
	float c = cosf(angle);
	float s = sinf(angle);
	r.m[0][0] =  c;
	r.m[0][2] =  s;
	r.m[2][0] = -s;
	r.m[2][2] =  c;
	return r;
}

M4f m4f_make_perspective(float fov_rad, float aspect, float znear, float zfar) {
	M4f m = {0};
	float f = 1.0f / tanf(fov_rad / 2.0f);

	m.m[0][0] = f / aspect;
	m.m[1][1] = f;
	m.m[2][2] = -(zfar + znear) / (zfar - znear);
	m.m[2][3] = -1.0f;
	m.m[3][2] = -(2.0f * zfar * znear) / (zfar - znear);
	m.m[3][3] = 0.0f;

	return m;
}

V4f m4f_mul_vec4_project(M4f* mat_proj, V4f* v) {
	V4f result = m4f_mul_vec4(mat_proj, v);

	if (result.w != 0.0) {
		result.x /= result.w;
		result.y /= result.w;
		result.z /= result.w;
	}
	return result;
}




M4f get_translation_matrix(float tx, float ty, float tz) {
	M4f mat = {{
		{1, 0, 0, tx},
		{0, 1, 0, ty},
		{0, 0, 1, tz},
		{0, 0, 0, 1}
	}};
	return mat;
}
M4f get_rotation_matrix_x(float angle) {
	M4f mat = {{
		{1, 0, 0, 0},
		{0, cos(angle), -sin(angle), 0},
		{0, sin(angle), cos(angle), 0},
		{0, 0, 0, 1}
	}};
	return mat;
}

M4f get_rotation_matrix_y(float angle) {
	M4f mat = {{
		{cos(angle), 0, sin(angle), 0},
		{0, 1, 0, 0},
		{-sin(angle), 0, cos(angle), 0},
		{0, 0, 0, 1}
	}};
	return mat;
}

M4f get_rotation_matrix_z(float angle) {
	M4f mat = {{
		{cos(angle), -sin(angle), 0, 0},
		{sin(angle), cos(angle), 0, 0},
		{0, 0, 1, 0},
		{0, 0, 0, 1}
	}};
	return mat;
}
M4f get_scaling_matrix(float sx, float sy, float sz) {
	M4f mat = {{
		{sx, 0, 0, 0},
		{0, sy, 0, 0},
		{0, 0, sz, 0},
		{0, 0, 0, 1}
	}};
	return mat;
}


M4f get_view_matrix(float cam_x, float cam_y, float cam_z) {
	return get_translation_matrix(-cam_x, -cam_y, -cam_z);
}



M4f calculate_transform_matrix(const Transform *transform) {
	M4f rotation_x = get_rotation_matrix_x(transform->rotation.x);
	M4f rotation_y = get_rotation_matrix_y(transform->rotation.y);
	M4f rotation_z = get_rotation_matrix_z(transform->rotation.z);
	M4f rotation_combined = m4f_mul_m4f(rotation_z, rotation_y);
	rotation_combined = m4f_mul_m4f(rotation_combined, rotation_x);

	M4f scaling = get_scaling_matrix(transform->scale.x, transform->scale.y, transform->scale.z);

	M4f translation = get_translation_matrix(transform->position.x, transform->position.y, transform->position.z);

	M4f transform_matrix;
	transform_matrix = m4f_mul_m4f(scaling, rotation_combined);
	transform_matrix = m4f_mul_m4f(translation, transform_matrix);
	return transform_matrix;
}


M4f calculate_view_matrix(const Transform *transform) {
	M4f rotation_x = get_rotation_matrix_x(-transform->rotation.x);
	M4f rotation_y = get_rotation_matrix_y(-transform->rotation.y);
	M4f rotation_z = get_rotation_matrix_z(-transform->rotation.z);
	M4f rotation_combined = m4f_mul_m4f(rotation_z, m4f_mul_m4f(rotation_y, rotation_x));

	M4f inverse_translation = get_translation_matrix(-transform->position.x, -transform->position.y, -transform->position.z);
	M4f view_matrix = m4f_mul_m4f(rotation_combined, inverse_translation);
	return view_matrix;
}



V3f normalize(V3f v) {
	float length = sqrt(v.x * v.x + v.y * v.y + v.z * v.z);

	if (length == 0) {
		return (V3f){0, 0, 0};
	}

	return (V3f){v.x / length, v.y / length, v.z / length};
}


V3f v3f_cross(V3f a, V3f b) {
	V3f result;
	result.x = a.y * b.z - a.z * b.y;
	result.y = a.z * b.x - a.x * b.z;
	result.z = a.x * b.y - a.y * b.x;
	return result;
}

float v3f_dot(V3f a, V3f b) {
	return a.x * b.x + a.y * b.y + a.z * b.z;
}




void camera_update(Camera* c) {
	c->perspective_projection = m4f_make_perspective(c->fov * ((3.14159265f) / 180.0f), c->aspect, 1.0f, 100.0f);
	M4f view_matrix = calculate_view_matrix(&c->transform);
	c->view_projection_matrix = m4f_mul_m4f(c->perspective_projection, view_matrix);
}




