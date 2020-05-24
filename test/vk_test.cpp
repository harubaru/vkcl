#include <vkcl/vkcl.h>

#include <cstdio>
#include <string>
#include <iostream>
#include <vector>

#include <cstdlib>

int main()
{
	std::cout << "Device allocation test - ";
	std::vector<vkcl::Device> devices = vkcl::QueryAllDevices();
	std::cout << "Success\n";

	for (auto dev : devices) {
		std::cout << dev.getName() << " " << dev.getId() << std::endl;
	}

	try {	// buffer r/w test
		std::cout << "Device Memory Test\n";

		const int BUFFER_COUNT = 4;
		const int TEST_SIZE = 0xF;
		const int TEST_RES = 0xDEADBEEF;

		vkcl::Buffer buffers[BUFFER_COUNT];
		for (int i = 0; i < BUFFER_COUNT; i++)
			buffers[i].Load(devices[0], sizeof(int) * TEST_SIZE);

		int *testdata = (int *)malloc(sizeof(int) * TEST_SIZE);

		for (int i = 0; i < TEST_SIZE; i++)
			testdata[i] = TEST_RES;

		for (int i = 0; i < BUFFER_COUNT; i++)
			buffers[i].UploadData((void *)testdata);
		
		free(testdata);

		for (int i = 0; i < BUFFER_COUNT; i++) {
			std::cout << "Buffer " << i << " integrity: ";
			testdata = (int *)buffers[i].GetData();
			for (int i = 0; i < TEST_SIZE; i++) {
				if (testdata[i] != (signed)TEST_RES) {
					std::cout << "Failed\n";
					return -1;
				}
			}
			std::cout << "Validated" << std::endl;
		}

		std::cout << "Success\n";
	} catch (vkcl::util::Exception& e) {
		std::cout << e.getMsg() << std::endl;
		return -1;
	}

	try {
		std::cout << "Compute Shader Test\n";

		const int BUFFER_COUNT = 3;
		const int TEST_SIZE = 0xF;

		vkcl::Buffer buffers[BUFFER_COUNT];
		for (auto i = 0; i < BUFFER_COUNT; i++) {
			buffers[i].Load(devices[0], sizeof(float) * TEST_SIZE);
		}

		float *testdata = (float *)malloc(sizeof(float) * TEST_SIZE);
		for (int i = 0; i < TEST_SIZE; i++) {
			testdata[i] = 2.0f;
		}

		buffers[0].UploadData((void *)testdata);	
		free(testdata);
		
		vkcl::Shader shader;

		try {
			shader.Load(devices[0], "./test/test_mul.spv", BUFFER_COUNT);
			shader.BindBuffers(buffers);
			shader.Run(TEST_SIZE, 1, 1);
		} catch (vkcl::util::Exception &e) {
			std::cout << e.getMsg() << std::endl;
			return -1;
		}

		for (int i = 1; i < BUFFER_COUNT; i++) {
			std::cout << "Buffer " << i << " contents: ";
			testdata = (float *)buffers[i].GetData();
			for (int i = 0; i < TEST_SIZE; i++) {
				std::cout << testdata[i] << " ";
			}
			std::cout << std::endl;
		}

		std::cout << "Success\n";

		shader.Delete();
	} catch (vkcl::util::Exception &e) {
	//	std::cout << e.getMsg() << std::endl;
		return -1;
	}

	for (auto dev : devices) {
		dev.Delete();
	}

	return 0;
}
