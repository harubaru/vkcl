#include <vkcl/vk_buffer.h>

namespace vkcl {

	void Buffer::CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usageflags, VkMemoryPropertyFlags memflags, VkBuffer &buffer, VkDeviceMemory &buffermemory)
	{
		uint32_t *families = devcopy.getQueueFamilyIndices();

		VkBufferCreateInfo BufferCreateInfo = {};
		BufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		BufferCreateInfo.size = size;
		BufferCreateInfo.usage = usageflags;
		BufferCreateInfo.sharingMode = VK_SHARING_MODE_CONCURRENT;
		BufferCreateInfo.queueFamilyIndexCount = 2;
		BufferCreateInfo.pQueueFamilyIndices = families;

		if (vkCreateBuffer(devcopy.get(), &BufferCreateInfo, NULL, &buffer) != VK_SUCCESS) {
			throw vkcl::util::Exception(std::string("Failed to create device buffer"));
		}

		VkMemoryRequirements MemoryRequirements;
		vkGetBufferMemoryRequirements(devcopy.get(), buffer, &MemoryRequirements);

		VkMemoryAllocateInfo AllocateInfo = {};
		AllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		AllocateInfo.allocationSize = MemoryRequirements.size;
		AllocateInfo.memoryTypeIndex = devcopy.MemoryType(MemoryRequirements.memoryTypeBits, memflags);

		if (vkAllocateMemory(devcopy.get(), &AllocateInfo, NULL, &buffermemory) != VK_SUCCESS) {
			throw vkcl::util::Exception(std::string("Failed to allocate memory for device buffer"));
		}

		if (vkBindBufferMemory(devcopy.get(), buffer, buffermemory, 0) != VK_SUCCESS) {
			throw vkcl::util::Exception(std::string("Failed to allocate memory for device buffer"));
		}

	}

	void Buffer::CopyBuffer(VkBuffer src, VkBuffer dst)
	{
		VkCommandBufferAllocateInfo cmdbufinfo = {};
		cmdbufinfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		cmdbufinfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		cmdbufinfo.commandPool = devcopy.getShortCommandPool();
		cmdbufinfo.commandBufferCount = 1;

		VkCommandBuffer cmdbuf;
		if (vkAllocateCommandBuffers(devcopy.get(), &cmdbufinfo, &cmdbuf) != VK_SUCCESS) {
			throw vkcl::util::Exception(std::string("Failed to allocate command buffer for transfer operation")); 
		}

		VkCommandBufferBeginInfo begininfo = {};
		begininfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		begininfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
		vkBeginCommandBuffer(cmdbuf, &begininfo);

		// VK COMMANDS START

		VkBufferCopy copyregion = {};
		copyregion.srcOffset = 0;
		copyregion.dstOffset = 0;
		copyregion.size = size;
		vkCmdCopyBuffer(cmdbuf, src, dst, 1, &copyregion);

		// VK COMMANDS END

		vkEndCommandBuffer(cmdbuf);

		VkSubmitInfo submitinfo = {};
		submitinfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitinfo.commandBufferCount = 1;
		submitinfo.pCommandBuffers = &cmdbuf;

		VkFence fence;
		VkFenceCreateInfo fenceinfo = {};
		fenceinfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		fenceinfo.flags = 0;
		vkCreateFence(devcopy.get(), &fenceinfo, nullptr, &fence);

		vkQueueSubmit(devcopy.getTransferQueue(), 1, &submitinfo, fence);
		vkWaitForFences(devcopy.get(), 1, &fence, VK_TRUE, UINT64_MAX);

		vkDestroyFence(devcopy.get(), fence, nullptr);
		vkFreeCommandBuffers(devcopy.get(), devcopy.getShortCommandPool(), 1, &cmdbuf);
	}

	void Buffer::Load(const Device &dev, VkDeviceSize size)
	{
		devcopy.Load(dev);
		this->size = size;

		try {
			CreateBuffer(size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, DevBuffer, DevBufferMemory);
		} catch (vkcl::util::Exception& e) {
			throw e;
		}
	
		freed = false;
		released = true;
	}

	void Buffer::Load(const Buffer &buf)
	{
		devcopy = buf.devcopy;
		HostReadBuffer = buf.HostReadBuffer;
		HostReadBufferMemory = buf.HostReadBufferMemory;
		DevBuffer = buf.DevBuffer;
		DevBufferMemory = buf.DevBufferMemory;
		size = buf.size;
		released = buf.released;
		freed = buf.freed;
	}

	void Buffer::Delete()
	{
		vkFreeMemory(devcopy.get(), DevBufferMemory, nullptr);
		vkDestroyBuffer(devcopy.get(), DevBuffer, nullptr);
	}

	Buffer::~Buffer()
	{
		if (!freed) {
			if (!released) {
				this->ReleaseData();
				released = true;
			}
			this->Delete();
			freed = true;
		}
	}

	void Buffer::UploadData(void *data)
	{
		VkBuffer HostBuffer;
		VkDeviceMemory HostBufferMemory;
		
		try {
			CreateBuffer(size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, HostBuffer, HostBufferMemory);
		} catch (vkcl::util::Exception& e) {
			throw e;
		}

		void *data_loc;
		vkMapMemory(devcopy.get(), HostBufferMemory, 0, size, 0, &data_loc);
		memcpy(data_loc, data, (size_t)size);
		vkUnmapMemory(devcopy.get(), HostBufferMemory);

		try {
			CopyBuffer(HostBuffer, DevBuffer);
		} catch (vkcl::util::Exception& e) {
			throw e;
		}

		vkFreeMemory(devcopy.get(), HostBufferMemory, nullptr);
		vkDestroyBuffer(devcopy.get(), HostBuffer, nullptr);
	}

	void *Buffer::GetData()
	{
		try {
			CreateBuffer(size, VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, HostReadBuffer, HostReadBufferMemory);
		} catch (vkcl::util::Exception& e) {
			throw e;
		}

		try {
			CopyBuffer(DevBuffer, HostReadBuffer);
		} catch (vkcl::util::Exception& e) {
			throw e;
		}

		void *data_loc;
		vkMapMemory(devcopy.get(), HostReadBufferMemory, 0, size, 0, &data_loc);

		released = false;

		return data_loc; // not implemented yet
	}

	void Buffer::ReleaseData()
	{
		if (!released) {
			vkUnmapMemory(devcopy.get(), HostReadBufferMemory);
			vkFreeMemory(devcopy.get(), HostReadBufferMemory, nullptr);
			vkDestroyBuffer(devcopy.get(), HostReadBuffer, nullptr);	
			released = true;	
		}
	}

}
