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
#include <ESPmDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <HTTPClient.h>
#include <HTTPUpdate.h>
#include <stdio.h>
#include <string.h>
#include "ota.h"
#include "config.h"

TaskHandle_t _OTA_Task;

void ota_Task( void * pvParameters );

/**
 * @brief ota setup function
 */
void ota_setup( void ) {
    /* 
     * set OTA parametersettuings 
     */
    ArduinoOTA.setHostname( config_get_HostName() );
    if ( strlen( config_get_OTAWlanPin() ) > 1 )
        ArduinoOTA.setPassword( config_get_OTAWlanPin() );
    /* 
     * start OTA-Server 
     */
    ArduinoOTA.onStart ([]() { log_i("Start"); } );
    ArduinoOTA.onEnd( []() { log_i("\nEnd"); } );
    ArduinoOTA.onProgress( []( unsigned int progress, unsigned int total ) { log_i("Progress: %u%%\r", ( progress / (total / 100) ) ) ; } );
    ArduinoOTA.onError( []( ota_error_t error ) {
        log_i( "Error[%u]: ", error );
        switch( error ) {
            case OTA_AUTH_ERROR:
                log_i( "Auth Failed" );
                break;
            case OTA_BEGIN_ERROR:
                log_i( "Begin Failed" );
                break;
            case OTA_CONNECT_ERROR:
                log_i( "Connect Failed" );
                break;
            case OTA_RECEIVE_ERROR:
                log_i( "Receive Failed" );
                break;
            case OTA_END_ERROR:
                log_i( "End Failed");
                break;
            default:
                break;
        }
    } );
    ArduinoOTA.begin();
}

void ota_StartTask( void ) {
    xTaskCreatePinnedToCore(
                    ota_Task,       /* Function to implement the task */
                    "ota Task",     /* Name of the task */
                    10000,          /* Stack size in words */
                    NULL,           /* Task input parameter */
                    1,              /* Priority of the task */
                    &_OTA_Task,       /* Task handle. */
                    1 );  /* Core where the task should run */  
}

int ota_scan( char * SSID ) {
    int retval = OTA_WLAN_FAILED;
    int n = 0;

    n = WiFi.scanNetworks();

    for ( int i = 0; i < n; ++i )
        log_i("%s with %ddb", WiFi.SSID( i ).c_str(), WiFi.RSSI( i ) );

    for ( int i = 0; i < n; ++i )
        if ( String( SSID ) == WiFi.SSID( i ) )
            retval = OTA_WLAN_OK;

    WiFi.disconnect();

    return( retval );
}

/**
 * @brief ota loop task
 * 
 * @param pvParameters 
 */
void ota_Task( void * pvParameters ) {
    log_i("Start OTA Task on Core: %d", xPortGetCoreID() );
    ota_setup();

    while( true ) {
        vTaskDelay( 10 );
        ArduinoOTA.handle();
    }
}
