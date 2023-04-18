#pragma once

#include "Device.h"
#include "SwapChain.h"
#include "Window.h"


#include <cassert>
#include <memory>
#include <vector>


class CRenderer
{
public:
	CRenderer (CWindow &window, CDevice& device);
	~CRenderer ();

	VkRenderPass getSwapChainRenderPass () const
	{
		return SwapChain->getRenderPass();
	}

	float getAspectRatio () const
	{
		return SwapChain->extentAspectRatio();
	}

	bool isFrameInProgress () const
	{
		return isFrameStarted;
	}

	VkCommandBuffer getCurrentCommandBuffer () const
	{
		assert(isFrameStarted && "Cannot get command buffer when frame not in progress");
		return commandBuffers[currentFrameIndex];
	}

	int getFrameIndex () const
	{
		assert(isFrameStarted && "Cannot get frame index when frame not in progress");
		return currentFrameIndex;
	}

	VkCommandBuffer beginFrame ();

	void endFrame ();

	void beginSwapChainRenderPass (VkCommandBuffer commandBuffer);

	void endSwapChainRenderPass (VkCommandBuffer commandBuffer);

private:
	void createCommandBuffers ();

	void freeCommandBuffers ();

	void recreateSwapChain ();

	CWindow &Window;
	CDevice& Device;
	std::unique_ptr<CSwapChain> SwapChain;
	std::vector<VkCommandBuffer> commandBuffers;

	uint32_t currentImageIndex;
	int currentFrameIndex {0};
	bool isFrameStarted {false};
};

