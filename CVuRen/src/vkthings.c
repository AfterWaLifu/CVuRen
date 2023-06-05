#include "vkthings.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef GLFW_INCLUDE_VULKAN
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#endif

#include "utils/dynamic_array.h"

#ifdef NDEBUG
#define VALIDATION_LAYERS 0
#else
#define VALIDATION_LAYERS 1
#endif

struct {
    VkInstance instance;
    VkDebugUtilsMessengerEXT debugMessenger;
} VULKAN;

void initVk() {
    createInstance();
    setupDebugMessenger();
}

void cleanVk() {
    if (VALIDATION_LAYERS) {
        DestroyDebugUtilsMessengerEXT(NULL);
    }
    vkDestroyInstance(VULKAN.instance, NULL);
}

void createInstance() {
    if (VALIDATION_LAYERS && !checkValidationLayersSupport()) {
        printf("validation layers requested, but not available");
    }

    dynamic_array_string glfwExtensions = { NULL, 0 };
    getRequiredExtensions(&glfwExtensions);

    uint32_t extensionCount = 0;
    vkEnumerateInstanceExtensionProperties(NULL, &extensionCount, NULL);
    VkExtensionProperties* extensions = (VkExtensionProperties*)malloc(sizeof(VkExtensionProperties) * extensionCount);

    vkEnumerateInstanceExtensionProperties(NULL, &extensionCount, extensions);

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
        .enabledExtensionCount = (uint32_t)glfwExtensions.length,
        .ppEnabledExtensionNames = (const char* const*)glfwExtensions.array,
    };

    VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo;
    char** layersNames = malloc(sizeof(char*));
    char name[] = { "VK_LAYER_KHRONOS_validation" };
    layersNames[0] = name;
    if (VALIDATION_LAYERS) {
        inst_info.enabledLayerCount = 1;
        inst_info.ppEnabledLayerNames = layersNames;
        populateDebugMessengerCreateInfo(&debugCreateInfo);
        inst_info.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;
    }

    if (vkCreateInstance(&inst_info, NULL, &VULKAN.instance) != VK_SUCCESS) {
        fprintf(stderr, "error on creating vulkan instance\n");
    }
}

uint32_t checkValidationLayersSupport() {
    uint32_t layerCount;
    vkEnumerateInstanceLayerProperties(&layerCount, NULL);

    VkLayerProperties* availableLayers = malloc(sizeof(VkLayerProperties) * layerCount);
    vkEnumerateInstanceLayerProperties(&layerCount, availableLayers);

    int found = 0;
    for (uint32_t j = 0; j < layerCount; ++j) {
        if (strcmp("VK_LAYER_KHRONOS_validation", availableLayers[j].layerName) == 0) {
            found = 1;
            break;
        }
    }
    if (!found) return 0;

    return layerCount;
}

void getRequiredExtensions(dynamic_array_string* extensions) {
    uint32_t glfwExtensionCount = 0;
    char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

    for (uint32_t i = 0; i < glfwExtensionCount; ++i) {
        das_pushback(extensions, glfwExtensions[i]);
    }
    if (VALIDATION_LAYERS) das_pushback(extensions, VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
}

VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageType,
    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData) {
    fprintf(stderr, "\nValidation layer:");
    fprintf(stderr, pCallbackData->pMessage);
    fprintf(stderr, "\n");
    return VK_FALSE;
}

void setupDebugMessenger() {
    if (!VALIDATION_LAYERS) return;

    VkDebugUtilsMessengerCreateInfoEXT createInfo;
    populateDebugMessengerCreateInfo(&createInfo);

    if (CreateDebugUtilsMessengerEXT(&createInfo, NULL) != VK_SUCCESS) {
        fprintf(stderr, "failed on creating an debug messenger");
    }
}

VkResult CreateDebugUtilsMessengerEXT(const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
    const VkAllocationCallbacks* pAllocator) {
    PFN_vkCreateDebugUtilsMessengerEXT func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(VULKAN.instance, "vkCreateDebugUtilsMessengerEXT");
    if (func) {
        return func(VULKAN.instance, pCreateInfo, pAllocator, &(VULKAN.debugMessenger));
    }
    else return VK_ERROR_EXTENSION_NOT_PRESENT;
}

void DestroyDebugUtilsMessengerEXT(const VkAllocationCallbacks* pAllocator) {
    PFN_vkDestroyDebugUtilsMessengerEXT func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(VULKAN.instance, "vkDestroyDebugUtilsMessengerEXT");
    if (func) {
        func(VULKAN.instance, VULKAN.debugMessenger, pAllocator);
    }
}

void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT* createInfo) {
    createInfo->sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    createInfo->flags = 0;
    createInfo->pNext = NULL;
    createInfo->messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    createInfo->messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    createInfo->pfnUserCallback = debugCallback;
    createInfo->pUserData = NULL;
}