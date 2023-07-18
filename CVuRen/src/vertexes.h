#pragma once

#include <vulkan/vulkan.h>
#include "cglm/cglm.h"

typedef struct Vertex {
	vec3 pos;
	vec4 color;
} Vertex;

typedef struct VertexAttribDescrStruct {
	VkVertexInputAttributeDescription* descrs;
	uint32_t count;
} VertexAttribDescrStruct;

VkVertexInputBindingDescription getBindDescription();
VertexAttribDescrStruct getAttributeDescriptions();