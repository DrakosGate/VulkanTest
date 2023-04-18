#pragma once

#include "Descriptors.h"
#include "Device.h"
#include "GameObject.h"
#include "Renderer.h"
#include "Window.h"


#include <memory>
#include <vector>


class CFirstApp
{
public:
	static constexpr int WIDTH = 800;
	static constexpr int HEIGHT = 600;

	CFirstApp ();

	~CFirstApp ();

	void run ();

private:
	void loadGameObjects ();

	CWindow Window {WIDTH, HEIGHT, "Vulkan Tutorial"};
	CDevice Device{Window};
	CRenderer Renderer {Window, Device};

	// note: order of declarations matters
	std::unique_ptr<CDescriptorPool> globalPool {};
	CGameObject::Map gameObjects;
};

