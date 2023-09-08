#include "Log.h"
#include "spdlog/sinks/stdout_color_sinks.h"

namespace XLEngine 
{
	std::shared_ptr<spdlog::logger> Log::s_CoreLogger;
	std::shared_ptr<spdlog::logger> Log::s_ClientLogger;

	void Log::Init()
	{
		spdlog::set_pattern("%^[%T] %n: %v%$");//������ʽ�����ʵ�����ɫ��ʱ���롢logger���Լ�ʵ�ʵ���Ϣ
		s_CoreLogger = spdlog::stdout_color_mt("XLEngine");
		s_CoreLogger->set_level(spdlog::level::trace);

		s_ClientLogger = spdlog::stdout_color_mt("APP");
		s_ClientLogger->set_level(spdlog::level::trace);
	}
}