#include "Descriptors.h"

// std
#include <cassert>
#include <stdexcept>



// *************** Descriptor Set Layout CBuilder *********************

CDescriptorSetLayout::CBuilder &CDescriptorSetLayout::CBuilder::addBinding (uint32_t binding, VkDescriptorType descriptorType, VkShaderStageFlags stageFlags, uint32_t count)
{
	assert(bindings.count(binding) == 0 && "Binding already in use");
	VkDescriptorSetLayoutBinding layoutBinding {};
	layoutBinding.binding = binding;
	layoutBinding.descriptorType = descriptorType;
	layoutBinding.descriptorCount = count;
	layoutBinding.stageFlags = stageFlags;
	bindings[binding] = layoutBinding;
	return *this;
}

std::unique_ptr<CDescriptorSetLayout> CDescriptorSetLayout::CBuilder::build () const
{
	return std::make_unique<CDescriptorSetLayout>(Device, bindings);
}

// *************** Descriptor Set Layout *********************

CDescriptorSetLayout::CDescriptorSetLayout (CDevice &device, std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings)
		: Device {device}, bindings {bindings}
{
	std::vector<VkDescriptorSetLayoutBinding> setLayoutBindings {};
	for (auto kv: bindings)
	{
		setLayoutBindings.push_back(kv.second);
	}

	VkDescriptorSetLayoutCreateInfo descriptorSetLayoutInfo {};
	descriptorSetLayoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	descriptorSetLayoutInfo.bindingCount = static_cast<uint32_t>(setLayoutBindings.size());
	descriptorSetLayoutInfo.pBindings = setLayoutBindings.data();

	if (vkCreateDescriptorSetLayout(device.GetDevice(), &descriptorSetLayoutInfo, nullptr, &descriptorSetLayout) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create descriptor set layout!");
	}
}

CDescriptorSetLayout::~CDescriptorSetLayout ()
{
	vkDestroyDescriptorSetLayout(Device.GetDevice(), descriptorSetLayout, nullptr);
}

// *************** Descriptor Pool CBuilder *********************

CDescriptorPool::CBuilder &CDescriptorPool::CBuilder::addPoolSize (VkDescriptorType descriptorType, uint32_t count)
{
	poolSizes.push_back({descriptorType, count});
	return *this;
}

CDescriptorPool::CBuilder &CDescriptorPool::CBuilder::setPoolFlags (VkDescriptorPoolCreateFlags flags)
{
	poolFlags = flags;
	return *this;
}

CDescriptorPool::CBuilder &CDescriptorPool::CBuilder::setMaxSets (uint32_t count)
{
	maxSets = count;
	return *this;
}

std::unique_ptr<CDescriptorPool> CDescriptorPool::CBuilder::build () const
{
	return std::make_unique<CDescriptorPool>(Device, maxSets, poolFlags, poolSizes);
}

// *************** Descriptor Pool *********************

CDescriptorPool::CDescriptorPool (CDevice &device, uint32_t maxSets, VkDescriptorPoolCreateFlags poolFlags, const std::vector<VkDescriptorPoolSize> &poolSizes)
		: Device {device}
{
	VkDescriptorPoolCreateInfo descriptorPoolInfo {};
	descriptorPoolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	descriptorPoolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
	descriptorPoolInfo.pPoolSizes = poolSizes.data();
	descriptorPoolInfo.maxSets = maxSets;
	descriptorPoolInfo.flags = poolFlags;

	if (vkCreateDescriptorPool(device.GetDevice(), &descriptorPoolInfo, nullptr, &descriptorPool) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create descriptor pool!");
	}
}

CDescriptorPool::~CDescriptorPool ()
{
	vkDestroyDescriptorPool(Device.GetDevice(), descriptorPool, nullptr);
}

bool CDescriptorPool::allocateDescriptor (const VkDescriptorSetLayout descriptorSetLayout, VkDescriptorSet &descriptor) const
{
	VkDescriptorSetAllocateInfo allocInfo {};
	allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocInfo.descriptorPool = descriptorPool;
	allocInfo.pSetLayouts = &descriptorSetLayout;
	allocInfo.descriptorSetCount = 1;

	// Might want to create a "DescriptorPoolManager" class that handles this case, and builds
	// a new pool whenever an old pool fills up. But this is beyond our current scope
	if (vkAllocateDescriptorSets(Device.GetDevice(), &allocInfo, &descriptor) != VK_SUCCESS)
	{
		return false;
	}
	return true;
}

void CDescriptorPool::freeDescriptors (std::vector<VkDescriptorSet> &descriptors) const
{
	vkFreeDescriptorSets(Device.GetDevice(), descriptorPool, static_cast<uint32_t>(descriptors.size()), descriptors.data());
}

void CDescriptorPool::resetPool ()
{
	vkResetDescriptorPool(Device.GetDevice(), descriptorPool, 0);
}

// *************** Descriptor Writer *********************

CDescriptorWriter::CDescriptorWriter (CDescriptorSetLayout &setLayout, CDescriptorPool &pool)
		: setLayout {setLayout}, pool {pool}
{
}

CDescriptorWriter &CDescriptorWriter::writeBuffer (uint32_t binding, VkDescriptorBufferInfo *bufferInfo)
{
	assert(setLayout.bindings.count(binding) == 1 && "Layout does not contain specified binding");

	auto &bindingDescription = setLayout.bindings[binding];

	assert(bindingDescription.descriptorCount == 1 && "Binding single descriptor info, but binding expects multiple");

	VkWriteDescriptorSet write {};
	write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	write.descriptorType = bindingDescription.descriptorType;
	write.dstBinding = binding;
	write.pBufferInfo = bufferInfo;
	write.descriptorCount = 1;

	writes.push_back(write);
	return *this;
}

CDescriptorWriter &CDescriptorWriter::writeImage (uint32_t binding, VkDescriptorImageInfo *imageInfo)
{
	assert(setLayout.bindings.count(binding) == 1 && "Layout does not contain specified binding");

	auto &bindingDescription = setLayout.bindings[binding];

	assert(bindingDescription.descriptorCount == 1 && "Binding single descriptor info, but binding expects multiple");

	VkWriteDescriptorSet write {};
	write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	write.descriptorType = bindingDescription.descriptorType;
	write.dstBinding = binding;
	write.pImageInfo = imageInfo;
	write.descriptorCount = 1;

	writes.push_back(write);
	return *this;
}

bool CDescriptorWriter::build (VkDescriptorSet &set)
{
	bool success = pool.allocateDescriptor(setLayout.getDescriptorSetLayout(), set);
	if (!success)
	{
		return false;
	}
	overwrite(set);
	return true;
}

void CDescriptorWriter::overwrite (VkDescriptorSet &set)
{
	for (auto &write: writes)
	{
		write.dstSet = set;
	}
	vkUpdateDescriptorSets(pool.Device.GetDevice(), writes.size(), writes.data(), 0, nullptr);
}


