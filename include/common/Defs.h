#ifndef MACROS_H
#define MACROS_H

#include <cstdio>
#include <cstdlib> // for exit()


#define ENABLE_LOG 1
#define OUTPUT_TIME_ENABLE 0 // whether output time when logging


#if ENABLE_LOG

#ifndef __GNUC__
#define __PRETTY_FUNCTION__ __FUNCTION__
#endif
#define PRINT_LOCATE printf(GRAY "In file %s, function %s, line %d " RESET, __FILE__, __PRETTY_FUNCTION__, __LINE__)

#if OUTPUT_TIME_ENABLE
#define TIME_STR GRAY __DATE__ " " __TIME__ RESET " "
#else
#define TIME_STR
#endif

#define PRINT_TIME printf(GRAY __DATE__ " " __TIME__ RESET " ")

#define RESET "\033[0m"
#define RED "\033[31m"	  /* Red */
#define GREEN "\033[32m"  /* Green */
#define YELLOW "\033[33m" /* Yellow */
#define GRAY "\033[30m"	  /* Light gray */
#define DARKRED "\033[1;31m"
#define DARKGREEN "\033[1;32m"
#define DARKYELLOW "\033[1;33m"


#define LOG_ERROR(fmt, ...)                          \
	do {                                             \
		printf(TIME_STR "[" RED "ERROR" RESET "] "); \
		PRINT_LOCATE;                                \
		printf(fmt "\n", ##__VA_ARGS__);             \
		exit(1);                                     \
	} while (0)
#define LOG_WARNING(fmt, ...)                             \
	do {                                                  \
		printf(TIME_STR "[" YELLOW "WARNING" RESET "] "); \
		PRINT_LOCATE;                                     \
		printf(fmt "\n", ##__VA_ARGS__);                  \
	} while (0)
#define LOG_INFO(fmt, ...) printf(TIME_STR "[" GREEN "INFO" RESET "] " fmt "\n", ##__VA_ARGS__)
#define ASSERT_EXIT(expr, message)                    \
	do {                                              \
		if (!(expr)) {                                \
			printf("[" RED "ERROR" RESET "] ");       \
			PRINT_LOCATE;                             \
			printf("Assertion fails! %s\n", message); \
			exit(1);                                  \
		}                                             \
	} while (0)
#define ASSERT(expr, message)                         \
	do {                                              \
		if (!(expr)) {                                \
			printf("[" YELLOW "WARNING" RESET "] ");  \
			PRINT_LOCATE;                             \
			printf("Assertion fails! %s\n", message); \
		}                                             \
	} while (0)
#else
#define PRINT_LOCATE
#define PRINT_TIME
#define LOG_ERROR(message)
#define LOG_WARNING(message)
#define LOG_INFO(message)
#define ASSERT_EXIT(expr, message)
#define ASSERT(expr, message)
#endif // ENABLE_LOG

// Macros to disable copying and moving
#define DISALLOW_COPY(cname)                              \
	cname(const cname&) = delete;			 /* NOLINT */ \
	cname& operator=(const cname&) = delete; /* NOLINT */

#define DISALLOW_MOVE(cname)                         \
	cname(cname&&) = delete;			/* NOLINT */ \
	cname& operator=(cname&&) = delete; /* NOLINT */

#define DISALLOW_COPY_AND_MOVE(cname) \
	DISALLOW_COPY(cname);             \
	DISALLOW_MOVE(cname);

#endif // MACROS_H
