#include "mgos_config.h"
#include "mgos_prometheus_metrics.h"
#include "mgos_prometheus_sensors.h"

void dht_drv_init();
void veml6075_drv_init();
void bme280_drv_init();
void sht31_drv_init();
void si7021_drv_init();
void htu21df_drv_init();
void mcp9808_drv_init();

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

  if (mgos_sys_config_get_sensors_pushgateway_period()>0)
    mgos_set_timer(mgos_sys_config_get_sensors_pushgateway_period()*1000, true, pushgateway_timer, NULL);
  return true;
}
