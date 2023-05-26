#include "loop.h"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

struct {
	GLFWwindow* window;
} WINDOW;

void initWindow() {
	glfwInit();

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

	WINDOW.window = glfwCreateWindow(WIDTH, HEIGHT, "C Vulkan Renderer", NULL, NULL);
}

void mainloop() {
	while (!glfwWindowShouldClose(WINDOW.window)) {
		glfwPollEvents();
	}
}

void clean() {
	glfwDestroyWindow(WINDOW.window);

	glfwTerminate();
}

void run() {
	initWindow();
	initVk();
	mainloop();
	clean();
}
