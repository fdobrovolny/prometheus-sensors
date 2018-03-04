#include "mgos_config.h"
#include "mgos_prometheus_sensors.h"

void dht_init();

bool mgos_prometheus_sensors_init(void) {
  dht_init();
  return true;
}
