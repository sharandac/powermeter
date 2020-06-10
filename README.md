# powermeter

A simple smartmeter based on ESP32. Supports the complete setup via web interface, MQTT and live monitoring via web interface.

# Install

Clone this repository and open it with platformIO.
Remember, the SPIFF must also be flashed. On a terminal in vscode you can do it with
```bash
platformio run -t uploadfs
platformio run -t upload
```

After that, take a look at your monitorport ...

```text
Read config from SPIFFS
scan for SSID "" ... not found
starting Wifi-AP with SSID "powermeter_aadee0"
AP IP address: 192.168.4.1
Start Main Task on Core: 1
Start NTP Task on Core: 1
Start Measurement Task on Core: 0
Start Websock Task on Core: 1
Start MQTT-Client on Core: 1
Start Webserver on Core: 1
NTP-client: renew time
Start OTA Task on Core: 1
Failed to obtain time
NTP-client: Thursday, January 01 1970 01:00:07
```
When the output look like this, congratulation!

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
# How it works

![signalpath](https://github.com/sharandac/powermeter/blob/master/data/signalpath.png)

# Hardware

![current sensor](https://github.com/sharandac/powermeter/blob/master/data/schematic.png)

Please pay attention to the Burden resistor. Some current sensors have a built-in Burden resistor. In this example a SCT-013-000 without a built-in Burden resistor was used. Please check the documentation for the right Burden resistor value.

# Interface

## live view
![live view](https://github.com/sharandac/powermeter/blob/master/data/live-view.png)
## MQTT settings
![mqtt](https://github.com/sharandac/powermeter/blob/master/data/mqtt-setting.png)
## measurement settings
![measurement](https://github.com/sharandac/powermeter/blob/master/data/measurement-setting.png)

