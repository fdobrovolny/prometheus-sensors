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

#ifdef MGOS_HAVE_VEML6075_I2C
#include "mgos_prometheus_sensors.h"
#include "mgos_time.h"
#include "mgos_veml6075.h"

static struct mgos_veml6075 *s_veml6075;

static void veml6075_prometheus_metrics(struct mg_connection *nc,
                                        void *user_data) {
  struct mgos_veml6075_stats stats;

  mgos_prometheus_metrics_printf(
      nc, GAUGE, "UV", "Ultra Violet light intensity, in sensor counts",
      "{band=\"UVA\",type=\"VEML6075\", sensor=\"0\"} %f",
      mgos_veml6075_getUVA(s_veml6075));
  mgos_prometheus_metrics_printf(
      nc, GAUGE, "UV", "Ultra Violet light intensity, in sensor counts",
      "{band=\"UVB\",type=\"VEML6075\", sensor=\"0\"} %f",
      mgos_veml6075_getUVB(s_veml6075));
  mgos_prometheus_metrics_printf(
      nc, GAUGE, "UVIndex",
      "2: Low, 5.5 Moderate, 7.5 High, 10.5 Very High, else Extreme",
      "{sensor=\"0\",type=\"VEML6075\"} %f",
      mgos_veml6075_getUVIndex(s_veml6075));

  if (mgos_veml6075_getStats(s_veml6075, &stats)) {
    mgos_prometheus_metrics_printf(
        nc, COUNTER, "sensor_read_total", "Total reads from sensor",
        "{sensor=\"0\",type=\"VEML6075\"} %u", stats.read);
    mgos_prometheus_metrics_printf(nc, COUNTER, "sensor_read_success_total",
                                   "Total successful reads from sensor",
                                   "{sensor=\"0\",type=\"VEML6075\"} %u",
                                   stats.read_success);
    mgos_prometheus_metrics_printf(
        nc, COUNTER, "sensor_read_success_cached_total",
        "Total successful cached reads from sensor",
        "{sensor=\"0\",type=\"VEML6075\"} %u", stats.read_success_cached);
    uint32_t errors =
        stats.read - stats.read_success - stats.read_success_cached;
    mgos_prometheus_metrics_printf(nc, COUNTER, "sensor_read_error_total",
                                   "Total unsuccessful reads from sensor",
                                   "{sensor=\"0\",type=\"VEML6075\"} %u",
                                   errors);
    mgos_prometheus_metrics_printf(
        nc, COUNTER, "sensor_read_success_usecs_total",
        "Total microseconds spent in reads from sensor",
        "{sensor=\"0\",type=\"VEML6075\"} %f", stats.read_success_usecs);
  }

  (void) user_data;
}

static void veml6075_timer_cb(void *user_data) {
  double start;
  uint32_t usecs = 0;
  float uva, uvb, uvindex;

  start = mgos_uptime();
  uva = mgos_veml6075_getUVA(s_veml6075);
  uvb = mgos_veml6075_getUVB(s_veml6075);
  uvindex = mgos_veml6075_getUVIndex(s_veml6075);
  usecs = 1000000 * (mgos_uptime() - start);
  LOG(LL_INFO, ("VEML6075 sensor=0 uva=%.1f uvb=%.1f uvindex=%.2f usecs=%lu",
                uva, uvb, uvindex, usecs));

  (void) user_data;
}

void veml6075_drv_init() {
  s_veml6075 = mgos_veml6075_create(
      mgos_i2c_get_global(), mgos_sys_config_get_sensors_veml6075_i2caddr());
  if (s_veml6075) {
    mgos_set_timer(mgos_sys_config_get_sensors_veml6075_period() * 1000, true,
                   veml6075_timer_cb, NULL);
    mgos_prometheus_metrics_add_handler(veml6075_prometheus_metrics, NULL);
  }
}

#else
void veml6075_drv_init() {
  LOG(LL_ERROR, ("VEML6075 disabled, include library in mos.yml to enable"));
}

#endif
