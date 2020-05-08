#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <time.h>

#include "log.h"

static void	(*cb_log)(const char *) = NULL;

static uint8_t	log_lvl_debug;
static uint8_t	log_lvl_info;
static uint8_t	log_lvl_warn;

void
log_setcb(void (*cb)(const char *))
{
	cb_log = cb;
}

void
log_enable(uint8_t log_lvl, uint8_t onoff)
{
	switch (log_lvl) {
	case LOG_LVL_DEBUG:
		log_lvl_debug = onoff;
		break;
	case LOG_LVL_INFO:
		log_lvl_info = onoff;
		break;
	case LOG_LVL_WARN:
		log_lvl_warn = onoff;
		break;
	}

	return;
}

void
log_debug(const char *format, ...)
{
	struct tm	*tm_info;
	time_t		 timer;
	va_list		 list;
	int		 len;
	static char	 buff[512];
	char		 cur_time[20];

	if (log_lvl_debug != 0)
		return;

	time(&timer);
	tm_info = localtime(&timer);
	strftime(cur_time, 20, "%Y-%m-%d %H:%M:%S", tm_info);

	len = snprintf(buff, sizeof(buff), "[%s] debug> ", cur_time);

	va_start(list, format);
	len += vsnprintf(buff + len, sizeof(buff) - len, format, list);
	snprintf(buff + len, sizeof(buff) - len, "\n");
	va_end(list);

	if (cb_log)
		cb_log(buff);
	else {
		fprintf(stdout, "%s", buff);
		fflush(stdout);
	}
}

void
log_info(const char *format, ...)
{
	struct tm	*tm_info;
	time_t		 timer;
	va_list		 list;
	int		 len;
	static char	 buff[512];
	char		 cur_time[20];

	if (log_lvl_info == 0)
		return;

	time(&timer);
	tm_info = localtime(&timer);
	strftime(cur_time, 20, "%Y-%m-%d %H:%M:%S", tm_info);

	len = snprintf(buff, sizeof(buff), "[%s] info> ", cur_time);

	va_start(list, format);
	len += vsnprintf(buff + len, sizeof(buff) - len, format, list);
	snprintf(buff + len, sizeof(buff) - len, "\n");
	va_end(list);

	if (cb_log)
		cb_log(buff);
	else {
		fprintf(stdout, "%s", buff);
		fflush(stdout);
	}
}

void
log_warnx(const char *format, ...)
{
	struct tm	*tm_info;
	time_t		 timer;
	va_list		 list;
	int		 len;
	static char	 buff[512];
	char		 cur_time[20];

	if (log_lvl_warn == 0)
		return;

	time(&timer);
	tm_info = localtime(&timer);
	strftime(cur_time, 20, "%Y-%m-%d %H:%M:%S", tm_info);

	len = snprintf(buff, sizeof(buff), "[%s] warn> ", cur_time);

	va_start(list, format);
	len += vsnprintf(buff + len, sizeof(buff) - len, format, list);
	snprintf(buff + len, sizeof(buff) - len, "\n");
	va_end(list);

	if (cb_log)
		cb_log(buff);
	else {
		fprintf(stderr, "%s", buff);
		fflush(stderr);
	}
}

void
log_warn(const char *format, ...)
{
	struct tm	*tm_info;
	time_t		 timer;
	va_list		 list;
	int		 len;
	static char	 buff[512];
	char		 cur_time[20];

	if (log_lvl_warn == 0)
		return;

	time(&timer);
	tm_info = localtime(&timer);
	strftime(cur_time, 20, "%Y-%m-%d %H:%M:%S", tm_info);

	len = snprintf(buff, sizeof(buff), "[%s] warn> ", cur_time);

	va_start(list, format);
	len += vsnprintf(buff + len, sizeof(buff) - len, format, list);
	snprintf(buff + len, sizeof(buff) - len, ": %s\n", strerror(errno));
	va_end(list);

	if (cb_log)
		cb_log(buff);
	else {
		fprintf(stderr, "%s", buff);
		fflush(stderr);
	}
}
