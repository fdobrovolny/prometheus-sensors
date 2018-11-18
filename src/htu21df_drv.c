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

#ifdef MGOS_HAVE_HTU21DF_I2C
#include "mgos_htu21df.h"
#include "mgos_prometheus_sensors.h"

static struct mgos_htu21df *s_htu21df;

static void htu21df_prometheus_metrics(struct mg_connection *nc, void *user_data) {
  struct mgos_htu21df_stats stats;

  mgos_prometheus_metrics_printf(nc, GAUGE,
                                 "temperature", "Temperature in Celcius",
                                 "{sensor=\"0\",type=\"HTU21DF\"} %f", mgos_htu21df_getTemperature(s_htu21df));
  mgos_prometheus_metrics_printf(nc, GAUGE,
                                 "humidity", "Relative humidity percentage",
                                 "{sensor=\"0\",type=\"HTU21DF\"} %f", mgos_htu21df_getHumidity(s_htu21df));

  if (mgos_htu21df_getStats(s_htu21df, &stats)) {
    mgos_prometheus_metrics_printf(nc, COUNTER,
                                   "sensor_read_total", "Total reads from sensor",
                                   "{sensor=\"0\",type=\"HTU21DF\"} %u", stats.read);
    mgos_prometheus_metrics_printf(nc, COUNTER,
                                   "sensor_read_success_total", "Total successful reads from sensor",
                                   "{sensor=\"0\",type=\"HTU21DF\"} %u", stats.read_success);
    mgos_prometheus_metrics_printf(nc, COUNTER,
                                   "sensor_read_success_cached_total", "Total successful cached reads from sensor",
                                   "{sensor=\"0\",type=\"HTU21DF\"} %u", stats.read_success_cached);
    uint32_t errors = stats.read - stats.read_success - stats.read_success_cached;
    mgos_prometheus_metrics_printf(nc, COUNTER,
                                   "sensor_read_error_total", "Total unsuccessful reads from sensor",
                                   "{sensor=\"0\",type=\"HTU21DF\"} %u", errors);
    mgos_prometheus_metrics_printf(nc, COUNTER,
                                   "sensor_read_success_usecs_total", "Total microseconds spent in reads from sensor",
                                   "{sensor=\"0\",type=\"HTU21DF\"} %f", stats.read_success_usecs);
  }

  (void)user_data;
}

static void htu21df_timer_cb(void *user_data) {
  float temperature, humidity;
  struct mgos_htu21df_stats stats_before, stats_after;
  uint32_t usecs = 0;

  mgos_htu21df_getStats(s_htu21df, &stats_before);
  temperature = mgos_htu21df_getTemperature(s_htu21df);
  humidity    = mgos_htu21df_getHumidity(s_htu21df);
  mgos_htu21df_getStats(s_htu21df, &stats_after);

  usecs = stats_after.read_success_usecs - stats_before.read_success_usecs;
  LOG(LL_INFO, ("HTU21DF sensor=0 temperature=%.2fC humidity=%.1f%% usecs=%u", temperature, humidity, usecs));

  (void)user_data;
}

void htu21df_drv_init() {
  #ifdef MGOS_HAVE_SI7021_I2C
  LOG(LL_WARN, ("HTU21DF and SI7021 are both on I2C address 0x40 -- do not enable both!"));
  #endif
  s_htu21df = mgos_htu21df_create(mgos_i2c_get_global(), mgos_sys_config_get_sensors_htu21df_i2caddr());
  if (s_htu21df) {
    mgos_set_timer(mgos_sys_config_get_sensors_htu21df_period() * 1000, true, htu21df_timer_cb, NULL);
    mgos_prometheus_metrics_add_handler(htu21df_prometheus_metrics, NULL);
  }
}

#else
void htu21df_drv_init() {
  LOG(LL_ERROR, ("HTU21DF disabled, include library in mos.yml to enable"));
}

#endif
