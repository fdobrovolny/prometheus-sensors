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

#include "mgos_prometheus_sensors_internal.h"

static void pushgateway_timer(void *user_data) {
  mgos_prometheus_metrics_push(MGOS_APP, mgos_sys_config_get_device_id());
  (void) user_data;
}

bool mgos_prometheus_sensors_init(void) {
  dht_drv_init();
  veml6075_drv_init();
  bme280_drv_init();
  si7021_drv_init();
  sht31_drv_init();
  htu21df_drv_init();
  mcp9808_drv_init();
  ccs811_drv_init();
  barometer_drv_init();

  if (mgos_sys_config_get_sensors_pushgateway_period() > 0) {
    mgos_set_timer(mgos_sys_config_get_sensors_pushgateway_period() * 1000,
                   true, pushgateway_timer, NULL);
  }
  return true;
}
