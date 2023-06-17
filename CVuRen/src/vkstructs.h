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