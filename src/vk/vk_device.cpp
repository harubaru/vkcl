#include <stdio.h>
#include <vkcl/vk_device.h>

namespace vkcl {

	static vkcl::Instance vkinstance;

	static uint32_t GetQueueFamily(uint32_t starting_point, VkPhysicalDevice PhysicalDevice, VkQueueFlagBits flags)
	{
		// Get Queue Family Indices
		uint32_t QueueFamilyIndex = 0;
		uint32_t QueueFamilyCount;
		vkGetPhysicalDeviceQueueFamilyProperties(PhysicalDevice, &QueueFamilyCount, nullptr);

		std::vector<VkQueueFamilyProperties> QueueFamilyProps(QueueFamilyCount);
		vkGetPhysicalDeviceQueueFamilyProperties(PhysicalDevice, &QueueFamilyCount, QueueFamilyProps.data());

		uint32_t i = starting_point;
		for (auto prop : QueueFamilyProps) {
			if (prop.queueCount > 0 && (prop.queueFlags & flags)) {
				QueueFamilyIndex = i;
				break;
			}

			i++;
		}

		if (QueueFamilyIndex == QueueFamilyCount) {
			throw vkcl::util::Exception("Failed to get Queue Family Index");
		}

		return QueueFamilyIndex;
	}

	std::vector<VkPhysicalDevice> QueryPhysicalDevices(VkInstance instance)
	{
		std::vector<VkPhysicalDevice> PhysicalDevices;
	
		uint32_t count;
		if (vkEnumeratePhysicalDevices(instance, &count, nullptr) != VK_SUCCESS) {
			throw vkcl::util::Exception("Failed to query amount of physical devices");
			return PhysicalDevices;
		}

		PhysicalDevices.resize(count);
		if (vkEnumeratePhysicalDevices(instance, &count, PhysicalDevices.data()) != VK_SUCCESS) {
			throw vkcl::util::Exception("Failed to query physical devices");
			return PhysicalDevices;
		}
		
		return PhysicalDevices;
	}

	std::vector<std::string> QueryPhysicalDeviceNames(VkInstance instance)
	{
		std::vector<VkPhysicalDevice> PhysicalDevices = QueryPhysicalDevices(instance);
		std::vector<std::string> PhysicalDeviceNames;

		for (VkPhysicalDevice device : PhysicalDevices) {
			VkPhysicalDeviceProperties props;
			vkGetPhysicalDeviceProperties(device, &props);
			PhysicalDeviceNames.emplace_back(std::string(props.deviceName));
		}

		return PhysicalDeviceNames;
	}

	Device::Device(VkInstance instance, VkPhysicalDevice PhysicalDevice)
	{
		Load(instance, PhysicalDevice);
	}

	Device::Device(const Device &dev)
	{
		Load(dev);
	}

	void Device::Load(const Device &dev)
	{
		this->PhysicalDevice = dev.PhysicalDevice;
		this->PhysicalDeviceProps = dev.PhysicalDeviceProps;
		this->device = dev.device;
		this->ComputeQueue = dev.ComputeQueue;
		this->TransferQueue = dev.TransferQueue;
		this->Pool_ShortLived = dev.Pool_ShortLived;
		this->Pool = dev.Pool;
		this->QueueFamilyIndices[0] = dev.QueueFamilyIndices[0];
		this->QueueFamilyIndices[1] = dev.QueueFamilyIndices[1];
		this->id = dev.id;
	}

	void Device::Load(VkInstance instance, VkPhysicalDevice PhysicalDevice)
	{
		(void)instance;
		this->PhysicalDevice = PhysicalDevice;

		vkGetPhysicalDeviceProperties(PhysicalDevice, &PhysicalDeviceProps);

		this->QueueFamilyIndices[0] = GetQueueFamily(0, PhysicalDevice, VK_QUEUE_COMPUTE_BIT);
		this->QueueFamilyIndices[1] = GetQueueFamily(QueueFamilyIndices[0] + 1, PhysicalDevice, VK_QUEUE_TRANSFER_BIT);

		// Create Logical Device
		float CompQueuePriorities = 1.0f;
		VkDeviceQueueCreateInfo CompQueueCreateInfo = {};
		CompQueueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		CompQueueCreateInfo.pNext = nullptr;
		CompQueueCreateInfo.flags = 0;
		CompQueueCreateInfo.queueFamilyIndex = QueueFamilyIndices[0];
		CompQueueCreateInfo.queueCount = 1;
		CompQueueCreateInfo.pQueuePriorities = &CompQueuePriorities;

		float TransQueuePriorities = 1.0f;
		VkDeviceQueueCreateInfo TransQueueCreateInfo = {};
		TransQueueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		TransQueueCreateInfo.pNext = nullptr;
		TransQueueCreateInfo.flags = 0;
		TransQueueCreateInfo.queueFamilyIndex = QueueFamilyIndices[1];
		TransQueueCreateInfo.queueCount = 1;
		TransQueueCreateInfo.pQueuePriorities = &TransQueuePriorities;

		VkDeviceQueueCreateInfo QueueCreateInfos[2];
		QueueCreateInfos[0] = CompQueueCreateInfo;
		QueueCreateInfos[1] = TransQueueCreateInfo;

		VkPhysicalDeviceFeatures PhysDevFeatures = {}; // We only need compute, and we still need this struct.
		VkDeviceCreateInfo DevCreateInfo = {};
		DevCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		DevCreateInfo.pNext = nullptr;
		DevCreateInfo.flags = 0;
		DevCreateInfo.queueCreateInfoCount = 2;
		DevCreateInfo.pQueueCreateInfos = QueueCreateInfos;
		DevCreateInfo.enabledLayerCount = 0;
		DevCreateInfo.ppEnabledLayerNames = nullptr;
		DevCreateInfo.enabledExtensionCount = 0;
		DevCreateInfo.ppEnabledExtensionNames = nullptr;
		DevCreateInfo.pEnabledFeatures = &PhysDevFeatures;
	
		if (vkCreateDevice(PhysicalDevice, &DevCreateInfo, nullptr, &device) != VK_SUCCESS) {
			throw vkcl::util::Exception("Failed to create device");
		}

		vkGetDeviceQueue(device, QueueFamilyIndices[0], 0, &ComputeQueue);
		vkGetDeviceQueue(device, QueueFamilyIndices[1], 0, &TransferQueue);

		VkCommandPoolCreateInfo Pool_ShortLivedInfo = {};
		Pool_ShortLivedInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		Pool_ShortLivedInfo.queueFamilyIndex = QueueFamilyIndices[1];
		Pool_ShortLivedInfo.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;

		VkCommandPoolCreateInfo PoolInfo = {};
		PoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		PoolInfo.queueFamilyIndex = QueueFamilyIndices[0];
		PoolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

		if (vkCreateCommandPool(device, &Pool_ShortLivedInfo, nullptr, &Pool_ShortLived) != VK_SUCCESS) {
			Pool_ShortLived = VK_NULL_HANDLE;
			throw vkcl::util::Exception("Failed to create Staging Command Pool");
		}

		if (vkCreateCommandPool(device, &PoolInfo, nullptr, &Pool) != VK_SUCCESS) {
			Pool = VK_NULL_HANDLE;
			throw vkcl::util::Exception("Failed to create Command Pool");
		}

		// Create the buffer allocator
		VmaAllocatorCreateInfo allocatorInfo = {};
		allocatorInfo.physicalDevice = PhysicalDevice;
		allocatorInfo.device = device;
		allocatorInfo.instance = instance;
		allocatorInfo.vulkanApiVersion = VK_API_VERSION_1_2;

		if (vmaCreateAllocator(&allocatorInfo, &allocator) != VK_SUCCESS) {
			throw vkcl::util::Exception("Failed to create allocator!");
		}
	}

	uint32_t Device::MemoryType(uint32_t Type, VkMemoryPropertyFlags Props)
	{
		VkPhysicalDeviceMemoryProperties MemProps;
		
		vkGetPhysicalDeviceMemoryProperties(this->PhysicalDevice, &MemProps);

		for (uint32_t i = 0; i < MemProps.memoryTypeCount; ++i) {
			if ((Type & (1 << i)) && ((MemProps.memoryTypes[i].propertyFlags & Props) == Props))
				return i;
		}

		return -1;
	}

	void Device::Delete()
	{
		vmaDestroyAllocator(allocator);

		if (Pool_ShortLived != VK_NULL_HANDLE)
			vkDestroyCommandPool(device, Pool_ShortLived, nullptr);
		if (Pool != VK_NULL_HANDLE)
			vkDestroyCommandPool(device, Pool, nullptr);
		if (device != VK_NULL_HANDLE)
			vkDestroyDevice(device, nullptr);
	}

	void Device::operator=(const Device &devb)
	{
		this->PhysicalDevice = devb.PhysicalDevice;
		this->PhysicalDeviceProps = devb.PhysicalDeviceProps;
		this->device = devb.device;
		this->ComputeQueue = devb.ComputeQueue;
		this->TransferQueue = devb.TransferQueue;
		this->Pool_ShortLived = devb.Pool_ShortLived;
		this->Pool = devb.Pool;
		this->QueueFamilyIndices[0] = devb.QueueFamilyIndices[0];
		this->QueueFamilyIndices[1] = devb.QueueFamilyIndices[1];
		this->allocator = devb.allocator;
	}

	std::vector<Device> QueryAllDevices()
	{
		std::vector<vkcl::Device> devices;

		try {
			if (!vkinstance.getLoaded())
				vkinstance.Load();
			
			std::vector<VkPhysicalDevice> physdevs = QueryPhysicalDevices(vkinstance.get());
			devices.resize(physdevs.size());

			for (uint32_t i = 0; i < physdevs.size(); i++) {
				vkcl::Device device(vkinstance.get(), physdevs[i]);
				device.setId(i);
				devices[i] = device;
			}

		} catch (vkcl::util::Exception &e) {
			vkcl::util::libLogger.Error(e.getMsg());
		}

		return devices;
	}
	

	// Buffer Operations

	void Device::CreateVKBuffer(VkDeviceSize size, VkBufferUsageFlags usageflags, VkMemoryPropertyFlags memflags, VkBuffer &buffer, VmaAllocation &allocation, VmaAllocationInfo *allocinfo)
	{
		uint32_t *families = getQueueFamilyIndices();

		VkBufferCreateInfo BufferCreateInfo = {};
		BufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		BufferCreateInfo.size = size;
		BufferCreateInfo.usage = usageflags;
		BufferCreateInfo.sharingMode = VK_SHARING_MODE_CONCURRENT;
		BufferCreateInfo.queueFamilyIndexCount = 2;
		BufferCreateInfo.pQueueFamilyIndices = families;

		VmaAllocationCreateInfo VbAllocInfo = {};

		if (memflags == (VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT)) {
			VbAllocInfo.usage = VMA_MEMORY_USAGE_CPU_ONLY;
			VbAllocInfo.flags = VMA_ALLOCATION_CREATE_MAPPED_BIT;
		} else if (memflags && VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT) {
			VbAllocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
		}

		vmaCreateBuffer(this->allocator, &BufferCreateInfo, &VbAllocInfo, &buffer, &allocation, allocinfo);
	}

	void Device::CopyVKBuffer(VkBuffer src, VkBuffer dst, VkDeviceSize size)
	{
		VkCommandBufferAllocateInfo cmdbufinfo = {};
		cmdbufinfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		cmdbufinfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		cmdbufinfo.commandPool = getShortCommandPool();
		cmdbufinfo.commandBufferCount = 1;

		VkCommandBuffer cmdbuf;
		if (vkAllocateCommandBuffers(device, &cmdbufinfo, &cmdbuf) != VK_SUCCESS) {
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
		vkCreateFence(device, &fenceinfo, nullptr, &fence);

		vkQueueSubmit(getTransferQueue(), 1, &submitinfo, fence);
		vkWaitForFences(device, 1, &fence, VK_TRUE, UINT64_MAX);

		vkDestroyFence(device, fence, nullptr);
		vkFreeCommandBuffers(device, getShortCommandPool(), 1, &cmdbuf);
	}


	Buffer *Device::CreateBuffer(VkDeviceSize size)
	{
		Buffer *buf = new Buffer;
		if (!buf)
			return nullptr;

		CreateVKBuffer(size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, buf->devbuffer, buf->devalloc, &buf->devinfo);

		return buf;
	}

	void Device::DeleteBuffer(Buffer *buffer)
	{
		vmaDestroyBuffer(allocator, buffer->devbuffer, buffer->devalloc);
		delete buffer;
	}

	void Device::UploadData(Buffer *buffer, void *data)
	{
		VkBuffer hostbuf;
		VmaAllocation hostalloc;
		VmaAllocationInfo allocinfo;

		CreateVKBuffer(buffer->devinfo.size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, hostbuf, hostalloc, &allocinfo);
		memcpy(allocinfo.pMappedData, data, buffer->devinfo.size);
		CopyVKBuffer(hostbuf, buffer->devbuffer, buffer->devinfo.size);

		vmaDestroyBuffer(allocator, hostbuf, hostalloc);
	}

	void *Device::DownloadData(Buffer *buffer)
	{
		void *data = malloc(buffer->devinfo.size);

		VkBuffer hostbuf;
		VmaAllocation hostalloc;
		VmaAllocationInfo allocinfo;

		CreateVKBuffer(buffer->devinfo.size, VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, hostbuf, hostalloc, &allocinfo);
		CopyVKBuffer(buffer->devbuffer, hostbuf, buffer->devinfo.size);
		memcpy(data, allocinfo.pMappedData, allocinfo.size);
		vmaDestroyBuffer(allocator, hostbuf, hostalloc);

		return data;
	}

	void Device::ReleaseData(void *data)
	{
		free(data);
	}


}
