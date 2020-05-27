/****************************************************************************
 *            websock.cpp
 *
 *  Sa April 27 12:01:00 2019
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
#include <WebSocketsServer.h>

#include "websock.h"
#include "config.h"
#include "measure.h"

WebSocketsServer webSocket = WebSocketsServer( WEBSOCK_PORT );
TaskHandle_t _WEBSOCK_Task;

/*
 * Websocket-Eventroutine. Abarbeitung der eingehenden Daten auf Port WEBSOCK_PORT
 */
void webSocketEvent(uint8_t num, WStype_t type, uint8_t * cmdpayload, size_t length) {

  switch (type) {
    case WStype_DISCONNECTED:           { 
                                          Serial.printf("WebSock-Client: Client disconnected\r\n");
                                          break;
                                        }
    case WStype_ERROR:                  { 
                                          Serial.printf("WebSock-Client: Error\r\n");
                                          break;
                                        }
    case WStype_BIN:                    { break; }
    case WStype_PING:                    { break; }
    case WStype_PONG:                    { break; }
    case WStype_FRAGMENT:               { break; }
    case WStype_FRAGMENT_BIN_START:     { break; }
    case WStype_FRAGMENT_FIN:           { break; }
    case WStype_FRAGMENT_TEXT_START:    { break; }
    case WStype_CONNECTED:              { 
                                          Serial.printf("WebSock-Client: Client connected\r\n");
                                          break;
                                        }
    case WStype_TEXT: {
      const char * cmd = (const char *)cmdpayload;
      char * value = (char *)cmdpayload;
      while( *value ) {
        if ( *value == '\\' ) {
          *value = '\0';
          value++;
          break;
        }
        value++;
      }

      /* Wlan SSID */
      if ( !strcmp("SAV", cmd ) ) {
        Serial.printf("Store Settings\r\n" );
        config_saveall();
        webSocket.sendTXT(num, "status\\Save" );
      }
      else if ( !strcmp("OSC", cmd ) ) {
        char stringbuffer[ numberOfSamples * 4 + 96 ]="";
        char tmp[64]="";
        uint16_t * mybuffer;

        sprintf( tmp,"OScopeProbe\\%d\\%f\\", numberOfSamples, measure_get_Iratio() );
        strcat( stringbuffer, tmp );

        mybuffer = measure_get_buffer( 0 );

        for( int i = 0 ; i < numberOfSamples ; i++ ) {
          sprintf(tmp, "%04x", mybuffer[ i ] );
          strcat( stringbuffer, tmp );
        }
        webSocket.sendTXT(num, stringbuffer );
      }
      /* Wlan SSID */
      else if ( !strcmp("STA", cmd ) ) {
        char request[256]="";

        webSocket.sendTXT(num, "status\\online" );
        sprintf( request, "WSS\\%s", config_get_WlanSSID() ); webSocket.sendTXT(num, request );
        sprintf( request, "WPS\\%s", config_get_WlanPassord() ); webSocket.sendTXT(num, request );
        sprintf( request, "OPP\\%s", config_get_OTAWlanPin() ); webSocket.sendTXT(num, request );
        sprintf( request, "ASS\\%s", config_get_OTALocalApSSID() ); webSocket.sendTXT(num, request );
        sprintf( request, "APS\\%s", config_get_OTALocalApPassword() ); webSocket.sendTXT(num, request );
        sprintf( request, "HST\\%s", config_get_HostName() ); webSocket.sendTXT(num, request );
        sprintf( request, "FUP\\%s", config_get_FirmwareURL() ); webSocket.sendTXT(num, request );
        sprintf( request, "SUP\\%s", config_get_SPIFFSURL() ); webSocket.sendTXT(num, request );
        sprintf( request, "MQS\\%s", config_get_MQTTServer() ); webSocket.sendTXT(num, request );
        sprintf( request, "MQU\\%s", config_get_MQTTUser() ); webSocket.sendTXT(num, request );
        sprintf( request, "MQP\\%s", config_get_MQTTPass() ); webSocket.sendTXT(num, request );
        sprintf( request, "MQT\\%s", config_get_MQTTTopic() ); webSocket.sendTXT(num, request );
        sprintf( request, "MQI\\%s", config_get_MQTTInterval() ); webSocket.sendTXT(num, request );
        sprintf( request, "PIN\\%s", config_get_MeasurePin() ); webSocket.sendTXT(num, request );
        sprintf( request, "BUR\\%s", config_get_MeasureBurdenResistor() ); webSocket.sendTXT(num, request );
        sprintf( request, "COI\\%s", config_get_MeasureCoilTurns() ); webSocket.sendTXT(num, request );
        sprintf( request, "VOL\\%s", config_get_MeasureVoltage() ); webSocket.sendTXT(num, request );
        sprintf( request, "OFF\\%s", config_get_MeasureOffset() ); webSocket.sendTXT(num, request );
        sprintf( request, "COU\\%s", config_get_MeasureCounter() ); webSocket.sendTXT(num, request );
        sprintf( request, "COS\\%s", config_get_MeasureCost() ); webSocket.sendTXT(num, request );
        sprintf( request, "CHS\\%s", config_get_MeasureChannels() ); webSocket.sendTXT(num, request );
      }
      /* Wlan SSID */
      else if ( !strcmp("HST", cmd ) ) {
        config_set_HostName( value );
      }
      /* Wlan SSID */
      else if ( !strcmp("STS", cmd ) ) {
        char request[128]="";
        sprintf( request, "status\\online ( %.3fkW / %.3fkWh / %.2f€ / Irms = %.3fA )"  , ( measure_get_power( 0 ) ) / 1000
                                                                                        , ( measure_get_poweroverall( 0 ) ) / 1000
                                                                                        , ( measure_get_poweroverall( 0 ) ) / 1000 * atof( config_get_MeasureCost() )
                                                                                        , measure_get_Irms( 0 ) );
        webSocket.sendTXT(num, request );
      }
      /* Wlan SSID */
      else if ( !strcmp("ASS", cmd ) ) {
        config_set_OTALocalApSSID( value );
      }
      /* Wlan Passwort */
      else if ( !strcmp("APS", cmd ) ) {
        config_set_OTALocalApPassword( value );
      }
      /* Wlan SSID */
      else if ( !strcmp("WSS", cmd ) ) {
        config_set_WlanSSID( value );
      }
      /* Wlan Passwort */
      else if ( !strcmp("WPS", cmd ) ) {
        config_set_WlanPassord( value );
      }
      /* Wlan Passwort */
      else if ( !strcmp("OPP", cmd ) ) {
        config_set_OTAWlanPin( value );
      }
      /* OTA Wlan Password*/
      else if ( !strcmp("FUP", cmd ) ) {
        config_set_FirmwareURL( value );
      }
      /* OTA Wlan Password*/
      else if ( !strcmp("SUP", cmd ) ) {
        config_set_SPIFFSURL( value );
      }
      /* MQTT Server */
      else if ( !strcmp("MQS", cmd ) ) {
        config_set_MQTTServer( value );
      }
      /* MQTT User */
      else if ( !strcmp("MQU", cmd ) ) {
        config_set_MQTTUser( value );
      }
      /* MQTT Pass */
      else if ( !strcmp("MQP", cmd ) ) {
        config_set_MQTTPass( value );
      }
      /* MQTT Topic */
      else if ( !strcmp("MQT", cmd ) ) {
        config_set_MQTTTopic( value );
      }
      /* MQTT Interval */
      else if ( !strcmp("MQI", cmd ) ) {
        config_set_MQTTInterval( value );
      }
      /* MQTT Topc */
      else if ( !strcmp("PIN", cmd ) ) {
        config_set_MeasurePin( value );
      }
      /* MQTT Topc */
      else if ( !strcmp("BUR", cmd ) ) {
        config_set_MeasureBurdenResistor( value );
      }
      /* MQTT Topc */
      else if ( !strcmp("COI", cmd ) ) {
        config_set_MeasureCoilTurns( value );
      }
      /* MQTT Topc */
      else if ( !strcmp("VOL", cmd ) ) {
        config_set_MeasureVoltage( value );
      }
      /* MQTT Topc */
      else if ( !strcmp("OFF", cmd ) ) {
        config_set_MeasureOffset( value );
      }
      /* MQTT Topc */
      else if ( !strcmp("CHS", cmd ) ) {
        config_set_MeasureChannels( value );
      }
      /* MQTT Topc */
      else if ( !strcmp("COS", cmd ) ) {
        config_set_MeasureCost( value );
      }
      /* MQTT Topc */
      else if ( !strcmp("COU", cmd ) ) {
        config_set_MeasureCounter( value );
        measure_set_poweroverall( 0, atof( value ) );
      }
    }
  }
}

/*
 * Websocket-Server einrichten und Eventroutine eintragen
 */
void websock_setup( void ) {
  webSocket.begin();
  webSocket.onEvent(webSocketEvent);
}

/*
 * 
 */
void websock_loop( void ) {
  webSocket.loop();  
}

/*
 * 
 */
void websock_StartTask ( void ) {

  xTaskCreatePinnedToCore(
                            websock_Task,   /* Function to implement the task */
                            "websock Task", /* Name of the task */
                            10000,      /* Stack size in words */
                            NULL,       /* Task input parameter */
                            1,          /* Priority of the task */
                            &_WEBSOCK_Task,       /* Task handle. */
                            _WEBSOCK_TASKCORE );  /* Core where the task should run */  
}

/*
 * 
 */
void websock_Task( void * pvParameters ) {

  Serial.printf("Start Websock Task on Core: %d\r\n", xPortGetCoreID() );

  websock_setup();

  while( true ) {
    vTaskDelay( 10 );
    webSocket.loop();
  }
}
