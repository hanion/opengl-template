#include "file.c"
#include "glad.h"
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

const char *shader_type_as_cstr(GLuint shader) {
	switch (shader) {
		case GL_VERTEX_SHADER:   return "GL_VERTEX_SHADER";
		case GL_FRAGMENT_SHADER: return "GL_FRAGMENT_SHADER";
		default:                 return "(Unknown)";
	}
}

bool shader_compile_source(const GLchar *source, GLenum shader_type, GLuint *shader) {
	*shader = glCreateShader(shader_type);
	glShaderSource(*shader, 1, &source, NULL);
	glCompileShader(*shader);

	GLint compiled = 0;
	glGetShaderiv(*shader, GL_COMPILE_STATUS, &compiled);

	if (!compiled) {
		GLchar message[1024];
		GLsizei message_size = 0;
		glGetShaderInfoLog(*shader, sizeof(message), &message_size, message);
		fprintf(stderr, "[ERROR]: could not compile %s\n", shader_type_as_cstr(shader_type));
		fprintf(stderr, "%.*s\n", message_size, message);
		return false;
	}

	return true;
}

bool shader_compile_file(const char *file_path, GLenum shader_type, GLuint *shader) {
	char *source = read_entire_file(file_path);
	if (source == NULL) {
		fprintf(stderr, "[ERROR]: failed to read file `%s`: %s\n", file_path, strerror(errno));
		errno = 0;
		return false;
	}
	bool ok = shader_compile_source(source, shader_type, shader);
	if (!ok) {
		fprintf(stderr, "[ERROR]: failed to compile `%s` shader file\n", file_path);
	}
	free(source);
	return ok;
}

bool shader_link_program(GLuint vert_shader, GLuint frag_shader, GLuint *program) {
	*program = glCreateProgram();

	glAttachShader(*program, vert_shader);
	glAttachShader(*program, frag_shader);
	glLinkProgram(*program);

	GLint linked = 0;
	glGetProgramiv(*program, GL_LINK_STATUS, &linked);
	if (!linked) {
		GLsizei message_size = 0;
		GLchar message[1024];

		glGetProgramInfoLog(*program, sizeof(message), &message_size, message);
		fprintf(stderr, "[ERROR]: Program Linking: %.*s\n", message_size, message);
	}

	glDeleteShader(vert_shader);
	glDeleteShader(frag_shader);

	return program;
}

bool shader_load_program(const char *vertex_file_path, const char *fragment_file_path, GLuint *program) {
	GLuint vert = 0;
	if (!shader_compile_file(vertex_file_path, GL_VERTEX_SHADER, &vert)) {
		return false;
	}

	GLuint frag = 0;
	if (!shader_compile_file(fragment_file_path, GL_FRAGMENT_SHADER, &frag)) {
		return false;
	}

	if (!shader_link_program(vert, frag, program)) {
		return false;
	}

	return true;
}



