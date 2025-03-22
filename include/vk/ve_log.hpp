#pragma once
#include "spdlog/spdlog.h"

#define VE_CHECKING

#define VE_THROW(...)                 \
{                                     \
	spdlog::error(__VA_ARGS__);         \
	std::string s(__FILE__);            \
	s.append(": ");                     \
	s.append(std::to_string(__LINE__)); \
	spdlog::throw_spdlog_ex(s);         \
}

#if defined(VE_CHECKING)
#define VE_ASSERT(X, ...) if (!(X)) VE_THROW(__VA_ARGS__);
#define VE_CHECK(X, M) vk::detail::resultCheck(X, M)
#else
#define VE_ASSERT(X, ...) X
#define VE_CHECK(X, M) void(X)
#endif

namespace antlog = spdlog;
