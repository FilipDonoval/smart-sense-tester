# ESP-MODEM

## Setup

### Add Dependency

- idf.py add-dependency "espressif/esp_modem^2.0.1"

### Pin Setup

- To power modem and SD card set PERIPHERAL_POWER_CONTROL pin 12 to high.
- Set PWR 4 to high for modem to turn on then PWR 4 back to low (wait after for around 10s).

### DTE Setup

#### For LilyGo A7670

- dte_config.uart_config.tx_io_num = 26;
- dte_config.uart_config.rx_io_num = 27;
- dte_config.uart_config.rts_io_num = 5;

### DCE Setup

#### For LilyGo A7670

- Using SIM7600 modem
- esp_modem_new_dev(ESP_MODEM_DCE_SIM7600, &dte_config,
                                           &dce_config, esp_netif)

## Errors

- Not waiting for modem to turn on (around 10s) after setting pin 4 to high then low. Fixed by adding delay after starting it.

## Docs

### LilyGo A7670 Pin Map

| Name                                 | GPIO NUM | Free |
| ------------------------------------ | -------- | ---- |
| Peripheral power control             | 12       | ❌    |
| Modem TX                             | 26       | ❌    |
| Modem RX                             | 27       | ❌    |
| Modem PWR                            | 4        | ❌    |
| Modem RESET                          | 5        | ❌    |
| Modem RING                           | 33       | ❌    |
| Modem DTR                            | 25       | ❌    |
| SD SCK                               | 14       | ❌    |
| SD MISO                              | 2        | ❌    |
| SD MOSI                              | 15       | ❌    |
| SD CS                                | 13       | ❌    |
| Battery ADC Pin                      | 35       | ❌    |
| Solar ADC Pin                        | 36       | ❌    |
| Default SDA                          | 21       | ✅️    |
| Default SCL                          | 22       | ✅️    |
| GNSS Tx    (A7670G-GPS Version Only) | 21       | ❌    |
| GNSS Rx    (A7670G-GPS Version Only) | 22       | ❌    |
| GNSS PPS   (A7670G-GPS Version Only) | 23       | ❌    |
| GNSS WakeUp(A7670G-GPS Version Only) | 19       | ❌    |
