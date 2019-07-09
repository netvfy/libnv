#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

static void	(*cb_log)(const char *) = NULL;

void
log_setcb(void (*cb)(const char *))
{
	cb_log = cb;
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

	time(&timer);
	tm_info = localtime(&timer);
	strftime(cur_time, 20, "%Y-%m-%d %H:%M:%S", tm_info);

	len = snprintf(buff, sizeof(buff), "[%s] ", cur_time);

	va_start(list, format);
	len += vsnprintf(buff + len, sizeof(buff) - len, format, list);
	snprintf(buff + len, sizeof(buff) - len, "\n");
	va_end(list);

	if (cb_log)
		cb_log(buff);
	else
		fprintf(stdout, "%s", buff);
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

	time(&timer);
	tm_info = localtime(&timer);
	strftime(cur_time, 20, "%Y-%m-%d %H:%M:%S", tm_info);

	len = snprintf(buff, sizeof(buff), "[%s] ", cur_time);

	va_start(list, format);
	len += vsnprintf(buff, sizeof(buff), format, list);
	snprintf(buff + len, sizeof(buff) - len, "\n");
	va_end(list);

	if (cb_log)
		cb_log(buff);
	else
		fprintf(stderr, "%s", buff);
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

	time(&timer);
	tm_info = localtime(&timer);
	strftime(cur_time, 20, "%Y-%m-%d %H:%M:%S", tm_info);

	len = snprintf(buff, sizeof(buff), "[%s] ", cur_time);

	va_start(list, format);
	len += vsnprintf(buff, sizeof(buff), format, list);
	snprintf(buff + len, sizeof(buff) - len, ": %s\n", strerror(errno));
	va_end(list);

	if (cb_log)
		cb_log(buff);
	else
		fprintf(stderr, "%s", buff);
}
