/****************************************************************************
              mqtt_client.cpp

    Sa April 27 12:01:00 2019
    Copyright  2019  Dirk Brosswick
    Email: dirk.brosswick@googlemail.com
 ****************************************************************************/

/*
    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
*/

/**

   \author Dirk Broßwick

*/
#include <WiFi.h>
#include <AsyncMqttClient.h>

#include "measure.h"
#include "mqttclient.h"
#include "config.h"

TaskHandle_t _ASYNCMQTT_Task;
AsyncMqttClient mqttClient;
bool disable_MQTT=false;

/*
 *
 */
void onMqttConnect(bool sessionPresent) {
  Serial.printf( "MQTT-Client: connected to %s\r\n", config_get_MQTTServer() );
  char topic[64] = "";
  snprintf( topic, sizeof( topic ), "cmd/%s/#", config_get_MQTTTopic() );
  mqttClient.subscribe( topic, 0 );
  Serial.printf( "MQTT-Client: subscribe [%s]\r\n", topic );
}

/*
 *
 */
void onMqttDisconnect(AsyncMqttClientDisconnectReason reason) {
  Serial.printf( "MQTT-Client: Disconnected from MQTT\r\n" );
  if ( WiFi.isConnected() )
    if ( strlen( config_get_MQTTServer() ) >= 1 && disable_MQTT == false ) {
      mqttClient.connect();
    }
}

/*
 *
 */
void onMqttSubscribe(uint16_t packetId, uint8_t qos) {
  Serial.printf( "MQTT-Client: Subscribe acknowledged [%d]\r\n", packetId );
}

/*
 *
 */
void onMqttUnsubscribe(uint16_t packetId) {
  Serial.printf( "MQTT-Client: Unsubscribe acknowledged [%d]\r\n", packetId );
}

/*
 *
 */
void onMqttMessage(char* topic, char* payload, AsyncMqttClientMessageProperties properties, size_t len, size_t index, size_t total) {
  Serial.printf( "MQTT-Client: Publish received, topic: [%s] , payload: [", topic );
  for ( int i = 0 ; i < len ; i++ ) {
    printf("%c",payload[i]);
  }
  printf("]\r\n");
}

/*
   send an message to MQTT
*/
void mqtt_client_publish( char * topic, char * payload ) {
  mqttClient.publish( topic , 1, false, payload, strlen( payload ), false, 0 );
}

/*
 *
 */
void mqtt_client_StartTask( void ) {
  xTaskCreatePinnedToCore(
                      mqtt_client_Task,    /* Function to implement the task */
                      "mqttclient Task",  /* Name of the task */
                      10000,              /* Stack size in words */
                      NULL,               /* Task input parameter */
                      1,                  /* Priority of the task */
                      &_ASYNCMQTT_Task,         /* Task handle. */
                      _MQTT_TASKCORE );   /* Core where the task should run */
}

/*
 *
 */
void mqtt_client_Task( void * pvParameters ) {

  /*
     set the timerevent for sending MQTT and reconnect
  */
  static uint64_t NextMeasureMillis = millis();
  static uint64_t NextMillis = millis() + 15000;
  static float measure_power[ VIRTUAL_CHANNELS ];
  static float measure_voltage[ VIRTUAL_CHANNELS ];
  static float measure_current[ VIRTUAL_CHANNELS ];

  memset( measure_power, 0, sizeof( measure_power ) );
  memset( measure_voltage, 0, sizeof( measure_voltage ) );
  memset( measure_current, 0, sizeof( measure_current ) );

  mqttClient.onConnect(onMqttConnect);
  mqttClient.onDisconnect(onMqttDisconnect);
  mqttClient.onSubscribe(onMqttSubscribe);
  mqttClient.onUnsubscribe(onMqttUnsubscribe);
  mqttClient.onMessage(onMqttMessage);

  mqttClient.setServer( config_get_MQTTServer() , 1883 );
  mqttClient.setClientId( config_get_HostName() );
  mqttClient.setCredentials( config_get_MQTTUser(), config_get_MQTTPass() );

  Serial.printf( "Start MQTT-Client on Core: %d\r\n", xPortGetCoreID() );

  vTaskDelay( 1000 );

  while( true ) {
    vTaskDelay( 10 );

    if ( WiFi.isConnected() ) {
      if ( !mqttClient.connected() ) {
        if ( strlen( config_get_MQTTServer() ) >= 1 && disable_MQTT == false ) {
          mqttClient.connect();
        }
      }
      else {
        int virtualchannel = 0;

        if ( atoi(config_get_MeasureChannels()) == 3 ) virtualchannel = 1;
        /*
        *  send N seconds an json msg to MQTT
        */
        if ( NextMillis < millis() ) {
          NextMillis += atol( config_get_MQTTInterval() ) * 1000l;
          char value[256] = "";
          char topic[128] = "";
          char temp[128] = "";
          float powersum=0;

          for( int channel=0 ; channel<atoi(config_get_MeasureChannels()) ; channel++ ) {
            powersum += measure_power[ channel ] / atof( config_get_MQTTInterval() );
          }

          snprintf( value, sizeof( value ), "{\"id\":\"%s\",\"all\":{\"power\":\"%.3f\",\"PowerUnit\":\"kWs\"},", config_get_HostName(), powersum );

          for ( int channel = 0 ; channel < atoi(config_get_MeasureChannels()) ; channel++ ) {
            if ( channel != 0 ) {
              strncat( value, ",", sizeof(value) );
            }
            snprintf( temp, sizeof( temp ), "\"channel%d\":{\"power\":\"%.3f\",\"voltage\":\"%.3f\",\"current\":\"%.3f\",\"frequence\":\"%.3f\"}"   , channel
                                                                                                                                                    , measure_power[ channel ] / atof( config_get_MQTTInterval() ) / 1000
                                                                                                                                                    , measure_voltage[ channel ] / atof( config_get_MQTTInterval() )
                                                                                                                                                    , measure_current[ channel ] / atof( config_get_MQTTInterval() )
                                                                                                                                                    , measure_get_max_freq() );
            strncat( value, temp, sizeof(value) );
            measure_power[ channel ] = 0;
            measure_voltage[ channel ] = 0;
            measure_current[ channel ] = 0;
          }
          snprintf( temp, sizeof( temp ), ",\"Interval\":\"%s\"", config_get_MQTTInterval() );
          strncat( value, temp, sizeof(value) );
          strncat( value, ",\"PowerUnit\":\"kWs\", \"VoltageUnit\":\"V\", \"CurrentUnit\":\"A\",\"FrequenceUnit\":\"Hz\"}", sizeof(value) );
          snprintf( topic, sizeof( topic ), "stat/%s/power", config_get_MQTTTopic() );
          mqtt_client_publish( topic , value );
        }

        /*
        *  send every second an json msg to MQTT
        */
        if ( NextMeasureMillis < millis() ) {
          NextMeasureMillis += 1000l;
          char value[1024] = "";
          char topic[128] = "";
          char temp[128]="";

          float powersum=0;

          for( int channel=0 ; channel<atoi(config_get_MeasureChannels()) ; channel++ ) {
            powersum += measure_get_power( channel ) / 1000;
          }

          snprintf( value, sizeof( value ), "{\"id\":\"%s\",\"all\":{\"power\":\"%.3f\",\"PowerUnit\":\"kWs\"},", config_get_HostName(), powersum );

          for ( int channel = 0 ; channel < atoi(config_get_MeasureChannels()) ; channel++ ) {

            measure_power[ channel ] += measure_get_power( channel );
            measure_voltage[ channel ] += measure_get_Vrms( channel );
            measure_current[ channel ] += measure_get_Irms( channel );

            if ( channel != 0 ) {
              strncat( value, ",", sizeof(value) );
            }
            snprintf( temp, sizeof( temp ), "\"channel%d\":{\"power\":\"%.3f\",\"voltage\":\"%.3f\",\"current\":\"%.3f\",\"frequence\":\"%.3f\"}"   , channel
                                                                                                                                                    , ( measure_get_Vrms( channel ) * measure_get_Irms( channel ) ) / 1000
                                                                                                                                                    , measure_get_Vrms( channel )
                                                                                                                                                    , measure_get_Irms( channel )
                                                                                                                                                    , measure_get_max_freq() );
            strncat( value, temp, sizeof(value) );
          }
          if ( virtualchannel != 0 ) {
            snprintf( temp, sizeof( temp ), ",\"channel3\":{\"current\":\"%.3f\"}", measure_get_Irms( atoi(config_get_MeasureChannels()) ) );
            strncat( value, temp, sizeof(value) );
          }
          strncat( value, ",\"PowerUnit\":\"kWs\", \"VoltageUnit\":\"V\", \"CurrentUnit\":\"A\",\"FrequenceUnit\":\"Hz\"}", sizeof(value) );

          snprintf( topic, sizeof( topic ), "stat/%s/realtimepower", config_get_MQTTTopic() );
          mqtt_client_publish( topic , value );
        }
      }
    }
  }
}

/*
 *
 */
void mqtt_client_disable( void ) {
  disable_MQTT = true;
  mqttClient.disconnect();
}

/*
 *
 */
void mqtt_client_enable( void ) {
  disable_MQTT = false;
  mqttClient.connect();
}
