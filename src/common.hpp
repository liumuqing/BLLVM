#pragma once

#include <cstddef> // std::size_t
#include <cstdio> // std::printf, etc
#include <cstdint>
#include <exception>
#include <string>
#include <plog/Log.h>

typedef uint64_t uaddr_t;

template<typename... Args>
void FATAL(const char * format, Args... args) {
	PLOGF.printf(format, args...);
	throw std::string("FATAL Exception");
}

template<typename... Args>
void INFO(const char * format, Args... args) {
	PLOGI.printf(format, args...);
}

template<typename... Args>
void DEBUG(const char * format, Args... args) {
	PLOGD.printf(format, args...);
}

template<typename... Args>
void WARN(const char * format, Args... args) {
	PLOGW.printf(format, args...);
}

#define FATAL_UNLESS(cond) do {if (!(cond)) {FATAL("[ERROR] %s:%d %s is not satisfied\n", __FILE__, __LINE__, #cond);}} while (0)
