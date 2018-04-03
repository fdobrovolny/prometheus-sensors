#include "mgos.h"

#ifdef MGOS_HAVE_SI7021_I2C
#include "mgos_si7021.h"
#include "mgos_config.h"
#include "mgos_prometheus_metrics.h"
#include "mgos_prometheus_sensors.h"

static struct mgos_si7021 *s_si7021;

static void si7021_prometheus_metrics(struct mg_connection *nc, void *user_data) {
  mgos_prometheus_metrics_printf(nc, GAUGE,
    "temperature", "Temperature in Celcius",
    "{sensor=\"0\",type=\"SI7021\"} %f", mgos_si7021_getTemperature(s_si7021));
  mgos_prometheus_metrics_printf(nc, GAUGE,
    "humidity", "Relative humidity percentage",
    "{sensor=\"0\",type=\"SI7021\"} %f", mgos_si7021_getHumidity(s_si7021));

  (void) user_data;
}

static void si7021_timer_cb(void *user_data) {
  double start;
  uint32_t usecs=0;
  float temperature, humidity;

  start=mgos_uptime();
  temperature=mgos_si7021_getTemperature(s_si7021);
  humidity=mgos_si7021_getHumidity(s_si7021);
  usecs=1000000*(mgos_uptime()-start);
  LOG(LL_INFO, ("SI7021 sensor=0 temperature=%.2fC humidity=%.1f%% usecs=%u", temperature, humidity, usecs));

  (void) user_data;
}

void si7021_drv_init() {
#ifdef MGOS_HAVE_HTU21DF_I2C
  LOG(LL_WARN, ("HTU21DF and SI7021 are both on I2C address 0x40 -- do not enable both!"));
#endif
  s_si7021 = mgos_si7021_create(mgos_i2c_get_global(), mgos_sys_config_get_sensors_si7021_i2caddr());
  if (s_si7021) {
    mgos_set_timer(mgos_sys_config_get_sensors_si7021_period()*1000, true, si7021_timer_cb, NULL);
    mgos_prometheus_metrics_add_handler(si7021_prometheus_metrics, NULL);
  }
}

#else
void si7021_drv_init() {
  LOG(LL_ERROR, ("SI7021 disabled, include library in mos.yml to enable"));
}
#endif
