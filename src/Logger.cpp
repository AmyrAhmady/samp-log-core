#include "Logger.hpp"
#include "LogManager.hpp"
#include "LogConfig.hpp"
#include "utils.hpp"

#include <fmt/format.h>
#include <fmt/time.h>
#include <ctime>


Logger::Logger(std::string module_name) :
	_moduleName(std::move(module_name)),
	_logFilePath(LogConfig::Get()->GetGlobalConfig().LogsRootFolder + _moduleName + ".log"),
	_logCounter(0)
{
	//create possibly non-existing folders before opening log file
	utils::EnsureFolders(_logFilePath);

	LogConfig::Get()->SubscribeLogger(this,
		std::bind(&Logger::OnConfigUpdate, this, std::placeholders::_1));
	if (_config.Append == false)
	{
		// create file if it doesn't exist, and truncate whole content
		std::ofstream logfile(_logFilePath, std::ofstream::trunc);
	}
}

Logger::~Logger()
{
	LogConfig::Get()->UnsubscribeLogger(this);
	LogRotationManager::Get()->UnregisterLogFile(_logFilePath);

	// wait until all log messages are processed, as we have this logger
	// referenced in the action lambda and deleting it would be bad
	while (_logCounter != 0)
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
}

bool Logger::Log(LogLevel level, std::string msg)
{
	if (!IsLogLevel(level))
		return false;

	auto current_time = Clock::now();
	LogManager::Get()->Queue([this, level, current_time, msg]()
	{
		std::string const
			time_str = FormatTimestamp(current_time),
			log_msg = FormatLogMessage(msg);

		WriteLogString(time_str, level, log_msg);
		LogManager::Get()->WriteLevelLogString(time_str, level, GetModuleName(), msg);

		auto const &level_config = LogConfig::Get()->GetLogLevelConfig(level);
		if (_config.PrintToConsole || level_config.PrintToConsole)
			PrintLogString(time_str, level, log_msg);

		--_logCounter;
	});

	++_logCounter;
	return true;
}

void Logger::OnConfigUpdate(Logger::Config const &config)
{
	_config = config;
	LogRotationManager::Get()->RegisterLogFile(_logFilePath, _config.Rotation);
}

std::string Logger::FormatTimestamp(Clock::time_point time)
{
	std::time_t now_c = std::chrono::system_clock::to_time_t(time);
	auto const &time_format = LogConfig::Get()->GetGlobalConfig().LogTimeFormat;
	return fmt::format("{:" + time_format + "}", fmt::localtime(now_c));
}

std::string Logger::FormatLogMessage(std::string message)
{
	fmt::memory_buffer log_string_buf;

	fmt::format_to(log_string_buf, "{:s}", message);

	return fmt::to_string(log_string_buf);
}

void Logger::WriteLogString(std::string const &time, LogLevel level, std::string const &message)
{
	utils::EnsureFolders(_logFilePath);
	std::ofstream logfile(_logFilePath,
		std::ofstream::out | std::ofstream::app);
	logfile <<
		"[" << time << "] " <<
		"[" << utils::GetLogLevelAsString(level) << "] " <<
		message << '\n' << std::flush;
}

void Logger::PrintLogString(std::string const &time, LogLevel level, std::string const &message)
{
	auto *loglevel_str = utils::GetLogLevelAsString(level);
	if (LogConfig::Get()->GetGlobalConfig().EnableColors)
	{
		utils::EnsureTerminalColorSupport();

		fmt::print("[");
		fmt::print(fmt::fg(fmt::rgb(255, 255, 150)), time);
		fmt::print("] [");
		fmt::print(fmt::fg(fmt::color::sandy_brown), GetModuleName());
		fmt::print("] [");
		auto loglevel_color = utils::GetLogLevelColor(level);
		if (level == LogLevel::FATAL)
			fmt::print(fmt::fg(fmt::color::white) | fmt::bg(loglevel_color), loglevel_str);
		else
			fmt::print(fmt::fg(loglevel_color), loglevel_str);
		fmt::print("] {:s}\n", message);
	}
	else
	{
		fmt::print("[{:s}] [{:s}] [{:s}] {:s}\n",
			time, GetModuleName(), loglevel_str, message);
	}
}
