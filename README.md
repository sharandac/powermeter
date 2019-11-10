# powermeter

A simple smartmeter based on ESP32. Supports the complete setup via web interface, MQTT and live monitoring via web interface.

# Install

Clone this repository and open it with platformIO.
Remember, the SPIFF must also be flashed.

After the first start an access point will be opened with an unique name like
```bash
powermeter_XXXXX
```
and an not so unique password
```bash
powermeter
```
After that you can configure the powermeter under the following IP-address with your favorite webbrowser
```bash
http://192.168.4.1 or powermeter_xxxxxx.local
```

# Interface

## live view
![live view](https://github.com/sharandac/powermeter/blob/master/data/live-view.png)
## MQTT settings
![mqtt](https://github.com/sharandac/powermeter/blob/master/data/mqtt-setting.png)
## measurement settings
![measurement](https://github.com/sharandac/powermeter/blob/master/data/measurement-setting.png)

