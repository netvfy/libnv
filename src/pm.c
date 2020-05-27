/*
 * Copyright (c) 2020 Nicolas Bouliane <nicboul@gmail.com>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include <inttypes.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#define PM_COUNTER	"counter"
#define PM_GAUGE	"gauge"
#define PM_HISTOGRAM	"histogram"

#define PM_LEN			128
#define PM_LABEL_MAX		5
#define PM_STORE_MAX		5
#define PM_METRIC_MAX		30
#define PM_HIST_BIN_MAX		10

typedef struct {
	char		name[PM_LEN];
	char		help[PM_LEN];
	char		type[PM_LEN];
	char		label[PM_LABEL_MAX][PM_LEN];
	double		bin[PM_HIST_BIN_MAX];
	uint32_t	n_label;	/* number of labels for that metric */
	uint32_t	n_bin;		/* number of bins for that histogram */
	uint32_t	n_store;	/* number of stores for that metric */
	struct {
		char	lblval[PM_LABEL_MAX][PM_LEN];
		union {
			double		counter;
			double		gauge;
			struct {
				double		sum;
				uint64_t	cnt[PM_HIST_BIN_MAX];
				uint64_t	inf;
			} histogram;
		};
	} s[PM_STORE_MAX];
} pm_metric;

static int32_t		pm_metric_get_store_index(pm_metric *, va_list *);
static int32_t		pm_dump_counter(pm_metric *, char *, const size_t);
static int32_t		pm_dump_gauge(pm_metric *, char *, const size_t);
static int32_t		pm_dump_histogram(pm_metric *, char *, const size_t);
static int32_t		pm_dump_label(pm_metric *, uint32_t, char *, const size_t);

static pm_metric	metric[PM_METRIC_MAX];
static uint32_t		n_metric;

/*
 * Returns the store index of a metric by matching the labels and the
 * labels values, if no match is found, an empty store index is returned.
 * If no store is available or an error occurs, -1 is returned.
 *
 * A metric has labels, and these labels can have different values. Every
 * permutation of these values correspond to a different store where a unique
 * counter, gauge or histogram is stored.
 *
 * labels: [user][node]
 * -------------------
 * values: [bob][nyc]   <- store 0
 * values: [alice][nyc] <- store 1
 * values: [bob][flo]   <- store 2
 *
 * A metric can have up to PM_STORE_MAX stores.
 */
int32_t
pm_metric_get_store_index(pm_metric *m, va_list *ap1)
{
	va_list		ap;
	uint32_t	i, j;
	uint32_t	ret;
	int32_t		idx = -1;
	uint8_t		match;
	const char	*label;
	char		lbltmp[PM_LABEL_MAX][PM_LEN];

	/* reorder the labels received */
	va_copy(ap, *ap1);
	for (i = 0; i < m->n_label; i++) {
		label = va_arg(ap, const char *);

		for (j = 0; j < m->n_label; j++) {
			if (strcmp(label, m->label[j]) == 0)
				break;
		}
		ret = snprintf(lbltmp[j], sizeof(lbltmp[j]), "\"%s\"",
			va_arg(ap, const char *));
		if (ret < 0 || ret >= sizeof(lbltmp[j])) goto out;
	}

	/* try to find a store where the labels values match,
	 * return a new store otherwise, or an error if all
	 * the stores are filled */
	match = 0;
	for (i = 0; i < PM_STORE_MAX; i++) {
		/* skip empty store */
		if (strlen(m->s[i].lblval[0]) == 0) {
			if (idx == -1)
				idx = i;
			continue;
		}
		/* skip if a value doesn't match */
		for (j = 0; j < m->n_label; j++) {
			if (strcmp(m->s[i].lblval[j], lbltmp[j]) != 0)
				break;
		}
		if (j == m->n_label) {
			match = 1;
			idx = i;
			break;
		}
	}

	/* copy the labels values to the new store */
	if (match == 0 && idx != -1) {
		for (i = 0; i < m->n_label; i++) {
			ret = snprintf(m->s[idx].lblval[i],
				sizeof(m->s[idx].lblval[i]), "%s", lbltmp[i]);
			if (ret < 0 || ret >= sizeof(lbltmp[j])) goto out;
		}
		m->n_store++;
	}
out:
	va_end(ap);
	return (idx);
}

/*
 * Creates and Returns a new metric. If an error occurs, NULL is returned. A
 * maximum of PM_METRIC_MAX can be created. This function expect that `name`,
 * `help` and `type` have a maximum lenght of PM_LEN-1. A metric can have a
 * maximum of PM_LABEL_MAX labels and each label can have a maximum lenght of
 * PM_LEN-1.
 */
pm_metric *
pm_metric_add(const char *name, const char *help, const char *type, const uint32_t n_label, ...)
{
	pm_metric	*m = NULL;
	va_list		 args;
	uint32_t	 i, j;
	int32_t		 ret = 0;

	for (i = 0; i < PM_METRIC_MAX; i++)
		if (strlen(metric[i].name) == 0)
			break;

	if (i == PM_METRIC_MAX)
		goto out;

	va_start(args, n_label);
	for (j = 0; j < n_label && j < PM_LABEL_MAX; j++) {
		ret = snprintf(metric[i].label[j], sizeof(metric[i].label[j]),
			"%s", va_arg(args, const char *));
		if (ret < 0 || ret >= sizeof(metric[i].label[j])) goto out;
	}

	ret = snprintf(metric[i].help, sizeof(metric[i].help), "%s", help);
	if (ret < 0 || ret >= sizeof(metric[i].help)) goto out;

	ret = snprintf(metric[i].type, sizeof(metric[i].type), "%s", type);
	if (ret < 0 || ret >= sizeof(metric[i].type)) goto out;

	ret = snprintf(metric[i].name, sizeof(metric[i].name), "%s", name);
	if (ret < 0 || ret >= sizeof(metric[i].name)) {
		metric[i].name[0] = '\0';
		goto out;
	}

	metric[i].n_label = n_label;
	m = &metric[i];
	n_metric++;
out:
	va_end(args);
	return (m);
}

/*
 * Create the bin values of an histogram. This function expect that every bin
 * received in parameter is of type double. if an error occurs, -1 is returned.
 */
int32_t
pm_metric_bins(pm_metric *m, const uint32_t n_bin, ...)
{
	va_list		args;
	uint32_t	i;
	int32_t		err = -1;

	if (strcmp(m->type, PM_HISTOGRAM) != 0)
		goto out;

	va_start(args, n_bin);
	for (i = 0; i < n_bin && i < PM_HIST_BIN_MAX; i++)
		m->bin[i] = va_arg(args, double);

	m->n_bin = i;
	err = 0;
out:
	va_end(args);
	return (err);
}

/*
 * Add the value passed in argument to the counter. The value added
 * can't be lower than 0. This function expect the metric labels and
 * their value in parameter. If an error occurs, -1 is returned.
 */
int32_t
pm_counter_add(pm_metric *m, const double value, ...)
{
	va_list	ap;
	int32_t	idx;
	int32_t	err = -1;

	if (strcmp(m->type, PM_COUNTER) != 0)
		goto out;

	if (value < 0)
		goto out;

	va_start(ap, value);
	if ((idx = pm_metric_get_store_index(m, &ap)) < 0)
		goto out;

	m->s[idx].counter += value;
	err = 0;
out:
	va_end(ap);
	return (err);
}

/*
 * Set the gauge to the value passed in argument. This function expect
 * the metric labels and their value in parameter. If an error occurs,
 * -1 is returned.
 */
int32_t
pm_gauge_set(pm_metric *m, const double value, ...)
{
	va_list	ap;
	int32_t	idx;
	int32_t	err = -1;

	if (strcmp(m->type, PM_GAUGE) != 0)
		goto out;

	va_start(ap, value);
	if ((idx = pm_metric_get_store_index(m, &ap)) < 0)
		goto out;

	m->s[idx].gauge = value;
	err = 0;
out:
	va_end(ap);
	return (err);
}

/*
 * Add the value received in argument to the gauge. This function expect
 * the metric labels and their value in parameter. If an error occurs,
 * -1 is returned.
 */
int32_t
pm_gauge_add(pm_metric *m, const double value, ...)
{
	va_list	ap;
	int32_t	idx;
	int32_t	err = -1;

	if (strcmp(m->type, PM_GAUGE) != 0)
		goto out;

	va_start(ap, value);
	if ((idx = pm_metric_get_store_index(m, &ap)) < 0)
		goto out;

	m->s[idx].gauge += value;
	err = 0;
out:
	va_end(ap);
	return (err);
}

/* Increase the hitcount of the bin matching the observed. This function expect
 * in parameter the metric labels and their values. If an error occurs,
 * -1 is returned.
 */
int32_t
pm_histogram_observe(pm_metric *m, const double value, ...)
{
	va_list		ap;
	uint32_t	i;
	int32_t		idx;
	int32_t		err = -1;

	if (strcmp(m->type, PM_HISTOGRAM) != 0)
		goto out;

	va_start(ap, value);
	if ((idx = pm_metric_get_store_index(m, &ap)) < 0)
		goto out;

	for (i = 0; i < m->n_bin; i++) {
		if (value <= m->bin[i])
			m->s[idx].histogram.cnt[i]++;
	}
	if (i == m->n_bin)
		m->s[idx].histogram.inf++;

	m->s[idx].histogram.sum += value;
	err = 0;
out:
	va_end(ap);
	return (err);
}

/* Print the counter metric in a buffer. If the return value is greater than
 * or equal to the bufsz argument, the buffer was too short and some of the
 * printed characters were discarded. If an error occurs, -1 is returned.
 */
int32_t
pm_dump_counter(pm_metric *m, char *buf, const size_t bufsz)
{
	uint32_t	i_store;
	int32_t		ret = 0, off = 0;

	if (m->n_store == 0)
		goto out;

	for (i_store = 0; i_store < m->n_store; i_store++) {
		if (i_store > 0) {
			ret = snprintf(buf+off, bufsz-off, "\n");
			if (ret < 0 || ret >= bufsz-off) goto error;
			off += ret;
		}
		ret = snprintf(buf+off, bufsz-off, "%s{", m->name);
		if (ret < 0 || ret >= bufsz-off) goto error;
		off += ret;

		ret = pm_dump_label(m, i_store, buf+off, bufsz-off);
		if (ret < 0 || ret >= bufsz-off) goto error;
		off += ret;

		ret = snprintf(buf+off, bufsz-off, "} %f", m->s[i_store].counter);
		if (ret < 0 || ret >= bufsz-off) goto error;
		off += ret;
	}
out:
	return (off);
error:
	off += ret;
	if (ret < 0)
		off = ret;
	return (off);
}

/* Print the gauge metric in a buffer. If the return value is greater than
 * or equal to the bufsz argument, the buffer was too short and some of the
 * printed characters were discarded. If an error occurs, -1 is returned.
 */
int32_t
pm_dump_gauge(pm_metric *m, char *buf, const size_t bufsz)
{
	uint32_t	i_store;
	int32_t		ret = 0, off = 0;

	if (m->n_store == 0)
		goto out;

	for (i_store = 0; i_store < m->n_store; i_store++) {
		if (i_store > 0) {
			ret = snprintf(buf+off, bufsz-off, "\n");
			if (ret < 0 || ret >= bufsz-off) goto error;
			off += ret;
		}
		ret = snprintf(buf+off, bufsz-off, "%s{", m->name);
		if (ret < 0 || ret >= bufsz-off) goto error;
		off += ret;

		ret = pm_dump_label(m, i_store, buf+off, bufsz-off);
		if (ret < 0 || ret >= bufsz-off) goto error;
		off += ret;

		ret = snprintf(buf+off, bufsz-off, "} %f", m->s[i_store].gauge);
		if (ret < 0 || ret >= bufsz-off) goto error;
		off += ret;
	}
out:
	return (off);
error:
	off += ret;
	if (ret < 0)
		off = ret;
	return (off);
}

/* Print the histogram metric in a buffer. If the return value is greater than
 * or equal to the bufsz argument, the buffer was too short and some of the
 * printed characters were discarded. If an error occurs, -1 is returned.
 */
int32_t
pm_dump_histogram(pm_metric *m, char *buf, const size_t bufsz)
{
	uint32_t	i_bin;
	uint32_t	i_store;
	int32_t		ret = 0, off = 0;
	char		lbltmp[(PM_LEN * PM_LABEL_MAX * 2) + (PM_LABEL_MAX * 2 - 1)];

	if (m->n_bin == 0 || m->n_store == 0)
		goto out;

	for (i_store = 0; i_store < m->n_store; i_store++) {
		ret = pm_dump_label(m, i_store, lbltmp, sizeof(lbltmp));
		if (ret < 0 || ret >= sizeof(lbltmp)) {
			ret = -1;
			goto error;
		}

		if (i_store > 0) {
			ret = snprintf(buf+off, bufsz-off, "\n");
			if (ret < 0 || ret >= bufsz-off) goto error;
			off += ret;
		}

		for (i_bin = 0; i_bin < m->n_bin; i_bin++) {
			ret = snprintf(buf+off, bufsz-off, "%s_bucket{%s,le=\"%f\"} %"PRIu64"\n",
				m->name, lbltmp, m->bin[i_bin], m->s[i_store].histogram.cnt[i_bin]);
			if (ret < 0 || ret >= bufsz-off) goto error;
			off += ret;
		}

		ret = snprintf(buf+off, bufsz-off, "%s_bucket{%s,le=\"+Inf\"} %"PRIu64"\n",
			m->name, lbltmp, m->s[i_store].histogram.inf);
		if (ret < 0 || ret >= bufsz-off) goto error;
		off += ret;

		ret = snprintf(buf+off, bufsz-off, "%s_sum{%s} %f\n",
			m->name, lbltmp, m->s[i_store].histogram.sum);
		if (ret < 0 || ret >= bufsz-off) goto error;
		off += ret;

		ret = snprintf(buf+off, bufsz-off, "%s_count{%s} %"PRIu64"",
			m->name, lbltmp, m->s[i_store].histogram.inf);
		if (ret < 0 || ret >= bufsz-off) goto error;
		off += ret;
	}
out:
	return (off);
error:
	off += ret;
	if (ret < 0)
		off = ret;
	return (off);
}

/* Print the metric labels and values in a buffer. If the return value is
 * greater than or equal to the bufsz argument, the buffer was too short and
 * some of the printed characters were discarded. If an error occurs, -1 is
 * returned.
 */
int32_t
pm_dump_label(pm_metric *m, uint32_t storeidx, char *buf, const size_t bufsz)
{
	uint32_t	i;
	int32_t		ret = 0, off = 0;

	for (i = 0; i < m->n_label; i++) {
		if (i > 0) {
			ret = snprintf(buf+off, bufsz-off, ",");
			if (ret < 0 || ret >= bufsz-off) goto error;
			off += ret;
		}

		ret = snprintf(buf+off, bufsz-off, "%s=%s",
			m->label[i], m->s[storeidx].lblval[i]);
		if (ret < 0 || ret >= bufsz-off) goto error;
		off += ret;
	}

	return (off);
error:
	off += ret;
	if (ret < 0)
		off = ret;
	return (off);
}

/* Print the metric in a buffer. If the return value is greater than or equal
 * to the bufsz argument, the buffer was too short and some of the printed
 * characters were discarded. If an error occurs, -1 is returned.
 */
int32_t
pm_dump(char *buf, const size_t bufsz)
{
	uint32_t	i;
	int32_t		ret = 0, off = 0;

	for (i = 0; i < n_metric; i++) {
		if (i > 0) {
			ret = snprintf(buf+off, bufsz-off, "\n\n");
			if (ret < 0 || ret >= bufsz-off) goto error;
			off += ret;
		}

		ret = snprintf(buf+off, bufsz-off, "# HELP %s %s\n# TYPE %s %s\n",
			metric[i].name, metric[i].help,  metric[i].name, metric[i].type);
		if (ret < 0 || ret >= bufsz-off) goto error;
		off += ret;

		if (strcmp(metric[i].type, PM_COUNTER) == 0) {
			ret = pm_dump_counter(&metric[i], buf+off, bufsz-off);
			if (ret < 0 || ret >= bufsz-off) goto error;
			off += ret;

		} else if (strcmp(metric[i].type, PM_GAUGE) == 0) {
			ret = pm_dump_gauge(&metric[i], buf+off, bufsz-off);
			if (ret < 0 || ret >= bufsz-off) goto error;
			off += ret;

		} else if (strcmp(metric[i].type, PM_HISTOGRAM) == 0) {
			ret = pm_dump_histogram(&metric[i], buf+off, bufsz-off);
			if (ret < 0 || ret >= bufsz-off) goto error;
			off += ret;
		}
	}

	return (off);
error:
	off += ret;
	if (ret < 0)
		off = ret;
	return (off);
}

int
example()
{
	pm_metric	*m1, *m2, *m3;

	char m1_label1[] = "node";
	char m1_label2[] = "user";

	m1 = pm_metric_add("controller_http_request_duration_seconds_count", \
		"Total number of connected agents", PM_COUNTER, \
		2, m1_label1, m1_label2);

	if (m1 == NULL) {
		printf("failed m1\n");
		return (-1);
	}

	pm_counter_add(m1, 1000, "node", "nyc1", "user", "bob");
	pm_counter_add(m1, 1000.40, "node", "nyc1", "user", "bob");

	pm_counter_add(m1, 3000, "node", "nyc1", "user", "alice");
	pm_counter_add(m1, 3000.50, "node", "nyc1", "user", "alice");


	m2 = pm_metric_add("controller_agent_connection_count_total", \
		"Total number of connected agent", PM_GAUGE,
		2, m1_label1, m1_label2);

	if (m2 == NULL) {
		printf("failed m2\n");
		return (-1);
	}

	pm_gauge_set(m2, 23, m1_label1, "tor1", m1_label2, "roger");
	pm_gauge_add(m2, -5, m1_label1, "tor1", m1_label2, "roger");

	pm_gauge_set(m2, 500, m1_label1, "miami1", m1_label2, "roger");
	pm_gauge_add(m2, -30, m1_label1, "miami1", m1_label2, "roger");


	m3 = pm_metric_add("controller_http_req_duration_seconds", \
		"Histogram of HTTP request duration in seconds", PM_HISTOGRAM,
		3, "node", "user", "handler");

	if (m3 == NULL) {
		printf("failed m3\n");
		return (-1);
	}

	pm_metric_bins(m3, 5, 0.05, 0.1, 0.2, 0.5, 1.0);

	pm_histogram_observe(m3, 0.04, "node", "miami1", "user", "bob", "handler", "/v1/");
	pm_histogram_observe(m3, 0.09, "node", "miami1", "user", "bob", "handler", "/v1/");
	pm_histogram_observe(m3, 0.19, "node", "miami1", "user", "bob", "handler", "/v1/");
	pm_histogram_observe(m3, 0.49, "node", "miami1", "user", "bob", "handler", "/v1/");
	pm_histogram_observe(m3, 0.99, "node", "miami1", "user", "bob", "handler", "/v1/");
	pm_histogram_observe(m3, 1.99, "node", "miami1", "user", "bob", "handler", "/v1/");

	pm_histogram_observe(m3, 0.03, "node", "miami1", "user", "bob", "handler", "/v2/");
	pm_histogram_observe(m3, 0.08, "node", "miami1", "user", "bob", "handler", "/v2/");
	pm_histogram_observe(m3, 0.18, "node", "miami1", "user", "bob", "handler", "/v2/");
	pm_histogram_observe(m3, 0.48, "node", "miami1", "user", "bob", "handler", "/v2/");
	pm_histogram_observe(m3, 0.98, "node", "miami1", "user", "bob", "handler", "/v2/");
	pm_histogram_observe(m3, 1.98, "node", "miami1", "user", "bob", "handler", "/v2/");

	pm_histogram_observe(m3, 0.041, "node", "tor1", "user", "bob", "handler", "/v2/");
	pm_histogram_observe(m3, 0.091, "node", "tor1", "user", "bob", "handler", "/v2/");
	pm_histogram_observe(m3, 0.191, "node", "tor1", "user", "bob", "handler", "/v2/");
	pm_histogram_observe(m3, 0.491, "node", "tor1", "user", "bob", "handler", "/v2/");
	pm_histogram_observe(m3, 0.991, "node", "tor1", "user", "bob", "handler", "/v2/");
	pm_histogram_observe(m3, 1.991, "node", "tor1", "user", "bob", "handler", "/v2/");

	int	total;
	char	buf[4000];

	total = pm_dump(buf, sizeof(buf));
	printf("%s\n", buf);
	printf("total: %d == %lu", total, sizeof(buf));

	return (0);
}
