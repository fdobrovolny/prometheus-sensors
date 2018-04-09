#include "mgos.h"

#ifdef MGOS_HAVE_BME280
#include "mgos_bme280.h"
#include "mgos_config.h"
#include "mgos_prometheus_metrics.h"
#include "mgos_prometheus_sensors.h"

static struct mgos_bme280 *    s_bme280;
static struct mgos_bme280_data s_bme280_data;

static void bme280_prometheus_metrics(struct mg_connection *nc, void *user_data) {
  bool bme280;
  struct mgos_bme280_stats stats;

  if (!s_bme280) {
    return;
  }
  bme280 = mgos_bme280_is_bme280(s_bme280);

  mgos_prometheus_metrics_printf(nc, GAUGE,
                                 "pressure", "Barometer pressure in HPa",
                                 "{sensor=\"0\", type=\"BM%s280\"} %f", bme280 ? "E" : "P", s_bme280_data.press);

  mgos_prometheus_metrics_printf(nc, GAUGE,
                                 "temperature", "Temperature in Celcius",
                                 "{sensor=\"0\",type=\"BM%s280\"} %f", bme280 ? "E" : "P", s_bme280_data.temp);

  if (mgos_bme280_is_bme280(s_bme280)) {
    mgos_prometheus_metrics_printf(nc, GAUGE,
                                   "humidity", "Relative humidity in percent",
                                   "{sensor=\"0\",type=\"BM%s280\"} %f", bme280 ? "E" : "P", s_bme280_data.humid);
  }

  if (mgos_bme280_getStats(s_bme280, &stats)) {
    mgos_prometheus_metrics_printf(nc, COUNTER,
                                   "sensor_read_total", "Total reads from sensor",
                                   "{sensor=\"0\",type=\"BM%s280\"} %u", bme280 ? "E" : "P", stats.read);
    mgos_prometheus_metrics_printf(nc, COUNTER,
                                   "sensor_read_success_total", "Total successful reads from sensor",
                                   "{sensor=\"0\",type=\"BM%s280\"} %u", bme280 ? "E" : "P", stats.read_success);
    mgos_prometheus_metrics_printf(nc, COUNTER,
                                   "sensor_read_success_cached_total", "Total successful cached reads from sensor",
                                   "{sensor=\"0\",type=\"BM%s280\"} %u", bme280 ? "E" : "P", stats.read_success_cached);
    uint32_t errors = stats.read - stats.read_success - stats.read_success_cached;
    mgos_prometheus_metrics_printf(nc, COUNTER,
                                   "sensor_read_error_total", "Total unsuccessful reads from sensor",
                                   "{sensor=\"0\",type=\"BM%s280\"} %u", bme280 ? "E" : "P", errors);
    mgos_prometheus_metrics_printf(nc, COUNTER,
                                   "sensor_read_success_usecs_total", "Total microseconds spent in reads from sensor",
                                   "{sensor=\"0\",type=\"BM%s280\"} %f", bme280 ? "E" : "P", stats.read_success_usecs);
  }

  (void)user_data;
}

static void bme280_timer_cb(void *user_data) {
  struct mgos_bme280_stats stats_before, stats_after;
  uint32_t usecs = 0;

  mgos_bme280_getStats(s_bme280, &stats_before);
  mgos_bme280_read(s_bme280, &s_bme280_data);
  mgos_bme280_getStats(s_bme280, &stats_after);

  usecs = stats_after.read_success_usecs - stats_before.read_success_usecs;

  if (mgos_bme280_is_bme280(s_bme280)) {
    LOG(LL_INFO, ("BME280 sensor=0 humidity=%.2f%% temperature=%.2fC pressure=%.1fHPa usecs=%u", s_bme280_data.humid, s_bme280_data.temp, s_bme280_data.press, usecs));
  } else {
    LOG(LL_INFO, ("BMP280 sensor=0 temperature=%.2fC pressure=%.1fHPa usecs=%u", s_bme280_data.temp, s_bme280_data.press, usecs));
  }

  (void)user_data;
}

void bme280_drv_init() {
  s_bme280 = mgos_bme280_i2c_create(mgos_sys_config_get_sensors_bme280_i2caddr());
  if (s_bme280) {
    mgos_set_timer(mgos_sys_config_get_sensors_bme280_period() * 1000, true, bme280_timer_cb, NULL);
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
