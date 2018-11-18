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

#pragma once

#pragma once

#include "mgos.h"
#include "mgos_prometheus_sensors.h"

void dht_drv_init();
void veml6075_drv_init();
void bme280_drv_init();
void sht31_drv_init();
void si7021_drv_init();
void htu21df_drv_init();
void mcp9808_drv_init();
void ccs811_drv_init();
void barometer_drv_init();
