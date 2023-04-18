#pragma once

#include "Camera.h"
#include "Device.h"
#include "FrameInfo.h"
#include "GameObject.h"
#include "Pipeline.h"


#include <memory>
#include <vector>


class CSimpleRenderSystem
{
public:
	CSimpleRenderSystem (CDevice& device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout);
	~CSimpleRenderSystem ();

	void renderGameObjects (FrameInfo &frameInfo);

private:
	void createPipelineLayout (VkDescriptorSetLayout globalSetLayout);

	void createPipeline (VkRenderPass renderPass);

	CDevice& Device;

	std::unique_ptr<CPipeline> Pipeline;
	VkPipelineLayout pipelineLayout;
};

