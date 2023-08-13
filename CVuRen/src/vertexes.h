#pragma once

#define CGLM_FORCE_DEPTH_ZERO_TO_ONE

#include <vulkan/vulkan.h>
#include "cglm/cglm.h"

#include <stdalign.h>

typedef struct Vertex {
	vec3 pos;
	vec4 color;
	vec3 texCoord;
} Vertex;

typedef struct VertexAttribDescrStruct {
	VkVertexInputAttributeDescription* descrs;
	uint32_t count;
} VertexAttribDescrStruct;

VkVertexInputBindingDescription getBindDescription();
VertexAttribDescrStruct getAttributeDescriptions();

typedef struct UniformBufferObject {
	alignas(16) mat4 model, view, proj;
} UniformBufferObject;