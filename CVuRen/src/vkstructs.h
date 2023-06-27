#pragma once

#include "vulkan/vulkan.h"

typedef struct vkimages {
	VkImage* swapchainImages;
	uint32_t count;
} vkimages;

typedef struct vkimageviews {
	VkImageView* swapChainImageViews;
	uint32_t count;
} vkimageviews;

typedef struct shaderfile {
	char* file;
	size_t size;
} shaderfile;

typedef struct framebuffer {
	VkFramebuffer* f;
	size_t count;
} framebuffer;