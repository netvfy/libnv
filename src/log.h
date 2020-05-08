#ifndef LOG_H
#define LOG_H

#define LOG_LVL_DEBUG	0x1
#define LOG_LVL_INFO	0x2
#define LOG_LVL_WARN	0x3

void	log_setcb(void (*cb)(const char *));
void	log_debug(const char *format, ...);
void	log_info(const char *format, ...);
void	log_warnx(const char *format, ...);
void	log_warn(const char *format, ...);

#endif
