#include "loop.h"

#include "window.h"
#include "vkthings.h"

void mainloop() {
	while (!glfwWindowShouldClose(WINDOW.window)) {
		glfwPollEvents();
		drawFrame();
	}
	deviceIdle();
}

void run() {
	initWindow();
	initVk();
	mainloop();
	cleanVk();
	cleanWindow();
}
