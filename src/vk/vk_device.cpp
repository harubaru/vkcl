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
		this->vendorname = dev.vendorname;
	}

	void Device::Load(VkInstance instance, VkPhysicalDevice PhysicalDevice)
	{
		(void)instance;
		this->PhysicalDevice = PhysicalDevice;

		vkGetPhysicalDeviceProperties(PhysicalDevice, &PhysicalDeviceProps);
		vendorname = PhysicalDeviceProps.deviceName;

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
	}

	std::vector<Device> QueryAllDevices()
	{
		std::vector<vkcl::Device> devices;

		try {
			if (!vkinstance.getLoaded())
				vkinstance.Load();
			
			std::vector<VkPhysicalDevice> physdevs = QueryPhysicalDevices(vkinstance.get());

			for (uint32_t i = 0; i < physdevs.size(); i++) {
				vkcl::Device device(vkinstance.get(), physdevs[i]);
				device.setId(i);
				devices.push_back(device);
			}

		} catch (vkcl::util::Exception &e) {
			vkcl::util::libLogger.Error(e.getMsg());
		}

		return devices;
	}

}
