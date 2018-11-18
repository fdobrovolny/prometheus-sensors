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

#ifdef MGOS_HAVE_BAROMETER
#include "mgos_barometer.h"
#include "mgos_prometheus_sensors.h"
#include <math.h>

#define NUM_BARO    5
struct mgos_barometer *s_barometer[NUM_BARO];

static void barometer_prometheus_metrics(struct mg_connection *nc, void *user_data) {
  struct mgos_barometer_stats stats;
  float temperature = NAN, humidity = NAN, pressure = NAN;

  for (int i = 0; i < NUM_BARO && s_barometer[i]; i++) {
    const char *type = mgos_barometer_get_device_name(s_barometer[i]);

    if (mgos_barometer_get_temperature(s_barometer[i], &temperature)) {
      mgos_prometheus_metrics_printf(nc, GAUGE,
                                     "temperature", "Temperature in Celcius",
                                     "{sensor=\"%d\",type=\"%s\"} %f", i, type, temperature);
    }

    if (mgos_barometer_get_pressure(s_barometer[i], &pressure)) {
      mgos_prometheus_metrics_printf(nc, GAUGE,
                                     "pressure", "Barometer pressure in Pascals",
                                     "{sensor=\"%d\", type=\"%s\"} %f", i, type, pressure);
    }

    if (mgos_barometer_get_humidity(s_barometer[i], &humidity)) {
      mgos_prometheus_metrics_printf(nc, GAUGE,
                                     "humidity", "Relative humidity percentage",
                                     "{sensor=\"%d\",type=\"%s\"} %f", i, type, humidity);
    }

    if (mgos_barometer_get_stats(s_barometer[i], &stats)) {
      mgos_prometheus_metrics_printf(nc, COUNTER,
                                     "sensor_read_total", "Total reads from sensor",
                                     "{sensor=\"%d\",type=\"%s\"} %u", i, type, stats.read);
      mgos_prometheus_metrics_printf(nc, COUNTER,
                                     "sensor_read_success_total", "Total successful reads from sensor",
                                     "{sensor=\"%d\",type=\"%s\"} %u", i, type, stats.read_success);
      mgos_prometheus_metrics_printf(nc, COUNTER,
                                     "sensor_read_success_cached_total", "Total successful cached reads from sensor",
                                     "{sensor=\"%d\",type=\"%s\"} %u", i, type, stats.read_success_cached);
      uint32_t errors = stats.read - stats.read_success - stats.read_success_cached;
      mgos_prometheus_metrics_printf(nc, COUNTER,
                                     "sensor_read_error_total", "Total unsuccessful reads from sensor",
                                     "{sensor=\"%d\",type=\"%s\"} %u", i, type, errors);
      mgos_prometheus_metrics_printf(nc, COUNTER,
                                     "sensor_read_success_usecs_total", "Total microseconds spent in reads from sensor",
                                     "{sensor=\"%d\",type=\"%s\"} %f", i, type, stats.read_success_usecs);
    }
  }

  (void)user_data;
  (void)humidity;
}

static void barometer_timer_cb(void *user_data) {
  struct mgos_barometer *baro = (struct mgos_barometer *)user_data;
  const char *           type = mgos_barometer_get_device_name(baro);

  float temperature = NAN, humidity = NAN, pressure = NAN;
  struct mgos_barometer_stats stats_before, stats_after;
  uint32_t usecs = 0;

  mgos_barometer_get_stats(baro, &stats_before);
  mgos_barometer_get_temperature(baro, &temperature);
  mgos_barometer_get_pressure(baro, &pressure);
  mgos_barometer_get_humidity(baro, &humidity);
  mgos_barometer_get_stats(baro, &stats_after);

  usecs = stats_after.read_success_usecs - stats_before.read_success_usecs;
  LOG(LL_INFO, ("%s temperature=%.2fC humidity=%.1f%% pressure=%.0fPa usecs=%u", type, temperature, humidity, pressure, usecs));

  (void)user_data;
}

static struct mgos_barometer *barometer_create(struct mgos_i2c *i2c, uint8_t i2caddr, enum mgos_barometer_type type) {
  struct mgos_barometer *b;

  b = mgos_barometer_create_i2c(i2c, i2caddr, type);
  if (!b) {
    return NULL;
  }
  mgos_barometer_set_cache_ttl(b, 1000);
  mgos_set_timer(mgos_sys_config_get_sensors_barometer_period() * 1000, true, barometer_timer_cb, b);
  return b;
}

void barometer_drv_init() {
  int num_baro = 0;

  if ((s_barometer[num_baro] = barometer_create(mgos_i2c_get_global(), 0x76, BARO_BME280))) {
    num_baro++;
  }

  if ((s_barometer[num_baro] = barometer_create(mgos_i2c_get_global(), 0x60, BARO_MPL115))) {
    num_baro++;
  }

  if ((s_barometer[num_baro] = barometer_create(mgos_i2c_get_global(), 0x60, BARO_MPL3115))) {
    num_baro++;
  }

  if ((s_barometer[num_baro] = barometer_create(mgos_i2c_get_global(), 0x77, BARO_MS5611))) {
    num_baro++;
  }

  if (num_baro > 0) {
    mgos_prometheus_metrics_add_handler(barometer_prometheus_metrics, NULL);
  }
}

#else
void barometer_drv_init() {
  LOG(LL_ERROR, ("Barometer disabled, include library in mos.yml to enable"));
}

#endif
