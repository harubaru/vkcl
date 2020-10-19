#include <vkcl/vk_instance.h>

#include <iostream>
#include <string>
#include <vector>

extern VkResult volkInitialize(void);

namespace vkcl {

	VkResult __CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT *pCreateInfo, const VkAllocationCallbacks *pAllocator, VkDebugUtilsMessengerEXT *pDebugMessenger)
	{
		auto func = (PFN_vkCreateDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");

		if (!func)
			return VK_ERROR_EXTENSION_NOT_PRESENT;
		
		return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
	}

	void __DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks *pAllocator)
	{
		auto func = (PFN_vkDestroyDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");

		if (func)
			func(instance, debugMessenger, pAllocator);
	}


	VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT severity, VkDebugUtilsMessageTypeFlagsEXT type, const VkDebugUtilsMessengerCallbackDataEXT *data, void *userdata)
	{
		std::cerr << data->pMessage << std::endl;

		return VK_FALSE;
	}

	Instance::~Instance()
	{
#ifdef _DEBUG
		__DestroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);
#endif
		vkDestroyInstance(instance, nullptr);
	}

	void Instance::Load()
	{
		if (Loaded == true)
			return;
		else
			Loaded = true;
		
		if (volkInitialize() != VK_SUCCESS)
			throw vkcl::util::Exception("Failed to create instance");
		
		VkApplicationInfo appInfo = {};
		appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		appInfo.pNext = nullptr;
		appInfo.pApplicationName = "vkcl";
		appInfo.applicationVersion = 0;
		appInfo.pEngineName = "vkcl";
		appInfo.engineVersion = VK_MAKE_VERSION(0, 2, 0);
		appInfo.apiVersion = VK_MAKE_VERSION(1, 1, 0);

		VkInstanceCreateInfo instInfo = {};
		instInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		instInfo.pNext = nullptr;
		instInfo.flags = 0;
		instInfo.pApplicationInfo = &appInfo;
#ifndef _DEBUG
		instInfo.enabledLayerCount = 0;
		instInfo.ppEnabledLayerNames = nullptr;
#else
		uint32_t layercount = 0;
		vkEnumerateInstanceLayerProperties(&layercount, nullptr);

		std::vector<VkLayerProperties> availableLayerProperties(layercount);
		vkEnumerateInstanceLayerProperties(&layercount, availableLayerProperties.data());

		std::cout << "Validation Layers Available:\n";
		for (auto prop : availableLayerProperties) {
			std::cout << prop.layerName << std::endl;
		}

		std::vector<const char *>layernames = {
			"VK_LAYER_KHRONOS_validation" // depending on how your system's vulkan sdk is setup, you might wanna change this.
		};

		std::vector<const char *>extensions = {
			VK_EXT_DEBUG_UTILS_EXTENSION_NAME	
		};

		VkDebugUtilsMessengerCreateInfoEXT createinfo = {};
		createinfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
		createinfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
		createinfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
		createinfo.pfnUserCallback = debugCallback;

		instInfo.enabledExtensionCount = 1;
		instInfo.ppEnabledExtensionNames = extensions.data();
		instInfo.enabledLayerCount = 1;
		instInfo.ppEnabledLayerNames = layernames.data();
		instInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT *) &createinfo;
#endif
		if (vkCreateInstance(&instInfo, nullptr, &instance) != VK_SUCCESS) {
			throw vkcl::util::Exception("Failed to create instance");
		}

//		volkLoadInstanceOnly(instance);
		volkLoadInstance(instance);

#ifdef _DEBUG
		// setup debug messenger

		VkDebugUtilsMessengerCreateInfoEXT createinfo2 = {};
		createinfo2.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
		createinfo2.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
		createinfo2.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
		createinfo2.pfnUserCallback = debugCallback;
		if (__CreateDebugUtilsMessengerEXT(instance, &createinfo2, nullptr, &debugMessenger) != VK_SUCCESS) {
			throw vkcl::util::Exception("Failed to create debugger messenger callback");
		}
#endif
	}

}
