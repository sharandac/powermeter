<p align="center">
<img src="https://img.shields.io/github/last-commit/sharandac/powermeter.svg?style=for-the-badge" />
&nbsp;
<img src="https://img.shields.io/github/license/sharandac/powermeter.svg?style=for-the-badge" />
&nbsp;
<a href="https://www.buymeacoffee.com/sharandac" target="_blank"><img src="https://img.shields.io/badge/Buy%20me%20a%20coffee-%E2%82%AC5-orange?style=for-the-badge&logo=buy-me-a-coffee" /></a>
</p>
<hr/>

# powermeter

A simple smartmeter based on ESP32. Supports the complete setup via web interface, MQTT and live monitoring via web interface.

# Install

Clone this repository and open it with platformIO. Remember, the SPIFF must also be flashed. On a terminal in vscode you can do it with
```bash
pio run -t uploadfs
pio run -t upload
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

![signalpath](https://github.com/sharandac/powermeter/blob/master/images/signalpath.png)

# Hardware

![current sensor](https://github.com/sharandac/powermeter/blob/master/images/schematic.png)

Please pay attention to the Burden resistor. Some current sensors have a built-in Burden resistor. In this example a SCT-013-000 without a built-in Burden resistor was used. Please check the documentation for the right Burden resistor value.

# Channel setting

At any time, 3 channels are read in simultaneously regardless of the settings. The individual power of each channel is calculated and the power of all channels together. If 3 channels are set, an additional virtual fourth channel is calculated with the sum of all currents. This enables us to measure the neutral conductor current indirectly. For more information about how three-phase current works, [read here](https://en.wikipedia.org/wiki/Three-phase_electric_power)!

# Interface

## live view
![live view](https://github.com/sharandac/powermeter/blob/master/images/preview.gif)
## MQTT settings
![mqtt](https://github.com/sharandac/powermeter/blob/master/images/mqtt-setting.png)
## measurement settings
![measurement](https://github.com/sharandac/powermeter/blob/master/images/measurement-setting.png)

# Contributors

Every Contribution to this repository is highly welcome! Don't fear to create pull requests which enhance or fix the project, you are going to help everybody.
<p>
If you want to donate to the author then you can buy me a coffee.
<br/><br/>
<a href="https://www.buymeacoffee.com/sharandac" target="_blank"><img src="https://img.shields.io/badge/Buy%20me%20a%20coffee-%E2%82%AC5-orange?style=for-the-badge&logo=buy-me-a-coffee" /></a>
</p>
