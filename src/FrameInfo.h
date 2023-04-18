#pragma once

#include <vulkan/vulkan.h>
#include "Camera.h"
#include "GameObject.h"

struct GlobalUbo
{
	glm::mat4 projection {1.f};
	glm::mat4 view {1.f};
	glm::mat4 inverseView {1.f};
	glm::vec4 ambientLightColor {1.f, 1.f, 1.f, .2f};  // w is intensity
};

struct FrameInfo
{
	int frameIndex;
	float frameTime;
	VkCommandBuffer commandBuffer;
	CCamera &camera;
	VkDescriptorSet globalDescriptorSet;
	CGameObject::Map &gameObjects;
};
