#pragma once

#include "Device.h"


#include <memory>
#include <unordered_map>
#include <vector>


class CDescriptorSetLayout
{
public:
	class CBuilder
	{
	public:
		CBuilder (CDevice& Device)
				: Device{Device}
		{
		}

		CBuilder &addBinding (uint32_t binding, VkDescriptorType descriptorType, VkShaderStageFlags stageFlags, uint32_t count = 1);

		std::unique_ptr<CDescriptorSetLayout> build () const;

	private:
		CDevice& Device;
		std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings {};
	};

	CDescriptorSetLayout (CDevice& Device, std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings);
	~CDescriptorSetLayout ();

	VkDescriptorSetLayout getDescriptorSetLayout () const
	{
		return descriptorSetLayout;
	}

private:
	CDevice& Device;
	VkDescriptorSetLayout descriptorSetLayout;
	std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings;

	friend class CDescriptorWriter;
};

class CDescriptorPool
{
public:
	class CBuilder
	{
	public:
		CBuilder (CDevice& Device)
				: Device{Device}
		{
		}

		CBuilder &addPoolSize (VkDescriptorType descriptorType, uint32_t count);

		CBuilder &setPoolFlags (VkDescriptorPoolCreateFlags flags);

		CBuilder &setMaxSets (uint32_t count);

		std::unique_ptr<CDescriptorPool> build () const;

	private:
		CDevice& Device;
		std::vector<VkDescriptorPoolSize> poolSizes {};
		uint32_t maxSets = 1000;
		VkDescriptorPoolCreateFlags poolFlags = 0;
	};

	CDescriptorPool (CDevice& Device, uint32_t maxSets, VkDescriptorPoolCreateFlags poolFlags, const std::vector<VkDescriptorPoolSize> &poolSizes);
	~CDescriptorPool ();

	bool allocateDescriptor (const VkDescriptorSetLayout descriptorSetLayout, VkDescriptorSet &descriptor) const;

	void freeDescriptors (std::vector<VkDescriptorSet> &descriptors) const;

	void resetPool ();

private:
	CDevice& Device;
	VkDescriptorPool descriptorPool;

	friend class CDescriptorWriter;
};

class CDescriptorWriter
{
public:
	CDescriptorWriter (CDescriptorSetLayout &setLayout, CDescriptorPool &pool);

	CDescriptorWriter &writeBuffer (uint32_t binding, VkDescriptorBufferInfo *bufferInfo);

	CDescriptorWriter &writeImage (uint32_t binding, VkDescriptorImageInfo *imageInfo);

	bool build (VkDescriptorSet &set);

	void overwrite (VkDescriptorSet &set);

private:
	CDescriptorSetLayout &setLayout;
	CDescriptorPool &pool;
	std::vector<VkWriteDescriptorSet> writes;
};


