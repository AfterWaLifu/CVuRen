#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

static const uint32_t WIDTH = 800;
static const uint32_t HEIGHT = 600;

struct {
	GLFWwindow* window;
} WINDOW;

void initWindow();
void cleanWindow();