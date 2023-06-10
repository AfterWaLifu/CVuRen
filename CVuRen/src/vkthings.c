#include "vkthings.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#ifndef GLFW_INCLUDE_VULKAN
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#endif

#include "utils/dynamic_array.h"
#include "utils/utils.h"

#ifdef NDEBUG
#define VALIDATION_LAYERS 0
#else
#define VALIDATION_LAYERS 1
#endif

struct VULKAN {
    VkInstance instance;
    VkPhysicalDevice physicalDevice;
    VkDevice device;
    VkQueue graphicsQueue;
    VkDebugUtilsMessengerEXT debugMessenger;
} VULKAN;

typedef struct QueueFamilyIndices {
    uint32_t graphicsFamily;
    bool itIs;
} QueueFamilyIndices;

//  PROTOTYPES
void createInstance();
//  DEVICES-RELATED
void pickPhysicalDevice();
bool isDeviceSuitable(VkPhysicalDevice device);
QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);
void createLogicalDevice();
//	VALIDATION THINGS
uint32_t checkValidationLayersSupport();
void getRequiredExtensions(dynamic_array_string* extensions);
static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageType,
    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData);
void setupDebugMessenger();
VkResult CreateDebugUtilsMessengerEXT(const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
    const VkAllocationCallbacks* pAllocator);
void DestroyDebugUtilsMessengerEXT(const VkAllocationCallbacks* pAllocator);
void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT* createInfo);

//  FROM .H
void initVk() {
    createInstance();
    setupDebugMessenger();
    pickPhysicalDevice();
    createLogicalDevice();
}
void cleanVk() {
    vkDestroyDevice(VULKAN.device, NULL);
    if (VALIDATION_LAYERS) {
        DestroyDebugUtilsMessengerEXT(NULL);
    }
    vkDestroyInstance(VULKAN.instance, NULL);
}
//  END OF .H

void createInstance() {
    if (VALIDATION_LAYERS && !checkValidationLayersSupport()) {
        printf("validation layers requested, but not available");
    }

    VULKAN.physicalDevice = VK_NULL_HANDLE;

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

void pickPhysicalDevice() {
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(VULKAN.instance, &deviceCount, NULL);
    
    if (deviceCount == 0) {
        c_throw("failed to find device with Vulkan support\n");
        return;
    }

    VkPhysicalDevice* devices = (VkPhysicalDevice*)malloc(sizeof(VkPhysicalDevice) * deviceCount);
    vkEnumeratePhysicalDevices(VULKAN.instance, &deviceCount, devices);

    for (uint32_t i = 0; i < deviceCount; ++i) {
        if (isDeviceSuitable(devices[i])) {
            VULKAN.physicalDevice = devices[i];
            break;
        }
    }
    if (VULKAN.physicalDevice == VK_NULL_HANDLE) {
        c_throw("failed to find suitable gpu");
        return;
    }

}

bool isDeviceSuitable(VkPhysicalDevice device) {
    VkPhysicalDeviceProperties deviceProperties;
    VkPhysicalDeviceFeatures deviceFeatures;
    vkGetPhysicalDeviceProperties(device, &deviceProperties);
    vkGetPhysicalDeviceFeatures(device, &deviceFeatures);
    QueueFamilyIndices qfi = findQueueFamilies(device);

    return (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) && qfi.itIs;
}

QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device)
{
    QueueFamilyIndices qfi = {UINT32_MAX};
    
    uint32_t qCount;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &qCount, NULL);
    VkQueueFamilyProperties* queueFamilies = (VkQueueFamilyProperties*)malloc(sizeof(VkQueueFamilyProperties)*qCount);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &qCount, queueFamilies);

    for (uint32_t i = 0; i < qCount; ++i) {
        if (queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            qfi.graphicsFamily = i;
            qfi.itIs = true;
            break;
        }
    }
    
    return qfi;
}

void createLogicalDevice() {
    QueueFamilyIndices indices = findQueueFamilies(VULKAN.physicalDevice);

    const float qPriority = 1.0;
    VkDeviceQueueCreateInfo queueCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
        .pNext = NULL,
        .flags = 0,
        .queueFamilyIndex = indices.graphicsFamily,
        .queueCount = 1,
        .pQueuePriorities = &qPriority
    };

    VkDeviceCreateInfo createInfo = {
        .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
        .pNext = NULL,
        .flags = 0,
        .queueCreateInfoCount = 1,
        .pQueueCreateInfos = &queueCreateInfo,
        .enabledLayerCount = 0,
        .ppEnabledLayerNames = NULL,
        .enabledExtensionCount = 0,
        .ppEnabledExtensionNames = NULL,
        .pEnabledFeatures = NULL
    };

    char** layersNames = malloc(sizeof(char*));
    char name[] = { "VK_LAYER_KHRONOS_validation" };
    layersNames[0] = name;
    if (VALIDATION_LAYERS) {
        createInfo.enabledLayerCount = 1;
        createInfo.ppEnabledLayerNames = layersNames;
    }
    
    if (vkCreateDevice(VULKAN.physicalDevice, &createInfo, NULL, &VULKAN.device)) {
        c_throw("failed to create logical device");
    }

    vkGetDeviceQueue(VULKAN.device, indices.graphicsFamily, 0, &VULKAN.graphicsQueue);
}

//  VALIDATION THINGS IMPLEMENTATION

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