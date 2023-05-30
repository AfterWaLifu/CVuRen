#include "vkthings.h"

#include <stdio.h>
#include <stdlib.h>

#ifndef GLFW_INCLUDE_VULKAN
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#endif

struct {
    VkInstance instance;
} VULKAN;

void initVk() {
    createInstance();
}

void createInstance() {

    uint32_t glfwExtensionCount = 0;
    const char** glfwExtensions;
    glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
    
    uint32_t extensionCount = 0;
    vkEnumerateInstanceExtensionProperties(NULL, &extensionCount, NULL);
    VkExtensionProperties* extensions = (VkExtensionProperties*) malloc(sizeof(VkExtensionProperties) * extensionCount);

    vkEnumerateInstanceExtensionProperties(NULL, &extensionCount, extensions);
    printf("extentions available - %u:\n", extensionCount);
    for (uint32_t i = 0; i < extensionCount; ++i) {
        if (extensions && extensions[i].extensionName)
            printf("\t%s\n", extensions[i].extensionName);
    }
    
    VkApplicationInfo app = {
        .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
        .pNext = NULL,
        .pApplicationName = "C Vulkan Renderer",
        .applicationVersion = 0,
        .pEngineName = "CVuRen",
        .engineVersion = 0,
        .apiVersion = VK_API_VERSION_1_0,
    };
    VkInstanceCreateInfo inst_info = {
        .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
        .pNext = NULL,
        .flags = 0,
        .pApplicationInfo = &app,
        .enabledLayerCount = 0,
        .ppEnabledLayerNames = NULL,
        .enabledExtensionCount = glfwExtensionCount,
        .ppEnabledExtensionNames = (const char* const*)glfwExtensions,
    };

    if (vkCreateInstance(&inst_info, NULL, &VULKAN.instance) != VK_SUCCESS) {
        fprintf(stderr, "error on creating vulkan instance\n");
    }
}

void cleanVk() {
    vkDestroyInstance(VULKAN.instance, NULL);
}
