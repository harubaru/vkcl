#include <vkcl/util_logging.h>

namespace vkcl::util {

	static const char *Prefixes[] = {
		"debug: ",
		"error: ",
		"info : ",
		"warn : "
	};

	Logger::Logger(std::string filename)
	{
		Load(filename);
	}

	void Logger::Load(std::string filename)
	{
		fstream = std::ofstream(filename);
	}

	void Logger::Print(LogLevel level, std::string msg)
	{
		if (fstream.is_open()) {
			fstream   << Prefixes[static_cast<int>(level)] << msg << std::endl;
		} else {
			std::cout << Prefixes[static_cast<int>(level)] << msg << std::endl;
		}
	}

	Logger libLogger;

}
