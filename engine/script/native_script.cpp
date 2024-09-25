#include "native_script.h"

#include "log.h"

namespace BE_NAMESPACE
{
	void NativeScript::start()
	{
		Log(LogTempCategory, LogSeverity::Log, "Starting Script");
	}
	void NativeScript::update(float tick)
	{
		// sample update
		// std::cout << "delta time is: " << tick << std::endl;
	}
}