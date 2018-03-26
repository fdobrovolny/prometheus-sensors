# Prometheus Sensors

This is a library that implements a set of sensor types, reading their values
periodically and exposing them to Prometheus using the `prometheus-metrics`
library.

## Supported Sensors

### DHT 

This is using `dht` library; each sensor reading takes approximately 32ms (28ms
if there is no sensor on the GPIO in). They are placed on timers so as to
minimize pre-emption of the main application. DHT sensors are polled in order
based on `sensors.dht_gpio` pins and with a period of `sensors.dht_period`
which defaults to 3 seconds.

Please see the upstream [source](https://github.com/mongoose-os-libs/dht)
for more information on the driver.

Reported values (all types are gauges):
```
temperature{sensor="0",type="DHT"} 3.5
temperature{sensor="1",type="DHT"} 6.2
humidity{sensor="0",type="DHT"} 43.900002
humidity{sensor="1",type="DHT"} 35.599998
```

### VEML6075

This is using `veml6075-i2c` library; one sensor is allowed based on I2C
address on the bus, normally `0x10`, but configurable with `sensors.veml6075_i2caddr`
in `mos.yml`.  The chip is polled with a period of `sensors.veml6075_period`
which defaults to 3 seconds, each sensor reading takes approximately 11ms.

Please see the upstream [source](https://github.com/mongoose-os-libs/veml6075-i2c)
for more information on the driver.

Reported values (all types are gauges):
```
UV{band="UVA",sensor="0",type="VEML6075"} 0
UV{band="UVB",sensor="0",type="VEML6075"} 2
UVIndex{sensor="0",type="VEML6075"} 0.00125
```

### BME280

This is using `bme280` library; one sensor is allowed based on I2C
address on the bus, normally `0x10`, but configurable with `sensors.bme280_i2caddr`
in `mos.yml`.  The chip is polled with a period of `sensors.bme280_period`
which defaults to 3 seconds, each sensor reading takes approximately 4ms.
Note that there are several versions of this popular barometer chip: BME280
is fully fledged and includes a hygrometer (measuring the relative humidity),
while BMP280 does not.

Please see the upstream [source](https://github.com/mongoose-os-libs/bme280)
for more information on the driver.

Reported values (all types are gauges):
```
temperature{sensor="0",type="BME280"} 18.4
humidity{sensor="0",type="BME280"} 77.4
pressure{sensor="0",type="BME280"} 96720.4
```
