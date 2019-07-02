#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

static void	(*cb_log)(const char *) = NULL;

void
log_setcb(void (*cb)(const char *))
{
	cb_log = cb;
}

void
log_info(const char *format, ...)
{
	va_list		 list;
	static char	 buff[512];
	int		 len;

	va_start(list, format);
	len = vsnprintf(buff, sizeof(buff), format, list);
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
	va_list		 list;
	static char	 buff[512];
	int		 len;

	va_start(list, format);
	len = vsnprintf(buff, sizeof(buff), format, list);
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
	va_list		 list;
	static char	 buff[512];
	int		 len;

	va_start(list, format);
	len = vsnprintf(buff, sizeof(buff), format, list);
	snprintf(buff + len, sizeof(buff) - len, ": %s\n", strerror(errno));
	va_end(list);

	if (cb_log)
		cb_log(buff);
	else
		fprintf(stderr, "%s", buff);
}
