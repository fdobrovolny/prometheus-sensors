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

#ifdef MGOS_HAVE_SI7021_I2C
#include "mgos_prometheus_sensors.h"
#include "mgos_si7021.h"

static struct mgos_si7021 *s_si7021;

static void si7021_prometheus_metrics(struct mg_connection *nc,
                                      void *user_data) {
  struct mgos_si7021_stats stats;

  mgos_prometheus_metrics_printf(nc, GAUGE, "temperature",
                                 "Temperature in Celcius",
                                 "{sensor=\"0\",type=\"SI7021\"} %f",
                                 mgos_si7021_getTemperature(s_si7021));
  mgos_prometheus_metrics_printf(
      nc, GAUGE, "humidity", "Relative humidity percentage",
      "{sensor=\"0\",type=\"SI7021\"} %f", mgos_si7021_getHumidity(s_si7021));

  if (mgos_si7021_getStats(s_si7021, &stats)) {
    mgos_prometheus_metrics_printf(
        nc, COUNTER, "sensor_read_total", "Total reads from sensor",
        "{sensor=\"0\",type=\"SI7021\"} %u", stats.read);
    mgos_prometheus_metrics_printf(nc, COUNTER, "sensor_read_success_total",
                                   "Total successful reads from sensor",
                                   "{sensor=\"0\",type=\"SI7021\"} %u",
                                   stats.read_success);
    mgos_prometheus_metrics_printf(
        nc, COUNTER, "sensor_read_success_cached_total",
        "Total successful cached reads from sensor",
        "{sensor=\"0\",type=\"SI7021\"} %u", stats.read_success_cached);
    uint32_t errors =
        stats.read - stats.read_success - stats.read_success_cached;
    mgos_prometheus_metrics_printf(nc, COUNTER, "sensor_read_error_total",
                                   "Total unsuccessful reads from sensor",
                                   "{sensor=\"0\",type=\"SI7021\"} %u", errors);
    mgos_prometheus_metrics_printf(
        nc, COUNTER, "sensor_read_success_usecs_total",
        "Total microseconds spent in reads from sensor",
        "{sensor=\"0\",type=\"SI7021\"} %f", stats.read_success_usecs);
  }

  (void) user_data;
}

static void si7021_timer_cb(void *user_data) {
  float temperature, humidity;
  struct mgos_si7021_stats stats_before, stats_after;
  uint32_t usecs = 0;

  mgos_si7021_getStats(s_si7021, &stats_before);
  temperature = mgos_si7021_getTemperature(s_si7021);
  humidity = mgos_si7021_getHumidity(s_si7021);
  mgos_si7021_getStats(s_si7021, &stats_after);

  usecs = stats_after.read_success_usecs - stats_before.read_success_usecs;
  LOG(LL_INFO, ("SI7021 sensor=0 temperature=%.2fC humidity=%.1f%% usecs=%lu",
                temperature, humidity, usecs));

  (void) user_data;
}

void si7021_drv_init() {
#ifdef MGOS_HAVE_HTU21DF_I2C
  LOG(LL_WARN, ("HTU21DF and SI7021 are both on I2C address 0x40 -- do not "
                "enable both!"));
#endif
  s_si7021 = mgos_si7021_create(mgos_i2c_get_global(),
                                mgos_sys_config_get_sensors_si7021_i2caddr());
  if (s_si7021) {
    mgos_set_timer(mgos_sys_config_get_sensors_si7021_period() * 1000, true,
                   si7021_timer_cb, NULL);
    mgos_prometheus_metrics_add_handler(si7021_prometheus_metrics, NULL);
  }
}

#else
void si7021_drv_init() {
  LOG(LL_ERROR, ("SI7021 disabled, include library in mos.yml to enable"));
}

#endif
