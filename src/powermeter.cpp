  /****************************************************************************
 *            powermeter.cpp
 *
 *  Sa April 27 09:23:14 2019
 *  Copyright  2019  Dirk Brosswick 
 *  Email: dirk.brosswick@googlemail.com
 ****************************************************************************/ 
/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */ 
#include <Arduino.h>
#include <WiFi.h>
#include <esp_wifi.h>

#include "measure.h"
#include "config.h"
#include "ota.h"
#include "webserver.h"
#include "mqttclient.h"
#include "ntp.h"

/**
 * @brief check WiFi connect and reconnect if network available or start a OTA softAP
 */
void connectWiFi( void ) {
    static bool APMODE = false;
    static bool firstRun = true;
    /*
     * basic setup on first run 
     */
    if( firstRun ) {
        WiFi.mode( WIFI_STA );
        esp_wifi_set_bandwidth( ESP_IF_WIFI_STA, WIFI_BW_HT20 );
        esp_wifi_set_ps( WIFI_PS_NONE );
        WiFi.setHostname( config_get_HostName() );
        firstRun = false;
    }
    /*
     * Check if WiFi connected
     */
    if ( WiFi.status() != WL_CONNECTED ) {
        int wlan_timeout = WLAN_CONNECT_TIMEOUT;

        log_i("WiFi connection lost, restart ... ");
        log_i("scan for SSID \"%s\" ... ", config_get_WlanSSID() );
        ota_scan( config_get_WlanSSID() );

        WiFi.begin( config_get_WlanSSID() , config_get_WlanPassord() );
        while ( WiFi.status() != WL_CONNECTED ){
            delay(1000);
            if ( wlan_timeout <= 0 ) {
                if( !APMODE ) {
                    WiFi.softAP( config_get_OTALocalApSSID(), config_get_OTALocalApPassword() );
                    log_i("failed and starting Wifi-AP with SSID \"%s\"", config_get_OTALocalApSSID() );
                    log_i("AP IP address: %s", WiFi.softAPIP().toString().c_str() );
                    APMODE = true;
                }
                break;
            }
            wlan_timeout--;
        }
        if ( WiFi.status() == WL_CONNECTED ) {
            log_i("connected, IP address: %s", WiFi.localIP().toString().c_str() );
        }
    }
}

/**
 * @brief arduino setup function
 */
void setup( void ) {
    /*
     * doing setup Serial an config
     */
    Serial.begin(115200);
    config_setup();
    connectWiFi();
    /*
     * Setup Tasks
     */
    log_i("Start Main Task on Core: %d", xPortGetCoreID() );
    ntp_StartTask();
    ota_StartTask();
    mqtt_client_StartTask();
    asyncwebserver_StartTask();
    measure_StartTask();
}

/**
 * @brief arduino main loop
 */
void loop() {
    /*
     * WiFi check status for reconnect
     */
    connectWiFi();
    delay(10);
}
