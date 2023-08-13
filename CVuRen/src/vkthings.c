#include "vkthings.h"

#include <vulkan/vulkan.h>
#include <stb_image.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "window.h"
#include "vkstructs.h"
#include "vertexes.h"

#include "utils/dynamic_array.h"
#include "utils/utils.h"

#ifdef NDEBUG
#define VALIDATION_LAYERS 0
#else
#define VALIDATION_LAYERS 1
#endif

// ======= VULKAN DATA STRUCT ======= //
static struct VULKAN {
    VkInstance instance;
    VkSurfaceKHR surface;
    
    VkPhysicalDevice physicalDevice;
    VkDevice device;
    
    VkQueue graphicsQueue;
    VkQueue presentQueue;
    VkSwapchainKHR swapchain;
    
    VkFormat swapchainImageFormat;
    VkExtent2D swapchainExtent;
    vkimages swapchainImages;
    vkimageviews swapchainImageViews;

    VkRenderPass renderPass;
    VkDescriptorSetLayout descriptorSetLayout;
    VkPipelineLayout pipelineLayout;
    VkPipeline pipeline;
    
    framebuffer swapchainFramebuffers;
    VkCommandPool commandPool;
    VkCommandBuffer* commandBuffer;

    VkSemaphore* imageAvailableSemaphore;
    VkSemaphore* renderFinishedSemaphore;
    VkFence* inFlightFence;

    bool framebufferResized;

    uint32_t currentFrame;

    VkBuffer vertexBuffer;
    VkDeviceMemory vertexBufferMemory;
    VkBuffer indexBuffer;
    VkDeviceMemory indexBufferMemory;

    VkBuffer* uniformBuffers;
    VkDeviceMemory* uniformBuffersMemory;
    void** uniformBuffersMapped;

    VkDescriptorPool descriptorPool;
    VkDescriptorSet* descriptorSets;

    VkImage textureImage;
    VkDeviceMemory textureImageMemory;
    VkImageView textureImageView;
    VkSampler textureSampler;

    VkImage depthImage;
    VkDeviceMemory depthImageMemory;
    VkImageView depthImageView;

    VkDebugUtilsMessengerEXT debugMessenger;
} VULKAN;

#ifdef NDEBUG
const char** validationLayers = NULL;
#else
const char* validationLayers[] = {"VK_LAYER_KHRONOS_validation\0"};
#endif

const char* deviceExtensions[] = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};
const uint32_t deviceExtensionsCount = 1;

const int MAX_FRAMES_IN_FLIGHT = 2;

#define VERTICES_COUNT 8
Vertex vertices[VERTICES_COUNT] = {
    {{-0.5f, -0.5f, 0.5f},{1.0f, 0.0f, 0.0f,1.0f}, {1.0f, 0.0f, 0.0f}},
    {{ 0.5f, -0.5f, 0.5f},{0.0f, 1.0f, 0.0f,1.0f}, {0.0f, 0.0f, 0.0f}},
    {{ 0.5f,  0.5f, 0.5f},{0.0f, 0.0f, 1.0f,1.0f}, {0.0f, 1.0f, 0.0f}},
    {{-0.5f,  0.5f, 0.5f},{0.0f, 1.0f, 0.0f,1.0f}, {1.0f, 1.0f, 0.0f}},

    {{-0.5f, -0.5f, 0.0f},{1.0f, 0.0f, 0.0f,1.0f}, {1.0f, 0.0f, 0.0f}},
    {{ 0.5f, -0.5f, 0.0f},{0.0f, 1.0f, 0.0f,1.0f}, {0.0f, 0.0f, 0.0f}},
    {{ 0.5f,  0.5f, 0.0f},{0.0f, 0.0f, 1.0f,1.0f}, {0.0f, 1.0f, 0.0f}},
    {{-0.5f,  0.5f, 0.0f},{0.0f, 1.0f, 0.0f,1.0f}, {1.0f, 1.0f, 0.0f}}
};
#define INDICES_COUNT 12
const uint32_t indices[INDICES_COUNT] = {
    0, 1, 2, 2, 3, 0,
    4, 5, 6, 6, 7, 4
};

typedef struct QueueFamilyIndices {
    uint32_t graphicsFamily;
    uint32_t presentFamily;
    bool itIs;
} QueueFamilyIndices;

typedef struct SwapChainSupportDetails {
    VkSurfaceCapabilitiesKHR* capabilities;
    VkSurfaceFormatKHR* formats;
    VkPresentModeKHR* presentModes;
    uint32_t formatsCount, presentModesCount;
} SwapChainSupportDetails;

//  PROTOTYPES
void createInstance();
void createSurface();
void createSwapChain();

void pickPhysicalDevice();
bool isDeviceSuitable(VkPhysicalDevice device);
QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);
void createLogicalDevice();
bool checkDeviceExtensionSupport(VkPhysicalDevice device);
SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device);
VkSurfaceFormatKHR chooseSwapSurfaceFormat(const VkSurfaceFormatKHR* formats, uint32_t count);
VkPresentModeKHR chooseSwapPresentMode(const VkPresentModeKHR* modes, uint32_t count);
VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR* capabilities);
void createImageViews();
void createRenderPass();
void createGraphicsPipeline();
shaderfile readFile(const char* filename);
VkShaderModule createShaderModule(shaderfile file);
void createFramebuffers();
void createCommandPool();
void createCommandBuffers();
void recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex);
void createSyncObjects();
void recreateSwapchain();
void clearupSwapchain();
void createVertexBuffer();
void createIndexBuffer();
uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage,
    VkMemoryPropertyFlags properties, VkBuffer* buffer, VkDeviceMemory* bufferMemory);
void copyBuffer(VkBuffer srcBuffer,VkBuffer dstBuffer, VkDeviceSize size);
void createDescriptorSetLayout();
void createUniformBuffers();
void updateUniformBuffer(uint32_t currentImage);
void createDescriptorPool();
void createDescriptorSets();
void createTextureImage();
void createImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling,
    VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage* image, VkDeviceMemory* imageMemory);
VkCommandBuffer beginSingleTimeCommands();
void endSingleTimeCommands(VkCommandBuffer commandBuffer);
void transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout);
void copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);
void createTextureImageView();
VkImageView createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags);
void createTextureSampler();
void createDepthResources();
VkFormat findSupportedFormat(const VkFormat* candidates, uint32_t candidatesCount,
    VkImageTiling tiling, VkFormatFeatureFlags features);
VkFormat findDepthFormat();
bool hasStancilComponent(VkFormat format);

static void framebufferResizeCallback(GLFWwindow* window, int width, int height);

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
    glfwSetFramebufferSizeCallback(WINDOW.window, framebufferResizeCallback);
    createInstance();
    setupDebugMessenger();
    createSurface();
    pickPhysicalDevice();
    createLogicalDevice();
    createSwapChain();
    createImageViews();
    createRenderPass();
    createDescriptorSetLayout();
    createGraphicsPipeline();
    createCommandPool();
    createDepthResources();
    createFramebuffers();
    createTextureImage();
    createTextureImageView();
    createTextureSampler();
    createVertexBuffer();
    createIndexBuffer();
    createUniformBuffers();
    createDescriptorPool();
    createDescriptorSets();
    createCommandBuffers();
    createSyncObjects();
}
void cleanVk() {
    clearupSwapchain();

    vkDestroySampler(VULKAN.device, VULKAN.textureSampler, NULL);
    vkDestroyImageView(VULKAN.device, VULKAN.textureImageView, NULL);
    vkDestroyImage(VULKAN.device, VULKAN.textureImage, NULL);
    vkFreeMemory(VULKAN.device, VULKAN.textureImageMemory, NULL);

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        vkDestroyBuffer(VULKAN.device, VULKAN.uniformBuffers[i], NULL);
        vkFreeMemory(VULKAN.device, VULKAN.uniformBuffersMemory[i], NULL);
    }

    vkDestroyDescriptorPool(VULKAN.device, VULKAN.descriptorPool, NULL);
    vkDestroyDescriptorSetLayout(VULKAN.device, VULKAN.descriptorSetLayout, NULL);

    vkDestroyBuffer(VULKAN.device, VULKAN.indexBuffer, NULL);
    vkFreeMemory(VULKAN.device, VULKAN.indexBufferMemory, NULL);

    vkDestroyBuffer(VULKAN.device, VULKAN.vertexBuffer, NULL);
    vkFreeMemory(VULKAN.device, VULKAN.vertexBufferMemory, NULL);

    vkDestroyPipeline(VULKAN.device, VULKAN.pipeline, NULL);
    vkDestroyPipelineLayout(VULKAN.device, VULKAN.pipelineLayout, NULL);

    vkDestroyRenderPass(VULKAN.device, VULKAN.renderPass, NULL);

    for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i) {
        vkDestroySemaphore(VULKAN.device, VULKAN.imageAvailableSemaphore[i], NULL);
        vkDestroySemaphore(VULKAN.device, VULKAN.renderFinishedSemaphore[i], NULL);
        vkDestroyFence(VULKAN.device, VULKAN.inFlightFence[i], NULL);
    }

    vkDestroyCommandPool(VULKAN.device, VULKAN.commandPool, NULL);
    
    vkDestroyDevice(VULKAN.device, NULL);

    if (VALIDATION_LAYERS) {
        DestroyDebugUtilsMessengerEXT(NULL);
    }

    vkDestroySurfaceKHR(VULKAN.instance, VULKAN.surface, NULL);
    vkDestroyInstance(VULKAN.instance, NULL);
}

void drawFrame() {
    vkWaitForFences(VULKAN.device, 1, VULKAN.inFlightFence+VULKAN.currentFrame, VK_TRUE, UINT64_MAX);

    uint32_t imageIndex = 0;
    
    VkResult result = vkAcquireNextImageKHR(VULKAN.device, VULKAN.swapchain, UINT64_MAX,
        VULKAN.imageAvailableSemaphore[VULKAN.currentFrame], VK_NULL_HANDLE, &imageIndex);

    if (result == VK_ERROR_OUT_OF_DATE_KHR) {
        recreateSwapchain();
        return;
    } else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
        c_throw("failed to acquire swapchain image");
    }

    updateUniformBuffer(VULKAN.currentFrame);

    vkResetFences(VULKAN.device, 1, VULKAN.inFlightFence + VULKAN.currentFrame);

    vkResetCommandBuffer(VULKAN.commandBuffer[VULKAN.currentFrame], 0);
    recordCommandBuffer(VULKAN.commandBuffer[VULKAN.currentFrame], imageIndex);

    VkSemaphore waitSemaphores[] = { VULKAN.imageAvailableSemaphore[VULKAN.currentFrame] };
    VkSemaphore signalSemaphores[] = { VULKAN.renderFinishedSemaphore[VULKAN.currentFrame] };
    VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
    VkSubmitInfo submitInfo = {
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .pNext = NULL,
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = waitSemaphores,
        .pWaitDstStageMask = waitStages,
        .commandBufferCount = 1,
        .pCommandBuffers = VULKAN.commandBuffer+VULKAN.currentFrame,
        .signalSemaphoreCount = 1,
        .pSignalSemaphores = signalSemaphores
    };

    if (vkQueueSubmit(VULKAN.graphicsQueue, 1, &submitInfo, VULKAN.inFlightFence[VULKAN.currentFrame]) != VK_SUCCESS) {
        c_throw("failed to submit draw command buffer");
    }

    VkPresentInfoKHR presentInfo = {
        .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
        .pNext = NULL,
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = signalSemaphores,
        .swapchainCount = 1,
        .pSwapchains = &VULKAN.swapchain,
        .pImageIndices = &imageIndex,
        .pResults = NULL
    };

    result = vkQueuePresentKHR(VULKAN.presentQueue, &presentInfo);

    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
        VULKAN.framebufferResized = false;
        recreateSwapchain();
    } else if (result != VK_SUCCESS) {
        c_throw("failed to present swapchain image");
    }

    VULKAN.currentFrame = (VULKAN.currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
}
void deviceIdle() {
    vkDeviceWaitIdle(VULKAN.device);
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
    if (VALIDATION_LAYERS) {
        inst_info.enabledLayerCount = VALIDATION_LAYERS;
        inst_info.ppEnabledLayerNames = validationLayers;
        populateDebugMessengerCreateInfo(&debugCreateInfo);
        inst_info.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;
    }

    if (vkCreateInstance(&inst_info, NULL, &VULKAN.instance) != VK_SUCCESS) {
        fprintf(stderr, "error on creating vulkan instance\n");
    }
}

void createSurface() {
    if (glfwCreateWindowSurface(VULKAN.instance, WINDOW.window, NULL, &(VULKAN.surface)) != VK_SUCCESS) {
        c_throw("failed to create window surface\n");
    }
}

void createSwapChain() {
    SwapChainSupportDetails scsd = querySwapChainSupport(VULKAN.physicalDevice);

    VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(scsd.formats, scsd.formatsCount);
    VkPresentModeKHR presentMode = chooseSwapPresentMode(scsd.presentModes, scsd.presentModesCount);
    VkExtent2D extent = chooseSwapExtent(scsd.capabilities);
    uint32_t imageCount = scsd.capabilities->minImageCount + 1;

    if (scsd.capabilities->maxImageCount > 0 && imageCount > scsd.capabilities->maxImageCount) {
        imageCount = scsd.capabilities->maxImageCount;
    }

    QueueFamilyIndices indices = findQueueFamilies(VULKAN.physicalDevice);
    uint32_t queueFamilyIndices[] = {indices.graphicsFamily, indices.presentFamily};
    bool qGraphNEQPresent = indices.graphicsFamily != indices.presentFamily;

    VkSwapchainCreateInfoKHR createInfo = {
        .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
        .pNext = NULL,
        .flags = 0,
        .surface = VULKAN.surface,
        .minImageCount = imageCount,
        .imageFormat = surfaceFormat.format,
        .imageColorSpace = surfaceFormat.colorSpace,
        .imageExtent = extent,
        .imageArrayLayers = 1,
        .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
        .imageSharingMode = qGraphNEQPresent ? VK_SHARING_MODE_CONCURRENT : VK_SHARING_MODE_EXCLUSIVE,
        .queueFamilyIndexCount = qGraphNEQPresent ? 2 : 0,
        .pQueueFamilyIndices = qGraphNEQPresent ? queueFamilyIndices : NULL,
        .preTransform = scsd.capabilities->currentTransform,
        .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
        .presentMode = presentMode,
        .clipped = VK_TRUE,
        .oldSwapchain = VK_NULL_HANDLE
    };

    if (vkCreateSwapchainKHR(VULKAN.device, &createInfo, NULL, &VULKAN.swapchain) != VK_SUCCESS) {
        c_throw("failed to create swapchain");
    }

    vkGetSwapchainImagesKHR(VULKAN.device, VULKAN.swapchain, &VULKAN.swapchainImages.count, NULL);
    VULKAN.swapchainImages.swapchainImages = (VkImage*)malloc(sizeof(VkImage) * VULKAN.swapchainImages.count);
    vkGetSwapchainImagesKHR(VULKAN.device, VULKAN.swapchain, &VULKAN.swapchainImages.count, VULKAN.swapchainImages.swapchainImages);

    VULKAN.swapchainImageFormat = surfaceFormat.format;
    VULKAN.swapchainExtent = extent;
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
        c_throw("failed to find suitable gpu\n");
        return;
    }

}

bool isDeviceSuitable(VkPhysicalDevice device) {
    VkPhysicalDeviceProperties deviceProperties;
    VkPhysicalDeviceFeatures deviceFeatures;
    vkGetPhysicalDeviceProperties(device, &deviceProperties);
    vkGetPhysicalDeviceFeatures(device, &deviceFeatures);
    QueueFamilyIndices qfi = findQueueFamilies(device);
    bool deviceExtSupported = checkDeviceExtensionSupport(device);
    SwapChainSupportDetails scsd = querySwapChainSupport(device);
    bool swapchainOk = (scsd.formatsCount > 0) && (scsd.presentModesCount>0);

    return (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) &&
        qfi.itIs && deviceExtSupported && swapchainOk && deviceFeatures.samplerAnisotropy;
}

QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device)
{
    QueueFamilyIndices qfi = {UINT32_MAX, UINT32_MAX, false};
    
    uint32_t qCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &qCount, NULL);
    VkQueueFamilyProperties* queueFamilies = (VkQueueFamilyProperties*)malloc(sizeof(VkQueueFamilyProperties)*qCount);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &qCount, queueFamilies);

    VkBool32 presentSupport = VK_FALSE;
    for (uint32_t i = 0; i < qCount; ++i) {
        if (qfi.graphicsFamily == UINT32_MAX && queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            qfi.graphicsFamily = i;
            continue;
        }
        vkGetPhysicalDeviceSurfaceSupportKHR(device, i, VULKAN.surface, &presentSupport);
        if (presentSupport) {
            qfi.presentFamily = i;
        }
        if (qfi.graphicsFamily != UINT32_MAX && qfi.presentFamily != UINT32_MAX) {
            qfi.itIs = true;
            break;
        }
    }
    return qfi;
}

void createLogicalDevice() {
    QueueFamilyIndices indices = findQueueFamilies(VULKAN.physicalDevice);

#define QUEUES_COUNT 2
    VkDeviceQueueCreateInfo queueCreateInfos[QUEUES_COUNT];
    uint32_t uniquiQueueFamilies[QUEUES_COUNT] = {indices.graphicsFamily, indices.presentFamily};

    const float qPriority = 1.0;
    for (int i = 0; i < QUEUES_COUNT; ++i) {
        queueCreateInfos[i].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfos[i].pNext = NULL;
        queueCreateInfos[i].flags = 0;
        queueCreateInfos[i].queueFamilyIndex = uniquiQueueFamilies[i];
        queueCreateInfos[i].queueCount = 1;
        queueCreateInfos[i].pQueuePriorities = &qPriority;
    }

    VkPhysicalDeviceFeatures deviceFeatures;
    memset(&deviceFeatures, 0, sizeof(VkPhysicalDeviceFeatures));
    deviceFeatures.logicOp = VK_TRUE;
    deviceFeatures.samplerAnisotropy = VK_TRUE;

    VkDeviceCreateInfo createInfo = {
        .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
        .pNext = NULL,
        .flags = 0,
        .queueCreateInfoCount = QUEUES_COUNT,
        .pQueueCreateInfos = queueCreateInfos,
        .enabledLayerCount = 0,
        .ppEnabledLayerNames = NULL,
        .enabledExtensionCount = deviceExtensionsCount,
        .ppEnabledExtensionNames = deviceExtensions,
        .pEnabledFeatures = &deviceFeatures
    };

    if (VALIDATION_LAYERS) {
        createInfo.enabledLayerCount = VALIDATION_LAYERS;
        createInfo.ppEnabledLayerNames = validationLayers;
    }
    
    if (vkCreateDevice(VULKAN.physicalDevice, &createInfo, NULL, &VULKAN.device)) {
        c_throw("failed to create logical device\n");
    }

    vkGetDeviceQueue(VULKAN.device, indices.graphicsFamily, 0, &VULKAN.graphicsQueue);
    vkGetDeviceQueue(VULKAN.device, indices.presentFamily, 0, &VULKAN.presentQueue);
}

bool checkDeviceExtensionSupport(VkPhysicalDevice device) {
    uint32_t extensionCount;
    vkEnumerateDeviceExtensionProperties(device, NULL, &extensionCount, NULL);
    VkExtensionProperties* availableExtensions = malloc(sizeof(VkExtensionProperties) * extensionCount);
    vkEnumerateDeviceExtensionProperties(device, NULL, &extensionCount, availableExtensions);

    bool found = false;
    for (uint32_t i = 0; i < deviceExtensionsCount; ++i) {
        for (uint32_t j = 0; j < extensionCount; ++j) {
            if (strcmp(deviceExtensions[i], availableExtensions[j].extensionName) == 0) {
                found = true;
                break;
            }
        }
        if (!found) {
            return false;
        }
    }
    return true;
}

SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device) {
    SwapChainSupportDetails details = {NULL,NULL,NULL,0,0};

    details.capabilities = (VkSurfaceCapabilitiesKHR*)malloc(sizeof(VkSurfaceCapabilitiesKHR));
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, VULKAN.surface, details.capabilities);

    vkGetPhysicalDeviceSurfaceFormatsKHR(device, VULKAN.surface, &details.formatsCount, NULL);
    if (details.formatsCount) {
        details.formats = (VkSurfaceFormatKHR*)malloc(sizeof(VkSurfaceFormatKHR) * details.formatsCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, VULKAN.surface, &details.formatsCount, details.formats);
    }
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, VULKAN.surface, &details.presentModesCount, NULL);
    if (details.presentModesCount) {
        details.presentModes = (VkPresentModeKHR*)malloc(sizeof(VkPresentModeKHR) * details.formatsCount);
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, VULKAN.surface,
            &details.presentModesCount, details.presentModes);
    }

    return details;
}

VkSurfaceFormatKHR chooseSwapSurfaceFormat(const VkSurfaceFormatKHR* formats, uint32_t count) {
    for (uint32_t i = 0; i < count; ++i) {
        if ((formats[i].format == VK_FORMAT_B8G8R8A8_SRGB) &&
            (formats[i].colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)) {
            return formats[i];
        }
    }
    return formats[0];
}

VkPresentModeKHR chooseSwapPresentMode(const VkPresentModeKHR* modes, uint32_t count) {
    for (uint32_t i = 0; i < count; ++i) {
        if (modes[i] == VK_PRESENT_MODE_MAILBOX_KHR) {
            return modes[i];
        }
    }
    return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR* capabilities) {
    if (capabilities->currentExtent.width != UINT32_MAX) {
        return capabilities->currentExtent;
    } else {
        int width, height;
        glfwGetFramebufferSize(WINDOW.window, &width, &height);

        VkExtent2D actualExtent = {
            (uint32_t)width, (uint32_t)height
        };
        actualExtent.width = u32_clamp(actualExtent.width, 
            capabilities->minImageExtent.width, capabilities->maxImageExtent.width);
        actualExtent.height = u32_clamp(actualExtent.height,
            capabilities->minImageExtent.height, capabilities->maxImageExtent.height);
        return actualExtent;
    }
}

void createImageViews() {
    VULKAN.swapchainImageViews.count = VULKAN.swapchainImages.count;
    VULKAN.swapchainImageViews.swapChainImageViews = 
        (VkImageView*)malloc(sizeof(VkImageView) * VULKAN.swapchainImageViews.count);

    for (uint32_t i = 0; i < VULKAN.swapchainImages.count; ++i) {
        VULKAN.swapchainImageViews.swapChainImageViews[i] =
            createImageView(VULKAN.swapchainImages.swapchainImages[i], VULKAN.swapchainImageFormat, VK_IMAGE_ASPECT_COLOR_BIT);
    }
}

void createRenderPass() {
    VkAttachmentDescription colorAttachment = {
        .flags = 0,
        .format = VULKAN.swapchainImageFormat,
        .samples = VK_SAMPLE_COUNT_1_BIT,
        .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
        .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
        .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
        .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
        .finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
    };
    VkAttachmentDescription depthAttachment = {
        .flags = 0,
        .format = findDepthFormat(),
        .samples = VK_SAMPLE_COUNT_1_BIT,
        .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
        .storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
        .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
        .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
        .finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
    };
    VkAttachmentDescription attachments[] = { colorAttachment, depthAttachment };

    VkAttachmentReference colorAttachmentRef = {
        .attachment = 0,
        .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
    };
    VkAttachmentReference depthAttachmentRef = {
        .attachment = 1,
        .layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
    };

    VkSubpassDescription subpass = {
        .flags = 0,
        .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
        .inputAttachmentCount = 0,
        .pInputAttachments = NULL,
        .colorAttachmentCount = 1,
        .pColorAttachments = &colorAttachmentRef,
        .pResolveAttachments = NULL,
        .pDepthStencilAttachment = &depthAttachmentRef,
        .preserveAttachmentCount = 0,
        .pPreserveAttachments = NULL
    };

    VkSubpassDependency dependency = {
        .srcSubpass = VK_SUBPASS_EXTERNAL,
        .dstSubpass = 0,
        .srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
        .dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
        .srcAccessMask = 0,
        .dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
        .dependencyFlags = 0
    };

    VkRenderPassCreateInfo renderPassInfo = {
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
        .pNext = NULL,
        .flags = 0,
        .attachmentCount = 2,
        .pAttachments = attachments,
        .subpassCount = 1,
        .pSubpasses = &subpass,
        .dependencyCount = 1,
        .pDependencies = &dependency
    };

    VULKAN.renderPass = malloc(sizeof(VkRenderPass));
    if (vkCreateRenderPass(VULKAN.device, &renderPassInfo, NULL, &VULKAN.renderPass) != VK_SUCCESS) {
        c_throw("failed to create render pass");
    }
}

void createGraphicsPipeline() {
    shaderfile vert = readFile("shaders/vert.spv");
    shaderfile frag = readFile("shaders/frag.spv");

    VkShaderModule vertShaderModule, fragShaderModule;
    vertShaderModule = createShaderModule(vert);
    fragShaderModule = createShaderModule(frag);

    VkPipelineShaderStageCreateInfo vertCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
        .pNext = NULL,
        .flags = 0,
        .stage = VK_SHADER_STAGE_VERTEX_BIT,
        .module = vertShaderModule,
        .pName = "main",
        .pSpecializationInfo = NULL
    };
    VkPipelineShaderStageCreateInfo fragCreateInfo = vertCreateInfo;
    fragCreateInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    fragCreateInfo.module = fragShaderModule;
    VkPipelineShaderStageCreateInfo shaderStages[] = {vertCreateInfo, fragCreateInfo};

    VkVertexInputBindingDescription bindingDescription = getBindDescription();
    VertexAttribDescrStruct attributeDescriptions = getAttributeDescriptions();

    VkPipelineVertexInputStateCreateInfo vertexInputInfo = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
        .pNext = NULL,
        .flags = 0,
        .vertexBindingDescriptionCount = 1,
        .pVertexBindingDescriptions = &bindingDescription,
        .vertexAttributeDescriptionCount = attributeDescriptions.count,
        .pVertexAttributeDescriptions = attributeDescriptions.descrs
    };

    VkPipelineInputAssemblyStateCreateInfo inputAssembly = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
        .pNext = NULL,
        .flags = 0,
        .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
        .primitiveRestartEnable = VK_FALSE
    };

    VkViewport viewport = {
        .x = 0.0f,
        .y = 0.0f,
        .width = (float)VULKAN.swapchainExtent.width,
        .height = (float)VULKAN.swapchainExtent.height,
        .minDepth = 0.0f,
        .maxDepth = 1.0f,
    };

    VkRect2D scissor = {
        .offset = {0,0},
        .extent = VULKAN.swapchainExtent
    };

    VkDynamicState dynamicStates[2] = {
        VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR
    };
    VkPipelineDynamicStateCreateInfo dynamicState = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
        .pNext = NULL,
        .flags = 0,
        .dynamicStateCount = 2,
        .pDynamicStates = dynamicStates
    };

    VkPipelineViewportStateCreateInfo viewportState = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
        .pNext = NULL,
        .flags = 0,
        .viewportCount = 1,
        .pViewports = &viewport,
        .scissorCount = 1,
        .pScissors = &scissor,
    };

    VkPipelineRasterizationStateCreateInfo rasterizer = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
        .pNext = NULL,
        .flags = 0,
        .depthClampEnable = VK_FALSE,
        .rasterizerDiscardEnable = VK_FALSE,
        .polygonMode = VK_POLYGON_MODE_FILL,
        .cullMode = VK_CULL_MODE_BACK_BIT,
        .frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE,
        .depthBiasEnable = VK_FALSE,
        .depthBiasConstantFactor = 0.0f,
        .depthBiasClamp = 0.0f,
        .depthBiasSlopeFactor = 0.0f,
        .lineWidth = 1.0f
    };

    VkPipelineMultisampleStateCreateInfo multisampling = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
        .pNext = NULL,
        .flags = 0,
        .rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
        .sampleShadingEnable = VK_FALSE,
        .minSampleShading = 1.0f,
        .pSampleMask = NULL,
        .alphaToCoverageEnable = VK_FALSE,
        .alphaToOneEnable = VK_FALSE
    };

    VkPipelineColorBlendAttachmentState colorBlendAttachment = {
        .blendEnable = VK_TRUE,
        .srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA,
        .dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
        .colorBlendOp = VK_BLEND_OP_ADD,
        .srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE,
        .dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO,
        .alphaBlendOp = VK_BLEND_OP_ADD,
        .colorWriteMask = VK_COLOR_COMPONENT_R_BIT |
        VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT |
        VK_COLOR_COMPONENT_A_BIT
    };

    VkPipelineColorBlendStateCreateInfo colorBlending = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
        .pNext = NULL,
        .flags = 0,
        .logicOpEnable = VK_TRUE,
        .logicOp = VK_LOGIC_OP_COPY,
        .attachmentCount = 1,
        .pAttachments = &colorBlendAttachment,
        .blendConstants = {0.0f,0.0f,0.0f,0.0f}
    };

    VkPipelineDepthStencilStateCreateInfo depthStencil = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
        .pNext = NULL,
        .flags = 0,
        .depthTestEnable = VK_TRUE,
        .depthWriteEnable = VK_TRUE,
        .depthCompareOp = VK_COMPARE_OP_LESS,
        .depthBoundsTestEnable = VK_FALSE,
        .stencilTestEnable = VK_TRUE,
        .front = {0,0,0,0,0,0,0},
        .back = {0,0,0,0,0,0,0},
        .minDepthBounds = 0.0f,
        .maxDepthBounds = 1.0f
    };

    VkPipelineLayoutCreateInfo pipelineLayoutInfo = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
        .pNext = NULL,
        .flags = 0,
        .setLayoutCount = 1,
        .pSetLayouts = &VULKAN.descriptorSetLayout,
        .pushConstantRangeCount = 0,
        .pPushConstantRanges = NULL 
    };

    VULKAN.pipelineLayout = malloc(sizeof(VkPipelineLayout));
    if (vkCreatePipelineLayout(VULKAN.device, &pipelineLayoutInfo, NULL, &VULKAN.pipelineLayout) != VK_SUCCESS) {
        c_throw("failed to create pipeline layout");
    }

    VkGraphicsPipelineCreateInfo pipelineInfo = {
        .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
        .pNext = NULL,
        .flags = 0,
        .stageCount = 2,
        .pStages = shaderStages,
        .pVertexInputState = &vertexInputInfo,
        .pInputAssemblyState = &inputAssembly,
        .pTessellationState = NULL,
        .pViewportState = &viewportState,
        .pRasterizationState = &rasterizer,
        .pMultisampleState = &multisampling,
        .pDepthStencilState = &depthStencil,
        .pColorBlendState = &colorBlending,
        .pDynamicState = &dynamicState,
        .layout = VULKAN.pipelineLayout,
        .renderPass = VULKAN.renderPass,
        .subpass = 0,
        .basePipelineHandle = VK_NULL_HANDLE,
        .basePipelineIndex = -1
    };

    VULKAN.pipeline = malloc(sizeof(VkPipeline));
    if (vkCreateGraphicsPipelines(VULKAN.device, VK_NULL_HANDLE, 1, &pipelineInfo, NULL, &VULKAN.pipeline) != VK_SUCCESS) {
        c_throw("failed to create graphics pipeline");
    }

    vkDestroyShaderModule(VULKAN.device, vertShaderModule, NULL);
    vkDestroyShaderModule(VULKAN.device, fragShaderModule, NULL);
}

shaderfile readFile(const char* filename) {
    shaderfile result = {NULL, 0};
    FILE* file;
    file = fopen(filename, "rb");

    if (file == NULL) {
        c_throw("can't find file for readFile func");
    }
    
    fseek(file, 0, SEEK_END);
    result.size = ftell(file);
    result.file = (char*)malloc( (result.size%4) == 0 ? result.size : result.size + (4-(result.size%4)) );
    fseek(file,0, SEEK_SET);
    for (uint32_t i = 0; i < result.size; ++i) {
        result.file[i] =  feof(file) ? '\0' : (char) fgetc(file) ;
    }

    fclose(file);

    return result;
}

VkShaderModule createShaderModule(shaderfile file) {
    VkShaderModuleCreateInfo createInfo = {
        .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
        .pNext = NULL,
        .flags = 0,
        .pCode = (uint32_t*) file.file,
        .codeSize = file.size
    };
    
    VkShaderModule shaderModule = malloc(sizeof(VkShaderModule));
    if (vkCreateShaderModule(VULKAN.device, &createInfo, NULL, &shaderModule) != VK_SUCCESS) {
        c_throw("can't create shader module");
    }
    return shaderModule;
}

void createFramebuffers() {
    VULKAN.framebufferResized = false;
    VULKAN.swapchainFramebuffers.f = malloc(sizeof(VkFramebuffer) * VULKAN.swapchainImageViews.count);
    VULKAN.swapchainFramebuffers.count = VULKAN.swapchainImageViews.count;

    for (size_t i = 0; i < VULKAN.swapchainFramebuffers.count; ++i) {
        VkImageView attachments[] = {
            VULKAN.swapchainImageViews.swapChainImageViews[i],
            VULKAN.depthImageView
        };

        VkFramebufferCreateInfo framebufferInfo = {
            .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
            .pNext = NULL,
            .flags = 0,
            .renderPass = VULKAN.renderPass,
            .attachmentCount = 2,
            .pAttachments = attachments,
            .width = VULKAN.swapchainExtent.width,
            .height = VULKAN.swapchainExtent.height,
            .layers = 1
        };

        if (vkCreateFramebuffer(VULKAN.device, &framebufferInfo, NULL, VULKAN.swapchainFramebuffers.f+i) != VK_SUCCESS) {
            c_throw("failed to create framebuffer");
        }
    }
}

void createCommandPool() {
    QueueFamilyIndices queueFamilyIndices = findQueueFamilies(VULKAN.physicalDevice);

    VkCommandPoolCreateInfo poolInfo = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        .pNext = NULL,
        .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
        .queueFamilyIndex = queueFamilyIndices.graphicsFamily
    };

    if (vkCreateCommandPool(VULKAN.device, &poolInfo, NULL, &VULKAN.commandPool) != VK_SUCCESS) {
        c_throw("failed to create command pool");
    }
}

void createCommandBuffers() {
    VULKAN.commandBuffer = malloc(sizeof(VkCommandBuffer) * MAX_FRAMES_IN_FLIGHT);
    VkCommandBufferAllocateInfo allocInfo = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .pNext = NULL,
        .commandPool = VULKAN.commandPool,
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount = (uint32_t) MAX_FRAMES_IN_FLIGHT
    };
    if (vkAllocateCommandBuffers(VULKAN.device, &allocInfo, VULKAN.commandBuffer)) {
        c_throw("failed to allocate command buffers");
    };
}

void recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex) {
    VkCommandBufferBeginInfo beginInfo = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .pNext = NULL,
        .flags = 0,
        .pInheritanceInfo = NULL
    };
    if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
        c_throw("failed to begin a command buffer");
    }

    VkClearValue clearColor[] = {
        {.color = {0.0f,0.0f,0.0f,1.0f},.depthStencil = {0.0f, 0}},
        {.color = {0.0f,0.0f,0.0f,0.0f},.depthStencil = {1.0f, 0}}
    };
    VkRenderPassBeginInfo renderPassInfo = {
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
        .pNext = NULL,
        .renderPass = VULKAN.renderPass,
        .framebuffer = VULKAN.swapchainFramebuffers.f[imageIndex],
        .renderArea = {
            .offset = {0,0},
            .extent = VULKAN.swapchainExtent
        },
        .clearValueCount = 2,
        .pClearValues = clearColor,
    };
    vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, VULKAN.pipeline);

    // = VIEWPORTING AND SCISSORING =
    VkViewport viewport = {
        .x = 0.0f,
        .y = 0.0f,
        .width = (float) VULKAN.swapchainExtent.width,
        .height = (float) VULKAN.swapchainExtent.height ,
        .minDepth = 0.0f,
        .maxDepth = 1.0f
    };
    vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

    VkRect2D scissor = {
        .offset = {0,0},
        .extent = VULKAN.swapchainExtent
    };
    vkCmdSetScissor(commandBuffer, 0, 1, &scissor);
    // =============================

    VkBuffer vertexBuffers[] = { VULKAN.vertexBuffer };
    VkDeviceSize offsets[] = {0};
    vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);
    vkCmdBindIndexBuffer(commandBuffer, VULKAN.indexBuffer, 0, VK_INDEX_TYPE_UINT32);

    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, VULKAN.pipelineLayout,
        0, 1, &VULKAN.descriptorSets[VULKAN.currentFrame], 0, NULL);

    vkCmdDrawIndexed(commandBuffer, INDICES_COUNT, 1, 0, 0, 0);

    vkCmdEndRenderPass(commandBuffer);

    if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
        c_throw("fauled to record command buffer");
    }
}

void createSyncObjects() {
    VULKAN.currentFrame = 0;
    VULKAN.imageAvailableSemaphore = malloc(sizeof(VkSemaphore) * MAX_FRAMES_IN_FLIGHT);
    VULKAN.renderFinishedSemaphore = malloc(sizeof(VkSemaphore) * MAX_FRAMES_IN_FLIGHT);
    VULKAN.inFlightFence = malloc(sizeof(VkFence) * MAX_FRAMES_IN_FLIGHT);

    VkSemaphoreCreateInfo semaphoreInfo = {
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
        .pNext = NULL,
        .flags = 0
    };
    VkFenceCreateInfo fenceInfo = {
        .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
        .pNext = NULL,
        .flags = VK_FENCE_CREATE_SIGNALED_BIT
    };

    for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i) {
        if (vkCreateSemaphore(VULKAN.device, &semaphoreInfo, NULL, VULKAN.imageAvailableSemaphore+i) != VK_SUCCESS ||
            vkCreateSemaphore(VULKAN.device, &semaphoreInfo, NULL, VULKAN.renderFinishedSemaphore+i) != VK_SUCCESS ||
            vkCreateFence(VULKAN.device, &fenceInfo, NULL, VULKAN.inFlightFence+i) != VK_SUCCESS) {
            c_throw("failed to create semaphores");
        }
    }
}

void recreateSwapchain() {
    int width = 0, height = 0;
    glfwGetFramebufferSize(WINDOW.window, &width, &height);
    while (width == 0 || height == 0) {
        glfwGetFramebufferSize(WINDOW.window, &width, &height);
        glfwWaitEvents();
    }

    vkDeviceWaitIdle(VULKAN.device);

    clearupSwapchain();

    createSwapChain();
    createImageViews();
    createDepthResources();
    createFramebuffers();
}

void clearupSwapchain() {
    vkDestroyImageView(VULKAN.device, VULKAN.depthImageView, NULL);
    vkDestroyImage(VULKAN.device, VULKAN.depthImage, NULL);
    vkFreeMemory(VULKAN.device, VULKAN.depthImageMemory, NULL);

    for (size_t i = 0; i < VULKAN.swapchainFramebuffers.count; ++i) {
        vkDestroyFramebuffer(VULKAN.device, VULKAN.swapchainFramebuffers.f[i], NULL);
    }

    for (uint32_t i = 0; i < VULKAN.swapchainImageViews.count; ++i) {
        vkDestroyImageView(VULKAN.device, VULKAN.swapchainImageViews.swapChainImageViews[i], NULL);
    }

    vkDestroySwapchainKHR(VULKAN.device, VULKAN.swapchain, NULL);
}

void createVertexBuffer() {
    VkDeviceSize bufferSize = sizeof(vertices[0]) * VERTICES_COUNT;

    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;

    createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        &stagingBuffer, &stagingBufferMemory);

    void* data;// = malloc(bufferSize);
    vkMapMemory(VULKAN.device, stagingBufferMemory, 0, bufferSize, 0, &data);
    memcpy(data, vertices, (size_t)bufferSize);
    vkUnmapMemory(VULKAN.device, stagingBufferMemory);

    createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, 
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &VULKAN.vertexBuffer, &VULKAN.vertexBufferMemory);

    copyBuffer(stagingBuffer, VULKAN.vertexBuffer, bufferSize);

    vkDestroyBuffer(VULKAN.device, stagingBuffer, NULL);
    vkFreeMemory(VULKAN.device, stagingBufferMemory, NULL);
}

void createIndexBuffer() {
    VkDeviceSize bufferSize = sizeof(indices[0]) * INDICES_COUNT;

    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        &stagingBuffer, &stagingBufferMemory);

    void* data; // = malloc(bufferSize);
    vkMapMemory(VULKAN.device, stagingBufferMemory, 0, bufferSize, 0, &data);
    memcpy(data, indices, bufferSize);
    vkUnmapMemory(VULKAN.device, stagingBufferMemory);

    createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &VULKAN.indexBuffer, &VULKAN.indexBufferMemory);
    copyBuffer(stagingBuffer, VULKAN.indexBuffer, bufferSize);

    vkDestroyBuffer(VULKAN.device, stagingBuffer, NULL);
    vkFreeMemory(VULKAN.device, stagingBufferMemory, NULL);
}

uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) {
    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(VULKAN.physicalDevice, &memProperties);

    for (uint32_t i = 0; i < memProperties.memoryTypeCount; ++i) {
        if ( (typeFilter & (1 << i)) &&
            ((memProperties.memoryTypes[i].propertyFlags & properties) == properties)) {
            return i;
        }
    }

    c_throw("failed to find suitable memory type");
    return UINT32_MAX;
}

void createBuffer(
    VkDeviceSize size, VkBufferUsageFlags usage,
    VkMemoryPropertyFlags properties, VkBuffer* buffer,
    VkDeviceMemory* bufferMemory) {
    
    VkBufferCreateInfo bufferInfo = {
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .pNext = NULL,
        .flags = 0,
        .size = size,
        .usage = usage,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
        .queueFamilyIndexCount = 0,
        .pQueueFamilyIndices = NULL
    };

    if (vkCreateBuffer(VULKAN.device, &bufferInfo, NULL, buffer) != VK_SUCCESS) {
        c_throw("failed to create vertex buffer");
    }

    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(VULKAN.device, *buffer, &memRequirements);

    VkMemoryAllocateInfo allocInfo = {
        .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
        .pNext = NULL,
        .allocationSize = memRequirements.size,
        .memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties)
    };

    if (vkAllocateMemory(VULKAN.device, &allocInfo, NULL, bufferMemory) != VK_SUCCESS) {
        c_throw("failed to allocate vertex buffer memory");
    }

    vkBindBufferMemory(VULKAN.device, *buffer, *bufferMemory, 0);
}

void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size) {
    VkCommandBuffer commandBuffer = beginSingleTimeCommands();

    VkBufferCopy copyRegion = {
        .srcOffset = 0,
        .dstOffset = 0,
        .size = size
    };
    vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

    endSingleTimeCommands(commandBuffer);
}

void createDescriptorSetLayout() {
    VkDescriptorSetLayoutBinding uboLayoutBinding = {
        .binding = 0,
        .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
        .descriptorCount = 1,
        .stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
        .pImmutableSamplers = NULL
    };

    VkDescriptorSetLayoutBinding samplerLayoutBinding = {
        .binding = 1,
        .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
        .descriptorCount = 1,
        .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
        .pImmutableSamplers = NULL
    };

    VkDescriptorSetLayoutBinding bindings[] = {
        uboLayoutBinding,
        samplerLayoutBinding
    };

    VkDescriptorSetLayoutCreateInfo layoutInfo = {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
        .pNext = NULL,
        .flags = 0,
        .bindingCount = 2,
        .pBindings = bindings
    };

    if (vkCreateDescriptorSetLayout(VULKAN.device, &layoutInfo, NULL, &VULKAN.descriptorSetLayout) != VK_SUCCESS) {
        c_throw("failed to create descriptor set layout");
    }
}

void createUniformBuffers() {
    VkDeviceSize bufferSize = sizeof(UniformBufferObject);

    VULKAN.uniformBuffers = malloc(sizeof(VkBuffer*) * MAX_FRAMES_IN_FLIGHT);
    VULKAN.uniformBuffersMemory = malloc(sizeof(VkDeviceMemory*) * MAX_FRAMES_IN_FLIGHT);
    VULKAN.uniformBuffersMapped = malloc(sizeof(void**) * MAX_FRAMES_IN_FLIGHT);

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i) {
        createBuffer(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
            VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, VULKAN.uniformBuffers+i, VULKAN.uniformBuffersMemory+i);
        vkMapMemory(VULKAN.device, VULKAN.uniformBuffersMemory[i], 0, bufferSize, 0, VULKAN.uniformBuffersMapped + i);
    }

}

bool firstTime = true;
void updateUniformBuffer(uint32_t currentImage) {
    UniformBufferObject ubo = {
        GLM_MAT4_IDENTITY_INIT,GLM_MAT4_ZERO_INIT,GLM_MAT4_ZERO_INIT
    };
    static mat4 s_model;
    if (firstTime) { 
        glm_mat4_copy(GLM_MAT4_IDENTITY, s_model);
        firstTime = false;
    }

    vec3 eye = { 2.0f, 2.0f, 2.0f };
    vec3 center = { 0.0f, 0.0f, 0.0f };
    vec3 up = { 0.0f, 0.0f, 1.0f };
    
    glm_rotate_z(s_model, glm_rad(3.0f), ubo.model);
    glm_mat4_copy(ubo.model, s_model);
    glm_lookat(eye, center, up, ubo.view);
    glm_perspective(glm_rad(45.0f),
        (float)VULKAN.swapchainExtent.width / (float)VULKAN.swapchainExtent.height,
        0.0f, 1.0f, ubo.proj);

    ubo.proj[1][1] *= -1;

    memcpy(VULKAN.uniformBuffersMapped[currentImage], &ubo, sizeof(ubo));
}

void createDescriptorPool() {
    VkDescriptorPoolSize poolSizes[] = {
        {
            .type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
            .descriptorCount = MAX_FRAMES_IN_FLIGHT
        },
        {
            .type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
            .descriptorCount = MAX_FRAMES_IN_FLIGHT
        }
    };

    VkDescriptorPoolCreateInfo poolInfo = {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
        .pNext = NULL,
        .flags = 0,
        .maxSets = MAX_FRAMES_IN_FLIGHT,
        .poolSizeCount = 2,
        .pPoolSizes = poolSizes
    };

    if (vkCreateDescriptorPool(VULKAN.device, &poolInfo, NULL, &VULKAN.descriptorPool) != VK_SUCCESS) {
        c_throw("failed to create descriptor pool");
    };
}

void createDescriptorSets() {
    VkDescriptorSetLayout layouts[2] = {VULKAN.descriptorSetLayout, VULKAN.descriptorSetLayout };

    VkDescriptorSetAllocateInfo allocInfo = {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
        .pNext = NULL,
        .descriptorPool = VULKAN.descriptorPool,
        .descriptorSetCount = MAX_FRAMES_IN_FLIGHT,
        .pSetLayouts = layouts
    };

    VULKAN.descriptorSets = malloc(sizeof(VkDescriptorSet) * MAX_FRAMES_IN_FLIGHT);
    if (vkAllocateDescriptorSets(VULKAN.device, &allocInfo, VULKAN.descriptorSets) != VK_SUCCESS) {
        c_throw("failed to allocate descriptor sets");
    };

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i) {
        VkDescriptorBufferInfo bufferInfo = {
            .buffer = VULKAN.uniformBuffers[i],
            .offset = 0,
            .range = sizeof(UniformBufferObject)
        };
        VkDescriptorImageInfo imageInfo = {
            .sampler = VULKAN.textureSampler,
            .imageView = VULKAN.textureImageView,
            .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
        };
        VkWriteDescriptorSet descriptorWrite[] = {
            {
                .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                .pNext = NULL,
                .dstSet = VULKAN.descriptorSets[i],
                .dstBinding = 0,
                .dstArrayElement = 0,
                .descriptorCount = 1,
                .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                .pImageInfo = NULL,
                .pBufferInfo = &bufferInfo,
                .pTexelBufferView = NULL
            },
            {
                .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                .pNext = NULL,
                .dstSet = VULKAN.descriptorSets[i],
                .dstBinding = 1,
                .dstArrayElement = 0,
                .descriptorCount = 1,
                .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                .pImageInfo = &imageInfo,
                .pBufferInfo = NULL,
                .pTexelBufferView = NULL
            }
        };
        vkUpdateDescriptorSets(VULKAN.device, 2, descriptorWrite, 0, NULL);
    }
}

void createTextureImage() {
    int tWidth, tHeight, tChannels;
    stbi_uc* pixels = stbi_load("textures/texture.png", &tWidth, &tHeight, &tChannels, STBI_rgb_alpha);
    VkDeviceSize imageSize = tWidth * tHeight * tChannels;

    if (!pixels) {
        c_throw("failed to load texture image");
    }

    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    createBuffer(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
        VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &stagingBuffer, &stagingBufferMemory);

    void* data;
    vkMapMemory(VULKAN.device, stagingBufferMemory, 0, imageSize, 0, &data);
    memcpy(data, pixels, (size_t)imageSize);
    vkUnmapMemory(VULKAN.device, stagingBufferMemory);

    stbi_image_free(pixels);

    createImage((uint32_t)tWidth, (uint32_t)tHeight, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_TILING_OPTIMAL,
        VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        &VULKAN.textureImage, &VULKAN.textureImageMemory);

    transitionImageLayout(VULKAN.textureImage, VK_FORMAT_R8G8B8A8_SRGB,
        VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
    copyBufferToImage(stagingBuffer, VULKAN.textureImage, (uint32_t)tWidth, (uint32_t)tHeight);
    transitionImageLayout(VULKAN.textureImage, VK_FORMAT_R8G8B8A8_SRGB,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    vkDestroyBuffer(VULKAN.device, stagingBuffer, NULL);
    vkFreeMemory(VULKAN.device, stagingBufferMemory, NULL);
}

void createImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling,
    VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage* image, VkDeviceMemory* imageMemory) {
    VkImageCreateInfo imageInfo = {
        .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
        .pNext = NULL,
        .flags = 0,
        .imageType = VK_IMAGE_TYPE_2D,
        .format = format,
        .extent = {
            .width = width,
            .height = height,
            .depth = 1
            },
        .mipLevels = 1,
        .arrayLayers = 1,
        .samples = VK_SAMPLE_COUNT_1_BIT,
        .tiling = tiling,
        .usage = usage,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
        .queueFamilyIndexCount = 0,
        .pQueueFamilyIndices = NULL,
        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED
    };

    if (vkCreateImage(VULKAN.device, &imageInfo, NULL, image) != VK_SUCCESS) {
        c_throw("failed to create image");
    }

    VkMemoryRequirements memReq;
    vkGetImageMemoryRequirements(VULKAN.device, *image, &memReq);

    VkMemoryAllocateInfo allocInfo = {
        .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
        .pNext = NULL,
        .allocationSize = memReq.size,
        .memoryTypeIndex = findMemoryType(memReq.memoryTypeBits, properties)
    };

    if (vkAllocateMemory(VULKAN.device, &allocInfo, NULL, imageMemory) != VK_SUCCESS) {
        c_throw("failed to alloc texture image memory");
    }
    vkBindImageMemory(VULKAN.device, *image, *imageMemory, 0);
}

VkCommandBuffer beginSingleTimeCommands() {
    VkCommandBufferAllocateInfo allocInfo = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .pNext = NULL,
        .commandPool = VULKAN.commandPool,
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount = 1
    };

    VkCommandBuffer commandBuffer;
    vkAllocateCommandBuffers(VULKAN.device, &allocInfo, &commandBuffer);

    VkCommandBufferBeginInfo beginInfo = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .pNext = NULL,
        .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
        .pInheritanceInfo = NULL
    };

    vkBeginCommandBuffer(commandBuffer, &beginInfo);
    
    return commandBuffer;
}

void endSingleTimeCommands(VkCommandBuffer commandBuffer) {
    vkEndCommandBuffer(commandBuffer);

    VkSubmitInfo submitInfo = {
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .pNext = NULL,
        .waitSemaphoreCount = 0,
        .pWaitSemaphores = NULL,
        .pWaitDstStageMask = NULL,
        .commandBufferCount = 1,
        .pCommandBuffers = &commandBuffer,
        .signalSemaphoreCount = 0,
        .pSignalSemaphores = NULL
    };

    vkQueueSubmit(VULKAN.graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(VULKAN.graphicsQueue);

    vkFreeCommandBuffers(VULKAN.device, VULKAN.commandPool, 1, &commandBuffer);
}

void transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout) {
    VkCommandBuffer commandBuffer = beginSingleTimeCommands();

    VkImageMemoryBarrier barrier = {
        .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
        .pNext = NULL,
        .srcAccessMask = 0,
        .dstAccessMask = 0,
        .oldLayout = oldLayout,
        .newLayout = newLayout,
        .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .image = image,
        .subresourceRange = {
            .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
            .baseMipLevel = 0,
            .levelCount = 1,
            .baseArrayLayer = 0,
            .layerCount = 1
            }
    };

    VkPipelineStageFlags srcStage, dstStage;

    if (newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
        if (hasStancilComponent(format)) {
            barrier.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
        }
    } else {
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    }

    if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        srcStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        dstStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    } else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
        srcStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        dstStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    } else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
        srcStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        dstStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    } else {
        c_throw("unsupported layour transition");
    }

    vkCmdPipelineBarrier(commandBuffer,
        srcStage,dstStage,
        0,0,NULL,0,NULL,
        1,&barrier);

    endSingleTimeCommands(commandBuffer);
}

void copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height) {
    VkCommandBuffer commandBuffer = beginSingleTimeCommands();

    VkBufferImageCopy region = {
        .bufferOffset = 0,
        .bufferRowLength = 0,
        .bufferImageHeight = 0,
        .imageSubresource = {
            .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
            .mipLevel = 0,
            .baseArrayLayer = 0,
            .layerCount = 1
            },
        .imageOffset = {0,0,0},
        .imageExtent = {width, height, 1}
    };

    vkCmdCopyBufferToImage(commandBuffer,
        buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

    endSingleTimeCommands(commandBuffer);
}

void createTextureImageView() {
    VULKAN.textureImageView = createImageView(VULKAN.textureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_ASPECT_COLOR_BIT);
}

VkImageView createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags) {
    VkImageViewCreateInfo viewInfo = {
        .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
        .pNext = NULL,
        .flags = 0,
        .image = image,
        .viewType = VK_IMAGE_VIEW_TYPE_2D,
        .format = format,
        .components = {0,0,0,0},
        .subresourceRange = {
            .aspectMask = aspectFlags,
            .baseMipLevel = 0,
            .levelCount = 1,
            .baseArrayLayer = 0,
            .layerCount = 1
            }
    };

    VkImageView imageView;
    if (vkCreateImageView(VULKAN.device, &viewInfo, NULL, &imageView) != VK_SUCCESS) {
        c_throw("failed to create texture image view");
    }

    return imageView;
}

void createTextureSampler() {
    VkPhysicalDeviceProperties properties;
    vkGetPhysicalDeviceProperties(VULKAN.physicalDevice, &properties);

    VkSamplerCreateInfo samplerInfo = {
        .sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
        .pNext = NULL,
        .flags = 0,
        .magFilter = VK_FILTER_LINEAR,
        .minFilter = VK_FILTER_LINEAR,
        .mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR,
        .addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER,
        .addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER,
        .addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER,
        .mipLodBias = 0.0f,
        .anisotropyEnable = VK_TRUE,
        .maxAnisotropy = properties.limits.maxSamplerAnisotropy,
        .compareEnable = VK_FALSE,
        .compareOp = VK_COMPARE_OP_ALWAYS,
        .minLod = 0.0f,
        .maxLod = 0.0f,
        .borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK,
        .unnormalizedCoordinates = VK_FALSE
    };
    
    if (vkCreateSampler(VULKAN.device, &samplerInfo, NULL, &VULKAN.textureSampler) != VK_SUCCESS) {
        c_throw("failed to create texture sampler");
    }
}

void createDepthResources() {
    VkFormat depthFormat = findDepthFormat();
    createImage(VULKAN.swapchainExtent.width, VULKAN.swapchainExtent.height, depthFormat, VK_IMAGE_TILING_OPTIMAL,
        VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        &VULKAN.depthImage, &VULKAN.depthImageMemory);
    VULKAN.depthImageView = createImageView(VULKAN.depthImage, depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT);

    transitionImageLayout(VULKAN.depthImage, depthFormat, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
}

VkFormat findSupportedFormat(const VkFormat* candidates, uint32_t candidatesCount,
    VkImageTiling tiling, VkFormatFeatureFlags features) {
    for (uint32_t i = 0; i < candidatesCount; ++i) {
        VkFormatProperties props;
        vkGetPhysicalDeviceFormatProperties(VULKAN.physicalDevice, candidates[i], &props);

        if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features) {
            return candidates[i];
        } else if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features) {
            return candidates[i];
        }
    }
    c_throw("failed to find supported format");
    return VK_FORMAT_MAX_ENUM;
}

VkFormat findDepthFormat() {
    VkFormat cands[] = { VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT };
    return findSupportedFormat(
        cands, 3,
        VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
}

bool hasStancilComponent(VkFormat format) {
    return format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT;
}

void framebufferResizeCallback(GLFWwindow* window, int width, int height) {
    VULKAN.framebufferResized = true;
}

//  VALIDATION THINGS IMPLEMENTATION

uint32_t checkValidationLayersSupport() {
    uint32_t layerCount;
    vkEnumerateInstanceLayerProperties(&layerCount, NULL);

    VkLayerProperties* availableLayers = malloc(sizeof(VkLayerProperties) * layerCount);
    vkEnumerateInstanceLayerProperties(&layerCount, availableLayers);

    int found = 0;
    for (int i = 0; i < VALIDATION_LAYERS; ++i) {
        for (uint32_t j = 0; j < layerCount; ++j) {
            if (strcmp(validationLayers[i], availableLayers[j].layerName) == 0) {
                found = 1;
                break;
            }
        }
        if (!found) return 0;
    }

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
    fprintf(stderr, "\n| Validation layer |\n%s\n", pCallbackData->pMessage);
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