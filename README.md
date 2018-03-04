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
