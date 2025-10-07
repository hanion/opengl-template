#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>


#include "math.c"
#include "shader.c"

#define GLAD_GL_IMPLEMENTATION
#include "glad.h"
#define GLFW_INCLUDE_GLEXT
#include <GLFW/glfw3.h>
#include <GL/glext.h>

#define SCREEN_WIDTH  800
#define SCREEN_HEIGHT 600
#define ENABLE_VSYNC 1

static double global_scroll_y;

void draw_mesh(Mesh* mesh, Transform* transform, Camera* camera) {
	M4f model = calculate_transform_matrix(transform);
	M4f mvp = m4f_mul_m4f(camera->view_projection_matrix, model);
	glUniformMatrix4fv(mesh->mvp_location, 1, GL_TRUE, &mvp.m[0][0]);
	glBindVertexArray(mesh->vao);
	glUniform3f(mesh->color_location, 0.8f, 0.2f, 0.2f);
	glDrawElements(GL_TRIANGLES, mesh->indices_count, GL_UNSIGNED_INT, 0);
	//glDrawElements(GL_POINTS, mesh.indices_count, GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);
}

Mesh cube_generate_mesh() {
	static const Vertex vertices[8] = {
		{ .pos = {-0.5f, -0.5f, -0.5f} },
		{ .pos = { 0.5f, -0.5f, -0.5f} },
		{ .pos = { 0.5f,  0.5f, -0.5f} },
		{ .pos = {-0.5f,  0.5f, -0.5f} },
		{ .pos = {-0.5f, -0.5f,  0.5f} },
		{ .pos = { 0.5f, -0.5f,  0.5f} },
		{ .pos = { 0.5f,  0.5f,  0.5f} },
		{ .pos = {-0.5f,  0.5f,  0.5f} }
	};
	static const Index indices[36] = {
		0, 1, 2, 2, 3, 0,
		5, 4, 7, 7, 6, 5,
		4, 0, 3, 3, 7, 4,
		1, 5, 6, 6, 2, 1,
		3, 2, 6, 6, 7, 3,
		4, 5, 1, 1, 0, 4
	};
	Mesh cube_mesh = {0};
	cube_mesh.vertices = (Vertex*)vertices;
	cube_mesh.indices = (Index*)indices;
	cube_mesh.vertices_count = sizeof(vertices) / sizeof(Vertex);
	cube_mesh.indices_count = sizeof(indices) / sizeof(Index);
	return cube_mesh;
}

void mesh_init(Mesh* mesh, GLuint program) {
	glGenVertexArrays(1, &mesh->vao);
	glBindVertexArray(mesh->vao);

	glGenBuffers(1, &mesh->vbo);
	glBindBuffer(GL_ARRAY_BUFFER, mesh->vbo);
	glBufferData(GL_ARRAY_BUFFER, mesh->vertices_count * sizeof(Vertex), mesh->vertices, GL_STATIC_DRAW);

	glGenBuffers(1, &mesh->ebo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, mesh->indices_count * sizeof(Index), mesh->indices, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, pos));
	glEnableVertexAttribArray(0);
	//glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, col));
	//glEnableVertexAttribArray(1);
	mesh->mvp_location   = glGetUniformLocation(program, "mvp");
	mesh->color_location = glGetUniformLocation(program, "u_color");
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

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
	global_scroll_y += yoffset;
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
	glfwSetScrollCallback(window, scroll_callback);


	GLuint program = 0;
	if (!shader_load_program("cube.vert", "cube.frag", &program)) {
		fprintf(stderr, "[ERROR]: could not load shader program.\n");
		glfwTerminate();
		exit(1);
	}



	Mesh cube_mesh = cube_generate_mesh();
	mesh_init(&cube_mesh, program);

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glFrontFace(GL_CW);


	Camera cam = {0};
	cam.transform.position.z = 2,
	cam.transform.scale = (V3f){1,1,1};
	cam.fov = 50;
	cam.aspect = (float)SCREEN_WIDTH/(float)SCREEN_HEIGHT;
	camera_update(&cam);

	Transform cube_transform = {
		.position = {0, 0, 0},
		.scale = {1, 1, 1}
	};

	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);


	float sensitivity = 0.002f; // adjust for speed

	double time = glfwGetTime();
	double prev_time = 0.0;
	double delta_time = 0.0f;
	while (!glfwWindowShouldClose(window)) {
		int width, height;
		glfwGetWindowSize(window, &width, &height);

		double xpos, ypos;
		glfwGetCursorPos(window, &xpos, &ypos);


		static double lastX = 0, lastY = 0;
		static int firstMouse = 1;

		if (firstMouse) {
			lastX = xpos;
			lastY = ypos;
			firstMouse = 0;
		}

		double xoffset = xpos - lastX;
		double yoffset = lastY - ypos;

		lastX = xpos;
		lastY = ypos;

		xoffset *= sensitivity;
		yoffset *= sensitivity;

		cam.transform.rotation.y -= xoffset;
		cam.transform.rotation.x += yoffset;
		if (cam.transform.rotation.x >  1.5f) cam.transform.rotation.x =  1.5f;
		if (cam.transform.rotation.x < -1.5f) cam.transform.rotation.x = -1.5f;

		glfwPollEvents();

		V3f forward = { sinf(cam.transform.rotation.y), 0, cosf(cam.transform.rotation.y) };
		V3f right = { forward.z, 0, -forward.x };

		float speed = 5.0f * delta_time;
		if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
			cam.transform.position.x += forward.x * speed;
			cam.transform.position.z -= forward.z * speed;
		}
		if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
			cam.transform.position.x -= forward.x * speed;
			cam.transform.position.z += forward.z * speed;
		}
		if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
			cam.transform.position.x -= right.x * speed;
			cam.transform.position.z -= right.z * speed;
		}
		if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
			cam.transform.position.x += right.x * speed;
			cam.transform.position.z += right.z * speed;
		}


		if (global_scroll_y != 0.0) {
			cam.fov -= (float)global_scroll_y;
			if (cam.fov < 1.0f)    cam.fov = 1.0f;
			if (cam.fov > 1000.0f) cam.fov = 1000.0f;
			global_scroll_y = 0.0;
		}


		const float bg_color = 20.0f/255.0f;
		glClearColor(bg_color, bg_color, bg_color, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glUseProgram(program);

		cam.aspect = (float)width/(float)height;

		float cube_speed = 0.01f;
		cube_transform.rotation.x += cube_speed;
		cube_transform.rotation.y += cube_speed;
		cube_transform.rotation.z += cube_speed;
		//cube_transform.rotation = (V3f){time, time * 0.6f, 0};
		camera_update(&cam);

		draw_mesh(&cube_mesh, &cube_transform, &cam);

		glfwSwapBuffers(window);

		double cur_time = glfwGetTime();
		delta_time = cur_time - prev_time;
		time += delta_time;
		prev_time = cur_time;
	}

    glfwDestroyWindow(window);
	glfwTerminate();
	return 0;
}
