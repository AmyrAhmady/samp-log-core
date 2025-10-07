#pragma once

#include "Api.hpp"

#include <string>


namespace samplog
{
	class PluginLogger
	{
	public:
		explicit PluginLogger(std::string pluginname) :
			_logger(Api::Get()->CreateLogger(pluginname.insert(0, "plugins/").c_str()))
		{ }
		~PluginLogger() = default;
		PluginLogger(PluginLogger const &rhs) = delete;
		PluginLogger& operator=(PluginLogger const &rhs) = delete;
		PluginLogger(PluginLogger &&other) = delete;
		PluginLogger& operator=(PluginLogger &&other) = delete;

	private:
		Logger_t _logger;

	public:
		inline bool IsLogLevel(LogLevel log_level)
		{
			return _logger->IsLogLevel(log_level);
		}

		inline bool Log(LogLevel level, const char *msg)
		{
			return _logger->Log(level, msg);
		}

		inline bool Log(AMX * const amx, const LogLevel level, const char *msg)
		{
			return _logger->Log(level, msg);
		}

		inline bool operator()(LogLevel level, const char *msg)
		{
			return Log(level, msg);
		}

		inline bool operator()(AMX * const amx, const LogLevel level, const char *msg)
		{
			return Log(amx, level, msg);
		}
	};

	typedef PluginLogger PluginLogger_t;

}
