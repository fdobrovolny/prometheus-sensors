#include "mgos.h"

#ifdef MGOS_HAVE_HTU21DF_I2C
#include "mgos_htu21df.h"
#include "mgos_config.h"
#include "mgos_prometheus_metrics.h"
#include "mgos_prometheus_sensors.h"

static struct mgos_htu21df *s_htu21df;

static void htu21df_prometheus_metrics(struct mg_connection *nc, void *user_data) {
  mgos_prometheus_metrics_printf(nc, GAUGE,
    "temperature", "Temperature in Celcius",
    "{sensor=\"0\",type=\"HTU21DF\"} %f", mgos_htu21df_getTemperature(s_htu21df));
  mgos_prometheus_metrics_printf(nc, GAUGE,
    "humidity", "Relative humidity percentage",
    "{sensor=\"0\",type=\"HTU21DF\"} %f", mgos_htu21df_getHumidity(s_htu21df));

  (void) user_data;
}

static void htu21df_timer_cb(void *user_data) {
  double start;
  uint32_t usecs=0;
  float temperature, humidity;

  start=mgos_uptime();
  temperature=mgos_htu21df_getTemperature(s_htu21df);
  humidity=mgos_htu21df_getHumidity(s_htu21df);
  usecs=1000000*(mgos_uptime()-start);
  LOG(LL_INFO, ("HTU21DF sensor=0 temperature=%.2f humidity=%.1f usecs=%u", temperature, humidity, usecs));

  (void) user_data;
}

void htu21df_drv_init() {
  s_htu21df = mgos_htu21df_create(mgos_i2c_get_global(), mgos_sys_config_get_sensors_htu21df_i2caddr());
  if (s_htu21df) {
    mgos_set_timer(mgos_sys_config_get_sensors_htu21df_period()*1000, true, htu21df_timer_cb, NULL);
    mgos_prometheus_metrics_add_handler(htu21df_prometheus_metrics, NULL);
  }
}

#else
void htu21df_drv_init() {
  LOG(LL_ERROR, ("HTU21DF disabled, include library in mos.yml to enable"));
}
#endif
