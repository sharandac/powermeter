  /****************************************************************************
 *            measure.cpp
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

#include "measure.h"
#include "config.h"
#include "ota.h"
#include "webserver.h"
#include "mqttclient.h"
#include "ntp.h"

static bool APMODE = false;
/*
 * 
 */
void connectWiFi() {
  int wlan_timeout = WLAN_CONNECT_TIMEOUT;

  if ( APMODE == true ) return;
  /*
   * Check if WiFi connected
   */
  WiFi.setHostname( config_get_HostName() );

  if ( WiFi.status() != WL_CONNECTED ) {

    Serial.printf("WiFi lost, restart ... ");
    /*
     * start a new connection and wait for it
     */
    while ( WiFi.status() != WL_CONNECTED ){
        WiFi.begin( config_get_WlanSSID() , config_get_WlanPassord() );
        delay(1000);
        if ( wlan_timeout <= 0 ) {
          APMODE = true;
          break;
        }
        wlan_timeout--;
    }

    if ( APMODE == true ) {
      WiFi.softAP( config_get_OTALocalApSSID(), config_get_OTALocalApPassword() );
      IPAddress myIP = WiFi.softAPIP();
      Serial.printf("failed\r\nstarting Wifi-AP with SSID \"%s\"\r\n", config_get_OTALocalApSSID() );
      Serial.printf("AP IP address: ");
      Serial.println(myIP);      
    }
    else {
      Serial.printf("connected\r\nIP address: " );
      Serial.println( WiFi.localIP() );
    }
  }
}

/*
 * 
 */
void setup() 
{
  /*
   * doing setup Serial an config
   */
  Serial.begin(115200);
  config_setup();

  /*
   * scan for SSID, if noct, setup an own AP
   */
  Serial.printf("scan for SSID \"%s\" ... ", config_get_WlanSSID() );  
  if ( ota_scan( config_get_WlanSSID() ) == OTA_WLAN_OK ) {
    WiFi.mode( WIFI_STA );
    Serial.printf("found\r\nconnect to %s\r\n", config_get_WlanSSID() );
    WiFi.begin( config_get_WlanSSID() , config_get_WlanPassord() );    
  }
  else {
    Serial.printf("not found\r\nstarting Wifi-AP with SSID \"%s\"\r\n", config_get_OTALocalApSSID() );
    WiFi.softAP( config_get_OTALocalApSSID(), config_get_OTALocalApPassword() );
    IPAddress myIP = WiFi.softAPIP();
    Serial.print("AP IP address: ");
    Serial.println(myIP);
    APMODE = true; 
  }

  /*
   * Connect to WiFi
   */
  connectWiFi();

  /*
   * Setup Tasks
   */
  Serial.printf("Start Main Task on Core: %d\r\n", xPortGetCoreID() );
  ntp_StartTask();
  ota_StartTask();
  mqtt_client_StartTask();
  asyncwebserver_StartTask();
  measure_StartTask();
}

/*
 * 
 */
void loop() {
  delay(10);
  /*
   * WiFi check status for reconnect
   */
  connectWiFi();
}
