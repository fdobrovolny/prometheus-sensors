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

#ifdef MGOS_HAVE_SHT31_I2C
#include "mgos_prometheus_sensors.h"
#include "mgos_sht31.h"

static struct mgos_sht31 *s_sht31;

static void sht31_prometheus_metrics(struct mg_connection *nc,
                                     void *user_data) {
  struct mgos_sht31_stats stats;

  mgos_prometheus_metrics_printf(
      nc, GAUGE, "temperature", "Temperature in Celcius",
      "{sensor=\"0\",type=\"SHT31\"} %f", mgos_sht31_getTemperature(s_sht31));
  mgos_prometheus_metrics_printf(
      nc, GAUGE, "humidity", "Relative humidity percentage",
      "{sensor=\"0\",type=\"SHT31\"} %f", mgos_sht31_getHumidity(s_sht31));

  if (mgos_sht31_getStats(s_sht31, &stats)) {
    mgos_prometheus_metrics_printf(
        nc, COUNTER, "sensor_read_total", "Total reads from sensor",
        "{sensor=\"0\",type=\"SHT31\"} %u", stats.read);
    mgos_prometheus_metrics_printf(nc, COUNTER, "sensor_read_success_total",
                                   "Total successful reads from sensor",
                                   "{sensor=\"0\",type=\"SHT31\"} %u",
                                   stats.read_success);
    mgos_prometheus_metrics_printf(
        nc, COUNTER, "sensor_read_success_cached_total",
        "Total successful cached reads from sensor",
        "{sensor=\"0\",type=\"SHT31\"} %u", stats.read_success_cached);
    uint32_t errors =
        stats.read - stats.read_success - stats.read_success_cached;
    mgos_prometheus_metrics_printf(nc, COUNTER, "sensor_read_error_total",
                                   "Total unsuccessful reads from sensor",
                                   "{sensor=\"0\",type=\"SHT31\"} %u", errors);
    mgos_prometheus_metrics_printf(
        nc, COUNTER, "sensor_read_success_usecs_total",
        "Total microseconds spent in reads from sensor",
        "{sensor=\"0\",type=\"SHT31\"} %f", stats.read_success_usecs);
  }

  (void) user_data;
}

static void sht31_timer_cb(void *user_data) {
  float temperature, humidity;
  struct mgos_sht31_stats stats_before, stats_after;
  uint32_t usecs = 0;

  mgos_sht31_getStats(s_sht31, &stats_before);
  temperature = mgos_sht31_getTemperature(s_sht31);
  humidity = mgos_sht31_getHumidity(s_sht31);
  mgos_sht31_getStats(s_sht31, &stats_after);

  usecs = stats_after.read_success_usecs - stats_before.read_success_usecs;
  LOG(LL_INFO, ("SHT31 sensor=0 temperature=%.2fC humidity=%.1f%% usecs=%lu",
                temperature, humidity, usecs));

  (void) user_data;
}

void sht31_drv_init() {
#ifdef MGOS_HAVE_SI7021_I2C
#endif
  s_sht31 = mgos_sht31_create(mgos_i2c_get_global(),
                              mgos_sys_config_get_sensors_sht31_i2caddr());
  if (s_sht31) {
    mgos_set_timer(mgos_sys_config_get_sensors_sht31_period() * 1000, true,
                   sht31_timer_cb, NULL);
    mgos_prometheus_metrics_add_handler(sht31_prometheus_metrics, NULL);
  }
}

#else
void sht31_drv_init() {
  LOG(LL_ERROR, ("SHT31 disabled, include library in mos.yml to enable"));
}

#endif
