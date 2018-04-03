#include "mgos.h"

#ifdef MGOS_HAVE_MCP9808_I2C
#include "mgos_mcp9808.h"
#include "mgos_config.h"
#include "mgos_prometheus_metrics.h"
#include "mgos_prometheus_sensors.h"

static struct mgos_mcp9808 *s_mcp9808;

static void mcp9808_prometheus_metrics(struct mg_connection *nc, void *user_data) {
  mgos_prometheus_metrics_printf(nc, GAUGE,
    "temperature", "Temperature in Celcius",
    "{sensor=\"0\",type=\"MCP9808\"} %f", mgos_mcp9808_getTemperature(s_mcp9808));
  (void) user_data;
}

static void mcp9808_timer_cb(void *user_data) {
  double start;
  uint32_t usecs=0;
  float temperature;

  start=mgos_uptime();
  temperature=mgos_mcp9808_getTemperature(s_mcp9808);
  usecs=1000000*(mgos_uptime()-start);
  LOG(LL_INFO, ("MCP9808 sensor=0 temperature=%.2f usecs=%u", temperature, usecs));

  (void) user_data;
}

void mcp9808_drv_init() {
  s_mcp9808 = mgos_mcp9808_create(mgos_i2c_get_global(), mgos_sys_config_get_sensors_mcp9808_i2caddr());
  if (s_mcp9808) {
    mgos_set_timer(mgos_sys_config_get_sensors_mcp9808_period()*1000, true, mcp9808_timer_cb, NULL);
    mgos_prometheus_metrics_add_handler(mcp9808_prometheus_metrics, NULL);
  }
}

#else
void mcp9808_drv_init() {
  LOG(LL_ERROR, ("MCP9808 disabled, include library in mos.yml to enable"));
}
#endif
