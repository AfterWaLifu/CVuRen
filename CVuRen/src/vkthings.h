#pragma once

#include <vulkan/vulkan.h>

#include "utils/dynamic_array.h"

void initVk();
void createInstance();
void cleanVk();

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