#include "Buffer.h"

#include <cassert>
#include <cstring>

VkDeviceSize CBuffer::getAlignment (VkDeviceSize instanceSize, VkDeviceSize minOffsetAlignment)
{
	if (minOffsetAlignment > 0)
	{
		return (instanceSize + minOffsetAlignment - 1) & ~(minOffsetAlignment - 1);
	}
	return instanceSize;
}

CBuffer::CBuffer (CDevice &device, VkDeviceSize instanceSize, uint32_t instanceCount, VkBufferUsageFlags usageFlags, VkMemoryPropertyFlags memoryPropertyFlags, VkDeviceSize minOffsetAlignment)
		: Device {device}, instanceSize {instanceSize}, instanceCount {instanceCount}, usageFlags {usageFlags}, memoryPropertyFlags {memoryPropertyFlags}
{
	alignmentSize = getAlignment(instanceSize, minOffsetAlignment);
	bufferSize = alignmentSize * instanceCount;
	device.createBuffer(bufferSize, usageFlags, memoryPropertyFlags, buffer, memory);
}

CBuffer::~CBuffer ()
{
	unmap();
	vkDestroyBuffer(Device.GetDevice(), buffer, nullptr);
	vkFreeMemory(Device.GetDevice(), memory, nullptr);
}

VkResult CBuffer::map (VkDeviceSize size, VkDeviceSize offset)
{
	assert(buffer && memory && "Called map on buffer before create");
	return vkMapMemory(Device.GetDevice(), memory, offset, size, 0, &mapped);
}

void CBuffer::unmap ()
{
	if (mapped)
	{
		vkUnmapMemory(Device.GetDevice(), memory);
		mapped = nullptr;
	}
}

void CBuffer::writeToBuffer (void *data, VkDeviceSize size, VkDeviceSize offset)
{
	assert(mapped && "Cannot copy to unmapped buffer");

	if (size == VK_WHOLE_SIZE)
	{
		memcpy(mapped, data, bufferSize);
	}
	else
	{
		char *memOffset = (char *) mapped;
		memOffset += offset;
		memcpy(memOffset, data, size);
	}
}

VkResult CBuffer::flush (VkDeviceSize size, VkDeviceSize offset)
{
	VkMappedMemoryRange mappedRange = {};
	mappedRange.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
	mappedRange.memory = memory;
	mappedRange.offset = offset;
	mappedRange.size = size;
	return vkFlushMappedMemoryRanges(Device.GetDevice(), 1, &mappedRange);
}

VkResult CBuffer::invalidate (VkDeviceSize size, VkDeviceSize offset)
{
	VkMappedMemoryRange mappedRange = {};
	mappedRange.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
	mappedRange.memory = memory;
	mappedRange.offset = offset;
	mappedRange.size = size;
	return vkInvalidateMappedMemoryRanges(Device.GetDevice(), 1, &mappedRange);
}

VkDescriptorBufferInfo CBuffer::descriptorInfo (VkDeviceSize size, VkDeviceSize offset)
{
	return VkDescriptorBufferInfo {buffer, offset, size,};
}

void CBuffer::writeToIndex (void *data, int index)
{
	writeToBuffer(data, instanceSize, index * alignmentSize);
}

VkResult CBuffer::flushIndex (int index)
{
	return flush(alignmentSize, index * alignmentSize);
}

VkDescriptorBufferInfo CBuffer::descriptorInfoForIndex (int index)
{
	return descriptorInfo(alignmentSize, index * alignmentSize);
}

VkResult CBuffer::invalidateIndex (int index)
{
	return invalidate(alignmentSize, index * alignmentSize);
}


