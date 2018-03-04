#ifndef __MGOS_PROMETHEUS_SENSORS_H
#define __MGOS_PROMETHEUS_SENSORS_H

#include "mgos.h"
#include "mgos_gpio.h"

float mgos_prometheus_sensors_dht_get_temp(uint8_t idx);
float mgos_prometheus_sensors_dht_get_humidity(uint8_t idx);

#endif // __MGOS_PROMETHEUS_SENSORS_H
