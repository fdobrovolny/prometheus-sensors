#include "mgos_config.h"
#include "mgos_prometheus_metrics.h"
#include "mgos_prometheus_sensors.h"

void dht_init();

static void pushgateway_timer(void *user_data) {
  mgos_prometheus_metrics_push(MGOS_APP, mgos_sys_config_get_device_id());
  (void) user_data;
}

bool mgos_prometheus_sensors_init(void) {
  dht_init();
  if (mgos_sys_config_get_sensors_pushgateway_period()>0)
    mgos_set_timer(mgos_sys_config_get_sensors_pushgateway_period()*1000, true, pushgateway_timer, NULL);
  return true;
}
