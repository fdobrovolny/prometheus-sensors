/*
 * Copyright 2018 Google Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "mgos.h"

#ifdef MGOS_HAVE_CCS811_I2C
#include "mgos_ccs811.h"
#include "mgos_prometheus_sensors.h"

static struct mgos_ccs811 *s_ccs811;

static void ccs811_prometheus_metrics(struct mg_connection *nc, void *user_data) {
  struct mgos_ccs811_stats stats;

  mgos_prometheus_metrics_printf(nc, GAUGE,
                                 "eco2", "Effective CO2 in ppm",
                                 "{sensor=\"0\",type=\"CCS811\"} %f", mgos_ccs811_get_eco2(s_ccs811));
  mgos_prometheus_metrics_printf(nc, GAUGE,
                                 "tvoc", "Total Volatile Organic Compounds in ppb",
                                 "{sensor=\"0\",type=\"CCS811\"} %f", mgos_ccs811_get_tvoc(s_ccs811));

  if (mgos_ccs811_getStats(s_ccs811, &stats)) {
    mgos_prometheus_metrics_printf(nc, COUNTER,
                                   "sensor_read_total", "Total reads from sensor",
                                   "{sensor=\"0\",type=\"CCS811\"} %u", stats.read);
    mgos_prometheus_metrics_printf(nc, COUNTER,
                                   "sensor_read_success_total", "Total successful reads from sensor",
                                   "{sensor=\"0\",type=\"CCS811\"} %u", stats.read_success);
    mgos_prometheus_metrics_printf(nc, COUNTER,
                                   "sensor_read_success_cached_total", "Total successful cached reads from sensor",
                                   "{sensor=\"0\",type=\"CCS811\"} %u", stats.read_success_cached);
    uint32_t errors = stats.read - stats.read_success - stats.read_success_cached;
    mgos_prometheus_metrics_printf(nc, COUNTER,
                                   "sensor_read_error_total", "Total unsuccessful reads from sensor",
                                   "{sensor=\"0\",type=\"CCS811\"} %u", errors);
    mgos_prometheus_metrics_printf(nc, COUNTER,
                                   "sensor_read_success_usecs_total", "Total microseconds spent in reads from sensor",
                                   "{sensor=\"0\",type=\"CCS811\"} %f", stats.read_success_usecs);
  }

  (void)user_data;
}

static void ccs811_timer_cb(void *user_data) {
  float eco2, tvoc;
  struct mgos_ccs811_stats stats_before, stats_after;
  uint32_t usecs = 0;

  mgos_ccs811_getStats(s_ccs811, &stats_before);
  eco2 = mgos_ccs811_get_eco2(s_ccs811);
  tvoc = mgos_ccs811_get_tvoc(s_ccs811);
  mgos_ccs811_getStats(s_ccs811, &stats_after);

  usecs = stats_after.read_success_usecs - stats_before.read_success_usecs;
  LOG(LL_INFO, ("CCS811 sensor=0 eCO2=%.0fppm TVOC=%.0fppb usecs=%u", eco2, tvoc, usecs));

  (void)user_data;
}

void ccs811_drv_init() {
  s_ccs811 = mgos_ccs811_create(mgos_i2c_get_global(), mgos_sys_config_get_sensors_ccs811_i2caddr());
  if (s_ccs811) {
    mgos_set_timer(mgos_sys_config_get_sensors_ccs811_period() * 1000, true, ccs811_timer_cb, NULL);
    mgos_prometheus_metrics_add_handler(ccs811_prometheus_metrics, NULL);
  }
}

#else
void ccs811_drv_init() {
  LOG(LL_ERROR, ("CCS811 disabled, include library in mos.yml to enable"));
}

#endif
