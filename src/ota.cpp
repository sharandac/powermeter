/****************************************************************************
 *            ota.cpp
 *
 *  Sa April 27 12:17:32 2019
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

/**
 *
 * \author Dirk Broßwick
 *
 */
#include <ESPmDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>

#include <HTTPClient.h>
#include <HTTPUpdate.h>

#include <stdio.h>
#include <string.h>

#include "mqttclient.h"
#include "config.h"
#include "ota.h"

TaskHandle_t _OTA_Task;
/*
 * 
 */
void ota_setup( void ) {
  /* 
   * set OTA parametersettuings 
   */
  ArduinoOTA.setHostname( config_get_HostName() );
  if ( strlen( config_get_OTAWlanPin() ) > 1 ) ArduinoOTA.setPassword( (const char *) config_get_OTAWlanPin() );

  /* 
   * start OTA-Server 
   */
  ArduinoOTA.onStart([]() { Serial.println("Start"); });
  ArduinoOTA.onEnd([]() { Serial.println("\nEnd"); });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) { Serial.printf("Progress: %u%%\r", (progress / (total / 100))); });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
    else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
    else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
    else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
    else if (error == OTA_END_ERROR) Serial.println("End Failed");
    });
  ArduinoOTA.begin();
}

/*
 * 
 */
void ota_StartTask( void ) {
    xTaskCreatePinnedToCore(
                    ota_Task,       /* Function to implement the task */
                    "ota Task",     /* Name of the task */
                    10000,          /* Stack size in words */
                    NULL,           /* Task input parameter */
                    1,              /* Priority of the task */
                    &_OTA_Task,       /* Task handle. */
                    _OTA_TASKCORE );  /* Core where the task should run */  
}


/*
 * \brief Scant das Wlan nach einen bestimmten Wlan (OTA_WLAN_SSID) und schaltet wenn vorhanden
 *        in den OTA Modus
 */
int ota_scan( char * SSID ) {
  int retval = OTA_WLAN_FAILED;
  int n = 0;

  /*
   * scanne nach SSID
   */
  n = WiFi.scanNetworks();

  /*
   * durchsuche die gescannten SSID nach OTAWlanSSID
   */
  for (int i = 0; i < n; ++i) {
    if ( String( SSID ) == WiFi.SSID(i) ) {
      retval = OTA_WLAN_OK;
    }
  }

  WiFi.disconnect();

  return( retval );
}

/*
 * 
 */
void ota_Task( void * pvParameters ) {

  ota_setup();
  Serial.printf("Start OTA Task on Core: %d\r\n", xPortGetCoreID() );

  while( true ) {
    vTaskDelay( 10 );
    ArduinoOTA.handle();
  }
}
