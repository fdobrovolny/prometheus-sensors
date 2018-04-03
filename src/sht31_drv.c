#include "mgos.h"

#ifdef MGOS_HAVE_SHT31_I2C
#include "mgos_sht31.h"
#include "mgos_config.h"
#include "mgos_prometheus_metrics.h"
#include "mgos_prometheus_sensors.h"

static struct mgos_sht31 *s_sht31;

static void sht31_prometheus_metrics(struct mg_connection *nc, void *user_data) {
  mgos_prometheus_metrics_printf(nc, GAUGE,
    "temperature", "Temperature in Celcius",
    "{sensor=\"0\",type=\"SHT31\"} %f", mgos_sht31_getTemperature(s_sht31));
  mgos_prometheus_metrics_printf(nc, GAUGE,
    "humidity", "Relative humidity percentage",
    "{sensor=\"0\",type=\"SHT31\"} %f", mgos_sht31_getHumidity(s_sht31));

  (void) user_data;
}

static void sht31_timer_cb(void *user_data) {
  double start;
  uint32_t usecs=0;
  float temperature, humidity;

  start=mgos_uptime();
  temperature=mgos_sht31_getTemperature(s_sht31);
  humidity=mgos_sht31_getHumidity(s_sht31);
  usecs=1000000*(mgos_uptime()-start);
  LOG(LL_INFO, ("SHT31 sensor=0 temperature=%.2fC humidity=%.1f%% usecs=%u", temperature, humidity, usecs));

  (void) user_data;
}

void sht31_drv_init() {
#ifdef MGOS_HAVE_SI7021_I2C
#endif
  s_sht31 = mgos_sht31_create(mgos_i2c_get_global(), mgos_sys_config_get_sensors_sht31_i2caddr());
  if (s_sht31) {
    mgos_set_timer(mgos_sys_config_get_sensors_sht31_period()*1000, true, sht31_timer_cb, NULL);
    mgos_prometheus_metrics_add_handler(sht31_prometheus_metrics, NULL);
  }
}

#else
void sht31_drv_init() {
  LOG(LL_ERROR, ("SHT31 disabled, include library in mos.yml to enable"));
}
#endif
