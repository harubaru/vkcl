#ifndef CORE_EXCEPTION_H
#define CORE_EXCEPTION_H

#include <string>

namespace vkcl::util {

	class Exception {
	public:
		Exception() {  }
		Exception(std::string msg) : msg(std::move(msg)) {  }

		std::string getMsg() { return msg; }
	private:
		std::string msg;
	};

}

#endif
