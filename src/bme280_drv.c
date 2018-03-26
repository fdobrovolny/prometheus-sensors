#include "mgos.h"

#ifdef MGOS_HAVE_BME280
#include "mgos_bme280.h"
#include "mgos_config.h"
#include "mgos_prometheus_metrics.h"
#include "mgos_prometheus_sensors.h"

static struct mgos_bme280 *s_bme280;
static struct mgos_bme280_data s_bme280_data;

static void bme280_prometheus_metrics(struct mg_connection *nc, void *user_data) {

  if (!s_bme280) return;

  mgos_prometheus_metrics_printf(nc, GAUGE,
    "pressure", "Barometer pressure in HPa",
    "{sensor=\"0\", type=\"BME280\"} %f", s_bme280_data.press);

  mgos_prometheus_metrics_printf(nc, GAUGE,
    "temperature", "Temperature in Celcius",
    "{sensor=\"0\",type=\"BME280\"} %f", s_bme280_data.temp);

  if (mgos_bme280_is_bme280(s_bme280)) {
    mgos_prometheus_metrics_printf(nc, GAUGE,
      "humidity", "Relative humidity in percent",
      "{sensor=\"0\",type=\"BME280\"} %f", s_bme280_data.humid);
  }

  (void) user_data;
}

static void bme280_timer_cb(void *user_data) {
  double start;
  uint32_t usecs=0;

  start=mgos_uptime();
  mgos_bme280_read(s_bme280, &s_bme280_data);
  usecs=1000000*(mgos_uptime()-start);

  if (mgos_bme280_is_bme280(s_bme280)) {
    LOG(LL_INFO, ("sensor=0 humidity=%.1f%% temperature=%.1fC pressure=%.1fHPa usecs=%u", s_bme280_data.humid, s_bme280_data.temp, s_bme280_data.press, usecs));
  } else {
    LOG(LL_INFO, ("sensor=0 temperature=%.1fC pressure=%.1fHPa usecs=%u", s_bme280_data.temp, s_bme280_data.press, usecs));
  }

  (void) user_data;
}

void bme280_drv_init() {
  s_bme280 = mgos_bme280_i2c_create(mgos_sys_config_get_sensors_bme280_i2caddr());
  if (s_bme280) {
    mgos_set_timer(mgos_sys_config_get_sensors_bme280_period()*1000, true, bme280_timer_cb, NULL);
    mgos_prometheus_metrics_add_handler(bme280_prometheus_metrics, NULL);
  } else {
    LOG(LL_ERROR, ("Could not create BME280 sensor on I2C address 0x%02x", mgos_sys_config_get_sensors_bme280_i2caddr()));
  }
}

#else
void bme280_drv_init() {
  LOG(LL_ERROR, ("BME280 disabled, include library in mos.yml to enable"));
}
#endif
