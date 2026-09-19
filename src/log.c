#include "log.h"
#include <stdarg.h>
#include <stdio.h>

static int verbosity = 0;

void log_set_verbosity(int level) { verbosity = level; }

int log_get_verbosity() { return verbosity; }

static const char *log_get_level_str(int level) {
	switch (level) {
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

const char *log_get_verbosity_str() { return log_get_level_str(verbosity); }

void vlog(int level, const char *fmt, ...) {
	if (level > verbosity)
		return;

	fprintf(stderr, "%5s: ", log_get_level_str(level));
	va_list ap;
	va_start(ap, fmt);
	vfprintf(stderr, fmt, ap);
	va_end(ap);
	fputc('\n', stderr);
}
