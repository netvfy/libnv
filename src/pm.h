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

#ifndef PM_H
#define PM_H

#include <stddef.h>
#include <stdint.h>

typedef struct pm_metric pm_metric;

pm_metric	*pm_metric_add(const char *, const char *, const char *, const uint32_t, ...);
int32_t		 pm_metric_bins(pm_metric *, const uint32_t, ...);
int32_t		 pm_counter_add(pm_metric *, const double, ...);
int32_t		 pm_gauge_set(pm_metric *, const double, ...);
int32_t		 pm_gauge_add(pm_metric *, const double, ...);
int32_t		 pm_histogram_observe(pm_metric *, const double, ...);
int32_t		 pm_dump(char *, const size_t);

#endif
