#pragma once

#include "Buffer.h"
#include "Device.h"

// libs
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE

#include <glm/glm.hpp>


#include <memory>
#include <vector>


class CModel
{
public:
	struct Vertex
	{
		glm::vec3 position {};
		glm::vec3 color {};
		glm::vec3 normal {};
		glm::vec2 uv {};

		static std::vector<VkVertexInputBindingDescription> getBindingDescriptions ();

		static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions ();

		bool operator== (const Vertex &other) const
		{
			return position == other.position && color == other.color && normal == other.normal && uv == other.uv;
		}
	};

	struct Builder
	{
		std::vector<Vertex> vertices {};
		std::vector<uint32_t> indices {};

		void loadModel (const std::string &filepath);
	};

	CModel (CDevice &device, const CModel::Builder &builder);

	~CModel ();

	static std::unique_ptr<CModel> createModelFromFile (CDevice &device, const std::string &filepath);

	void bind (VkCommandBuffer commandBuffer);

	void draw (VkCommandBuffer commandBuffer);

private:
	void createVertexBuffers (const std::vector<Vertex> &vertices);

	void createIndexBuffers (const std::vector<uint32_t> &indices);

	CDevice &Device;

	std::unique_ptr<CBuffer> vertexBuffer;
	uint32_t vertexCount;

	bool hasIndexBuffer = false;
	std::unique_ptr<CBuffer> indexBuffer;
	uint32_t indexCount;
};

