#pragma once
#include <cstdint>
#include <fmt/core.h> // for logger



/*	Assertion Macros */
// https://docs.unrealengine.com/en-US/ProgrammingAndScripting/ProgrammingWithCPP/Assertions/index.html
// AssertionMacros.h unreal engine source
/*--------------------------------------------------------------------------------
	Basic options that by default depend on the build configuration and platform

	DS_DO_CHECKS	If true, then dscheck, dsverify, are compiled into the executables
	DS_LOGGING		If false, then no logs or text output will be produced
--------------------------------------------------------------------------------*/

#if DS_BUILD_DEBUG
#elif DS_BUILD_RELEASE
#else
#define DS_BUILD_DEBUG 1
#endif


#if DS_BUILD_DEBUG
#ifndef DS_DO_CHECKS
#define DS_DO_CHECKS	1
#endif

#ifndef DS_LOGGING
#define DS_LOGGING	1
#endif

#elif DS_BUILD_RELEASE
#ifndef DS_DO_CHECKS
#define DS_DO_CHECKS 0
#endif

#ifndef DS_LOGGING
#define DS_LOGGING 1
#endif
#else
#error Exactly one of [DS_BUILD_DEBUG DS_BUILD_RELEASE] should be defined to be 1
#endif

/** ASSERT MACROS
	dscheck: The dscheck family is the closest to the base assert, in that members of this family halt execution
		when the first parameter evaluates to a false value, and WILL NOT evaluate the expresion in release builds by default.
		Use this when you want to evaluate only debug expresions.

	dscheckCode: Executes Code within a do-while loop structure that runs once;
		primarily useful as a way to prepare information that another Check requires

	dsverify: The verify family is the same as the dscheck family. Except that WILL evaluate the expresion in release builds by default.
		Use this when you need to ALWAYS evaluate an expresion.

	dscheckm and dsverifym: do exactly the same as dscheck and dsverify but lets you define a custom message to show to the user.
*/

#if DS_DO_CHECKS
#ifndef dscheck
#define dscheck(expr) DS_CHECK_IMPL(expr, #expr )
#endif
#ifndef dscheckCode
#define dscheckCode(code)	do { code; } while ( false );
#endif
#ifndef dscheckm
#define dscheckm(expr, msg)	DS_CHECK_IMPL(expr, msg)
#endif

#ifndef dsverify
#define dsverify(expr) DS_CHECK_IMPL(expr, #expr )
#endif
#ifndef dsverifym
#define dsverifym(expr, msg) DS_CHECK_IMPL(expr, msg )
#endif

#define DS_CHECK_IMPL(expr, msg) do { if (!(expr)) { DS_FATAL(msg);  } } while (0)
#else

#define dscheck(expr)
#define dscheckCode(...)
#define dscheckm(expr, msg) expr

#define dsverify(expr) expr
#define dsverifym(expr, msg) expr


#endif // DS_DO_CHECKS



/** Destral Logging */
#if DS_LOGGING
#ifndef DS_MESSAGE_LOG
#define DS_MESSAGE_LOG(loglvl_, message_) ds::log::msg(loglvl_, __FILE__ , __LINE__, message_)
#endif
#else
#ifndef DS_MESSAGE_LOG
#define DS_MESSAGE_LOG(loglvl_, message_)
#endif
#endif

#define DS_TRACE(message_) DS_MESSAGE_LOG(ds::log::level::DS_LOG_TRACE, message_)
#define DS_LOG(message_) DS_MESSAGE_LOG(ds::log::level::DS_LOG_INFO, message_)
#define DS_WARNING(message_) DS_MESSAGE_LOG(ds::log::level::DS_LOG_WARNING, message_)
// Logs an error message and aborts the program
#define DS_FATAL(message_) do {ds::log::msg(ds::log::level::DS_LOG_ERROR, __FILE__ , __LINE__, message_); std::abort(); } while(0)

namespace ds::log {

	/** Log level enumeration */
	enum class level { DS_LOG_TRACE = 0, DS_LOG_INFO, DS_LOG_WARNING, DS_LOG_ERROR };

	/**
		Initializes the log system:	If file_id is nullptr it only logs in stdout.
		Else logs in stdout and creates a file like this: (supposing file_name = "AstralPixel")
		AstralPixel-2020.09.13-20.40.50.txt
	*/
	void init(const char* file_name = nullptr);

	/**
		Logs the message to the stdout and file if file is opened.
		Use the macro DS_MESSAGE_LOG to have all those parameters filled for you.
	*/
	void msg(level log_level, const char* file, int line, const std::string_view& msg);

	/**
		Closes the opened log file if necessary and does the cleanup of the log system
	*/
	void shutdown();
}

namespace ds {
	using i8 = int8_t;
	using i16 = int16_t;
	using i32 = int32_t;
	using i64 = int64_t;

	using u8 = uint8_t;
	using u16 = uint16_t;
	using u32 = uint32_t;
	using u64 = uint64_t;
}

/** Error Structure */
namespace ds {
	struct error {
		static inline error failure(const std::string& str) { error e; e.is_error = true; e.details = str; return e; }
		static inline error success() { return {}; }
		bool is_error = false;
		std::string details;
	};
}


/** Hash strings functions and literals */
namespace ds {
	// Fowler–Noll–Vo hash function v. 1a - the good
	constexpr u64 fnv1a_64bit(const char* curr) { 
		auto value = 14695981039346656037ull; while (*curr != 0) { value = (value ^ static_cast<u64>(*(curr++))) * 1099511628211ull; }
		return value;
	}
	constexpr u64 fnv1a_64bit(const std::string& str) {	return fnv1a_64bit(str.c_str());}
	
	inline namespace literals { constexpr u64 operator""_hs(const char* s, std::size_t) { return ds::fnv1a_64bit(s); } }
}




