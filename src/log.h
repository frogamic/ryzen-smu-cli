#ifndef LOG_H
#define LOG_H

enum { LOG_ERROR = 0, LOG_WARN, LOG_INFO, LOG_DEBUG, LOG_TRACE };

void log_set_verbosity(int level);
int log_get_verbosity(void);
const char *log_get_verbosity_str(void);
void vlog(int level, const char *fmt, ...) __attribute__((format(printf, 2, 3)));

#define VLOG(level, ...)                                                                                               \
	do {                                                                                                                 \
		if ((level) <= log_get_verbosity())                                                                                \
			vlog((level), __VA_ARGS__);                                                                                      \
	} while (0)

#endif // LOG_H
