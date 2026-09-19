#include "log.h"
#include <stdarg.h>
#include <stdio.h>

static int verbosity = 0;

void log_set_verbosity(int level) { verbosity = level; }

const int log_get_verbosity() { return verbosity; }

const char *log_get_verbosity_str() {
	switch (verbosity) {
	case LOG_ERROR:
		return "ERROR";
	case LOG_WARN:
		return "WARN";
	case LOG_INFO:
		return "INFO";
	case LOG_DEBUG:
		return "DEBUG";
	default:
		return "TRACE";
	};
};

void vlog(int level, const char *fmt, ...) {
	if (level > verbosity)
		return;

	va_list ap;
	va_start(ap, fmt);
	vfprintf(stderr, fmt, ap);
	va_end(ap);
	fputc('\n', stderr);
}
