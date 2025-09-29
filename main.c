#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define GLAD_GL_IMPLEMENTATION
#include "glad.h"
#define GLFW_INCLUDE_GLEXT
#include <GLFW/glfw3.h>
#include <GL/glext.h>

#define SCREEN_WIDTH  800
#define SCREEN_HEIGHT 600
#define ENABLE_VSYNC 1

char *read_entire_file(const char *file_path) {
	FILE *f = NULL;
	char *buffer = NULL;

	f = fopen(file_path, "r");
	if (f == NULL) goto fail;
	if (fseek(f, 0, SEEK_END) < 0) goto fail;

	long size = ftell(f);
	if (size < 0) goto fail;

	buffer = malloc(size + 1);
	if (buffer == NULL) goto fail;

	if (fseek(f, 0, SEEK_SET) < 0) goto fail;

	fread(buffer, 1, size, f);
	if (ferror(f)) goto fail;

	buffer[size] = '\0';

	if (f) {
		fclose(f);
		errno = 0;
	}
	return buffer;
fail:
	if (f) {
		int saved_errno = errno;
		fclose(f);
		errno = saved_errno;
	}
	if (buffer) {
		free(buffer);
	}
	return NULL;
}

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



void error_callback(int error, const char* description) {
	fprintf(stderr, "[ERROR]: %s\n", description);
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
	if (action == GLFW_PRESS) {
		if (key == GLFW_KEY_ESCAPE || key == GLFW_KEY_CAPS_LOCK) {
			glfwSetWindowShouldClose(window, GL_TRUE);
		}
	}
}

void window_size_callback(GLFWwindow* window, int width, int height) {
	glViewport(0, 0, width, height);
	glActiveTexture(GL_TEXTURE1);
	//glBindTexture(GL_TEXTURE_2D, texture);
	glTexImage2D(GL_TEXTURE_2D, 0,
			  GL_RGBA, width, height, 0,
			  GL_RGBA, GL_UNSIGNED_BYTE, NULL);
}


void message_callback(GLenum source, GLenum type, GLuint id, GLenum severity,
					  GLsizei length, const GLchar* message, const void* userParam) {
	(void) source;
	(void) id;
	(void) length;
	(void) userParam;
	fprintf(stderr, "[message_callback]: %s type = 0x%x, severity = 0x%x, message = %s\n",
		 (type == GL_DEBUG_TYPE_ERROR ? "** GL ERROR **" : ""),
		 type, severity, message);
}





int main(void) {
    glfwSetErrorCallback(error_callback);

	if (!glfwInit()) {
		fprintf(stderr, "[ERROR]: could not initialize GLFW\n");
		exit(1);
	}

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);

	GLFWwindow * const window = glfwCreateWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "opengl_template", NULL, NULL);
	if (window == NULL) {
		fprintf(stderr, "[ERROR]: could not create a window.\n");
		glfwTerminate();
		exit(1);
	}

    glfwMakeContextCurrent(window);
	gladLoadGL(glfwGetProcAddress);
	glfwSwapInterval(ENABLE_VSYNC);

	printf("OpenGL renderer: %s\n", glGetString(GL_RENDERER));
	printf("OpenGL version:  %s\n", glGetString(GL_VERSION));



#if ENABLE_GL_MESSAGE_CALLBACK
	if (glDebugMessageCallbackARB != NULL) {
		glEnable(GL_DEBUG_OUTPUT);
		glDebugMessageCallbackARB(message_callback, 0);
	}
#endif // ENABLE_GL_MESSAGE_CALLBACK
	glfwSetFramebufferSizeCallback(window, window_size_callback);
	glfwSetKeyCallback(window, key_callback);


	GLuint program = 0;
	if (!shader_load_program("shader.vert", "shader.frag", &program)) {
		fprintf(stderr, "[ERROR]: could not load shader program.\n");
		glfwTerminate();
		exit(1);
	}




	typedef struct {
		float x,y,z;
	} V3f;
	typedef struct {
		V3f pos;
		V3f col;
	} Vertex;

	static const Vertex vertices[3] = {
		{ .pos = { -0.5f, -0.5f, 0.0f }, .col = { 1.0f, 0.0f, 0.0f } },
		{ .pos = {  0.5f, -0.5f, 0.0f }, .col = { 0.0f, 1.0f, 0.0f } },
		{ .pos = {  0.0f,  0.5f, 0.0f }, .col = { 0.0f, 0.0f, 1.0f } }
	};

	GLuint vertex_buffer;
	glGenBuffers(1, &vertex_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    const GLint in_pos_location = glGetAttribLocation(program, "in_pos");
    const GLint in_col_location = glGetAttribLocation(program, "in_col");

	GLuint vertex_array;
	glGenVertexArrays(1, &vertex_array);
	glBindVertexArray(vertex_array);
	glEnableVertexAttribArray(in_pos_location);
	glVertexAttribPointer(in_pos_location, 3, GL_FLOAT, GL_FALSE,
					   sizeof(Vertex), (void*)offsetof(Vertex, pos));
	glEnableVertexAttribArray(in_col_location);
	glVertexAttribPointer(in_col_location, 3, GL_FLOAT, GL_FALSE,
					   sizeof(Vertex), (void*)offsetof(Vertex, col));



	double time = glfwGetTime();
	double prev_time = 0.0;
	double delta_time = 0.0f;
	while (!glfwWindowShouldClose(window)) {
		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		// draw triangle
		glUseProgram(program);
        glBindVertexArray(vertex_array);
		glDrawArrays(GL_TRIANGLES, 0, 3);

		glfwSwapBuffers(window);
		glfwPollEvents();

		double cur_time = glfwGetTime();
		delta_time = cur_time - prev_time;
		time += delta_time;
		prev_time = cur_time;
	}

    glfwDestroyWindow(window);
	glfwTerminate();
	return 0;
}
