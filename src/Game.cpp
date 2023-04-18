#include "Game.h"

#include "KeyboardInput.h"
#include "Buffer.h"
#include "Camera.h"
#include "systems/SimpleSystem.h"

// libs
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE

#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

// std
#include <array>
#include <cassert>
#include <chrono>
#include <stdexcept>


CFirstApp::CFirstApp ()
{
	globalPool = CDescriptorPool::CBuilder(Device).setMaxSets(CSwapChain::MAX_FRAMES_IN_FLIGHT).addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, CSwapChain::MAX_FRAMES_IN_FLIGHT).build();
	loadGameObjects();
}

CFirstApp::~CFirstApp ()
{
}

void CFirstApp::run ()
{
	std::vector<std::unique_ptr<CBuffer>> uboBuffers(CSwapChain::MAX_FRAMES_IN_FLIGHT);
	for (int i = 0; i < uboBuffers.size(); i++)
	{
		uboBuffers[i] = std::make_unique<CBuffer>(Device, sizeof(GlobalUbo), 1, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
		uboBuffers[i]->map();
	}

	auto globalSetLayout = CDescriptorSetLayout::CBuilder(Device).addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS).build();

	std::vector<VkDescriptorSet> globalDescriptorSets(CSwapChain::MAX_FRAMES_IN_FLIGHT);
	for (int i = 0; i < globalDescriptorSets.size(); i++)
	{
		auto bufferInfo = uboBuffers[i]->descriptorInfo();
		CDescriptorWriter(*globalSetLayout, *globalPool).writeBuffer(0, &bufferInfo).build(globalDescriptorSets[i]);
	}

	CSimpleRenderSystem simpleRenderSystem {Device, Renderer.getSwapChainRenderPass(), globalSetLayout->getDescriptorSetLayout()};
	CCamera camera {};

	auto viewerObject = CGameObject::createGameObject();
	viewerObject.transform.translation.z = -2.5f;
	CKeyboardMovementController cameraController {};

	auto currentTime = std::chrono::high_resolution_clock::now();
	while (!Window.shouldClose())
	{
		glfwPollEvents();

		auto newTime = std::chrono::high_resolution_clock::now();
		float frameTime = std::chrono::duration<float, std::chrono::seconds::period>(newTime - currentTime).count();
		currentTime = newTime;

		cameraController.moveInPlaneXZ(Window.getGLFWwindow(), frameTime, viewerObject);
		camera.setViewYXZ(viewerObject.transform.translation, viewerObject.transform.rotation);

		float aspect = Renderer.getAspectRatio();
		camera.setPerspectiveProjection(glm::radians(50.f), aspect, 0.1f, 100.f);

		if (auto commandBuffer = Renderer.beginFrame())
		{
			int frameIndex = Renderer.getFrameIndex();
			FrameInfo frameInfo {frameIndex, frameTime, commandBuffer, camera, globalDescriptorSets[frameIndex], gameObjects};

			// update
			GlobalUbo ubo {};
			ubo.projection = camera.getProjection();
			ubo.view = camera.getView();
			ubo.inverseView = camera.getInverseView();
			uboBuffers[frameIndex]->writeToBuffer(&ubo);
			uboBuffers[frameIndex]->flush();

			// render
			Renderer.beginSwapChainRenderPass(commandBuffer);

			// order here matters
			simpleRenderSystem.renderGameObjects(frameInfo);

			Renderer.endSwapChainRenderPass(commandBuffer);
			Renderer.endFrame();
		}
	}

	vkDeviceWaitIdle(Device.GetDevice());
}

void CFirstApp::loadGameObjects ()
{
	std::shared_ptr<CModel> Model = CModel::createModelFromFile(Device, "models/quad.obj");
	auto floor = CGameObject::createGameObject();
	floor.model = Model;
	floor.transform.translation = {0.f, .5f, 0.f};
	floor.transform.scale = {3.f, 1.f, 3.f};
	gameObjects.emplace(floor.getId(), std::move(floor));
}


