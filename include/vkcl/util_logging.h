#ifndef UTIL_LOGGING_H
#define UTIL_LOGGING_H

#include <string>
#include <fstream>
#include <iostream>

namespace vkcl::util {

	enum class LogLevel : int {
		Debug = 0,
		Error = 1,
		Info =  2,
		Warn =  3
	};

	class Logger {
	public:
		Logger() {  }
		Logger(std::string filename);
		void Load(std::string filename);

		void Debug(std::string msg) { Print(LogLevel::Debug, msg); }
		void Error(std::string msg) { Print(LogLevel::Error, msg); }
		void  Info(std::string msg) { Print(LogLevel::Info,  msg); }
		void  Warn(std::string msg) { Print(LogLevel::Warn,  msg); }

	private:
		std::ofstream fstream;
		void Print(LogLevel level, std::string msg);
	};

	extern Logger libLogger;

}

#endif
