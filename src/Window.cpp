#include "Window.h"

// std
#include <stdexcept>


CWindow::CWindow (int w, int h, std::string name)
	: width {w}, height {h}, windowName {name}
{
	initWindow();
}

CWindow::~CWindow ()
{
	glfwDestroyWindow(window);
	glfwTerminate();
}

void CWindow::initWindow ()
{
	glfwInit();
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

	window = glfwCreateWindow(width, height, windowName.c_str(), nullptr, nullptr);
	glfwSetWindowUserPointer(window, this);
	glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);
}

void CWindow::createWindowSurface (VkInstance instance, VkSurfaceKHR *surface)
{
	if (glfwCreateWindowSurface(instance, window, nullptr, surface) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to craete window surface");
	}
}

void CWindow::framebufferResizeCallback (GLFWwindow *window, int width, int height)
{
	auto Window = reinterpret_cast<CWindow *>(glfwGetWindowUserPointer(window));
 	Window->framebufferResized = true;
 	Window->width = width;
 	Window->height = height;
}


