#ifndef VK_DEVICE_H
#define VK_DEVICE_H

#include <vkcl/volk.h>

#include "util_exception.h"
#include "util_logging.h"
#include "vk_instance.h"

#include <vector>
#include <string>

namespace vkcl {

	std::vector<VkPhysicalDevice> QueryPhysicalDevices(VkInstance instance);

	class Device {
	public:
		Device() { }
		Device(const Device &dev);
		Device(VkInstance instance, VkPhysicalDevice PhysicalDevice);
		void Delete();

		void Load(const Device &dev);
		void Load(VkInstance instance, VkPhysicalDevice PhysicalDevice);
		uint32_t MemoryType(uint32_t Type, VkMemoryPropertyFlags Props);

		inline void setId(uint32_t id) { this->id = id; }
		inline uint32_t getId() { return id; }
		inline std::string getName() { return vendorname; }

		inline VkDevice get() { return device; }
		inline VkQueue getComputeQueue() { return ComputeQueue; }
		inline VkQueue getTransferQueue() { return TransferQueue; }
		inline VkPhysicalDeviceProperties getProps() { return PhysicalDeviceProps; }
		inline VkPhysicalDevice getPhysicalDev() { return PhysicalDevice; }
		inline VkCommandPool getShortCommandPool() { return Pool_ShortLived; } // Command pool for short lived command buffers
		inline VkCommandPool getCommandPool() { return Pool; }
		inline uint32_t *getQueueFamilyIndices() { return QueueFamilyIndices; } // [0] = Compute, [1] = Transfer

		inline VolkDeviceTable getDeviceTable() { return table; }
	
		void operator=(const Device &devb);
	protected:
		VkPhysicalDevice PhysicalDevice;
		VkPhysicalDeviceProperties PhysicalDeviceProps;
		VkDevice device;
		VkQueue ComputeQueue;
		VkQueue TransferQueue;
		VkCommandPool Pool_ShortLived;
		VkCommandPool Pool;
		uint32_t QueueFamilyIndices[2];
		uint32_t id;
		std::string vendorname;

		VolkDeviceTable table;
	};

	std::vector<vkcl::Device> QueryAllDevices();

}

#endif
