#include "vertexes.h"

VkVertexInputBindingDescription getBindDescription() {
	VkVertexInputBindingDescription bindingDescription = {
		.binding = 0,
		.stride = sizeof(Vertex),
		.inputRate = VK_VERTEX_INPUT_RATE_VERTEX
	};

	return bindingDescription;
}

VertexAttribDescrStruct getAttributeDescriptions() {
	VertexAttribDescrStruct attributeDescriptions;
	attributeDescriptions.count = 3;
	attributeDescriptions.descrs = malloc(sizeof(VkVertexInputAttributeDescription) * attributeDescriptions.count);

	attributeDescriptions.descrs[0].location = 0;
	attributeDescriptions.descrs[0].binding = 0;
	attributeDescriptions.descrs[0].format = VK_FORMAT_R32G32B32_SFLOAT;
	attributeDescriptions.descrs[0].offset = offsetof(Vertex, pos);

	attributeDescriptions.descrs[1].location = 1;
	attributeDescriptions.descrs[1].binding = 0;
	attributeDescriptions.descrs[1].format = VK_FORMAT_R32G32B32A32_SFLOAT;
	attributeDescriptions.descrs[1].offset = offsetof(Vertex, color);

	attributeDescriptions.descrs[2].location = 2;
	attributeDescriptions.descrs[2].binding = 0;
	attributeDescriptions.descrs[2].format = VK_FORMAT_R32G32B32_SFLOAT;
	attributeDescriptions.descrs[2].offset = offsetof(Vertex, texCoord);

	return attributeDescriptions;
}
