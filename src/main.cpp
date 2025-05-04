#include <vector>

#include "spdlog/sinks/basic_file_sink.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/spdlog.h"

#include "application.hpp"
#include "vec2.hpp"

int main(int argc, char* argv[])
{
	std::vector<spdlog::sink_ptr> sinks;
	sinks.push_back(std::make_shared<spdlog::sinks::stderr_color_sink_mt>());
	sinks[0]->set_pattern("%^[%Y-%m-%d %T.%e] [%L]%$ %v");
	sinks.push_back(std::make_shared<spdlog::sinks::basic_file_sink_st>("ant.log", true));
	sinks[1]->set_pattern("[%Y-%m-%d %T.%e] [%L] %v");
	auto combined_logger = std::make_shared<spdlog::logger>("default_logger", sinks.begin(), sinks.end());
	spdlog::set_default_logger(combined_logger);
	spdlog::set_level(spdlog::level::debug);
	return run_application(glm::ivec2(1000, 1000));
}
