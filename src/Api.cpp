#include "samplog/Api.hpp"
#include "Logger.hpp"
#include "LogManager.hpp"
#include "LogConfig.hpp"
#include "SampConfigReader.hpp"

#include <atomic>
#include <fmt/format.h>


std::atomic<unsigned int> RefCounter{ 0 };

class Api : public samplog::internal::IApi
{
public:
	samplog::ILogger *CreateLogger(const char *module) override
	{
		if (strstr(module, "log-core") != nullptr)
			return nullptr;

		return new Logger(module);
	}
};

extern "C" DLL_PUBLIC samplog::internal::IApi *samplog_GetApi(int version)
{
	if (RefCounter == 0)
	{
		LogConfig::Get()->Initialize();
		LogManager::Get(); // force init
	}

	samplog::internal::IApi *api = nullptr;
	switch (version)
	{
	case 1:
		api = new Api;
		break;
	default:
		LogManager::Get()->LogInternal(samplog::LogLevel::ERROR,
			fmt::format("unknown api version '{:d}'", version));
		return nullptr;
	}

	RefCounter++;
	return api;
}

extern "C" DLL_PUBLIC void samplog_DestroyApi(samplog::internal::IApi *api)
{
	if (api == nullptr)
		return;

	delete api;
	RefCounter--;

	if (RefCounter == 0)
	{
		LogRotationManager::Destroy();
		SampConfigReader::Destroy();
		LogConfig::Destroy();
		LogManager::Destroy();
	}
}
