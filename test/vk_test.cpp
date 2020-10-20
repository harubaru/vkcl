#include <vkcl/vkcl.h>

#include <cstdio>
#include <string>
#include <iostream>
#include <vector>

#include <cstdlib>

int main()
{
	std::vector<vkcl::Device> devices = vkcl::QueryAllDevices();

	std::cout << "Detected Devices:" << std::endl;
	for (size_t i = 0; i < devices.size(); i++) {
		printf("\t%s - GPU ID %d\n", devices[i].getName().c_str(), devices[i].getId());
	}
	std::cout << std::endl;

	for (size_t gpu = 0; gpu < devices.size(); gpu++) {
		{
			std::cout << "Device Memory Test on GPU ID " << gpu << std::endl;

			// test parameters
			const int BUFFER_COUNT = 4;
			const int TEST_SIZE = 0xFFFF;
			const int TEST_RES = 0xDEADBEEF;

			int *testdata = new int[TEST_SIZE];
			
			for (int i = 0; i < TEST_SIZE; i++)
				testdata[i] = TEST_RES;

			vkcl::Buffer *buffers[BUFFER_COUNT];
			for (int i = 0; i < BUFFER_COUNT; i++) {
				buffers[i] = devices[gpu].CreateBuffer(sizeof(int) * TEST_SIZE);
				devices[gpu].UploadData(buffers[i], testdata);
			}

			delete testdata;

			for (int i = 0; i < BUFFER_COUNT; i++) {
				std::cout << "Buffer " << i << " integrity: " << std::flush;
				testdata = (int *)devices[gpu].DownloadData(buffers[i]);

				for (int i = 0; i < TEST_SIZE; i++) {
					if (testdata[i] != (signed)TEST_RES) {
						std::cout << "Failed\n" << std::flush;
						return -1;
					}
				}
				
				std::cout << "Validated" << std::endl;
				devices[gpu].ReleaseData(testdata);
			}

			
			// Clean up after ourselves now
			for (int i = 0; i < BUFFER_COUNT; i++)
				devices[gpu].DeleteBuffer(buffers[i]);

			std::cout << "Success\n";
		}

		{
			// Compute Example

			std::cout << "Compute Shader Test\n";

			const int BUFFER_COUNT = 3;
			const int TEST_SIZE = 0x11;
			
			vkcl::Shader *shader;
			vkcl::Buffer *buffers[BUFFER_COUNT];

			float *testdata = new float[TEST_SIZE];

			for (int i = 0; i < BUFFER_COUNT; i++) {
				buffers[i] = devices[gpu].CreateBuffer(sizeof(int) * TEST_SIZE);
				devices[gpu].UploadData(buffers[i], testdata);
			}

			try {
				shader = devices[gpu].CreateShader("./test/test_mul.spv", BUFFER_COUNT);
				devices[gpu].BindBuffers(shader, buffers);
				devices[gpu].RunShader(shader, TEST_SIZE, 1, 1);
			} catch (vkcl::util::Exception &e) {
				std::cout << e.getMsg() << std::endl;
				return -1;
			}

			for (int i = 1; i < BUFFER_COUNT; i++) {
				std::cout << "Buffer " << i << " contents: ";

				testdata = (float *)devices[gpu].DownloadData(buffers[i]);

				for (int i = 0; i < TEST_SIZE; i++) {
					std::cout << testdata[i] << " ";
				}
				
				std::cout << std::endl;
				devices[gpu].ReleaseData(testdata);
			}

			std::cout << "Success\n";

			for (int i = 0; i < BUFFER_COUNT; i++)
				devices[gpu].DeleteBuffer(buffers[i]);

			devices[gpu].DeleteShader(shader);
		}
	}

	printf("Shutting down..\n");
	for (auto dev : devices) {
		dev.Delete();
	}

	return 0;
}
