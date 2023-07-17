#include "window.h"

void initWindow() {
	glfwInit();

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

	WINDOW.window = glfwCreateWindow(WIDTH, HEIGHT, "C Vulkan Renderer", NULL, NULL);
}

void cleanWindow() {
	glfwDestroyWindow(WINDOW.window);
	glfwTerminate();
}
