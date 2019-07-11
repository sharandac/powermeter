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

/*
This example uses FreeRTOS softwaretimers as there is no built-in Ticker library
*/
#include <WiFi.h>
#include <AsyncMqttClient.h>

#include "measure.h"
#include "mqttclient.h"
#include "config.h"

TaskHandle_t _ASYNCMQTT_Task;
AsyncMqttClient mqttClient;

/*
 *
 */
void onMqttConnect(bool sessionPresent) {
  Serial.printf( "connected to %s\r\n", config_get_MQTTServer() );
  char topic[64] = "";
  snprintf( topic, sizeof( topic ), "cmd/%s/#", config_get_MQTTTopic() );
  mqttClient.subscribe( topic, 0 );
  Serial.printf( "subscribe [%s]\r\n", topic );
  snprintf( topic, sizeof( topic ), "stat/%s/#", config_get_MQTTTopic() );
  mqttClient.subscribe( topic, 0 );
  Serial.printf( "subscribe [%s]\r\n", topic );
}

/*
 *
 */
void onMqttDisconnect(AsyncMqttClientDisconnectReason reason) {
  Serial.printf( "Disconnected from MQTT\r\n" );
  if ( WiFi.isConnected() )
    mqttClient.connect();
}

/*
 *
 */
void onMqttSubscribe(uint16_t packetId, uint8_t qos) {
  Serial.printf( "Subscribe acknowledged [%d]\r\n", packetId );
}

/*
 *
 */
void onMqttUnsubscribe(uint16_t packetId) {
  Serial.printf( "Unsubscribe acknowledged [%d]\r\n", packetId );
}

/*
 *
 */
void onMqttMessage(char* topic, char* payload, AsyncMqttClientMessageProperties properties, size_t len, size_t index, size_t total) {
  Serial.printf( "Publish received, topic: [%s] , payload: [%s]\r\n", topic, payload);
}

/*
 *
 */
void onMqttPublish(uint16_t packetId) {
  Serial.printf( "Publish acknowledged [%d]\r\n", packetId );
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
  static unsigned long NextMillis = millis() + 15000;

  mqttClient.onConnect(onMqttConnect);
  mqttClient.onDisconnect(onMqttDisconnect);
  mqttClient.onSubscribe(onMqttSubscribe);
  mqttClient.onUnsubscribe(onMqttUnsubscribe);
  mqttClient.onMessage(onMqttMessage);
  mqttClient.onPublish(onMqttPublish);

  mqttClient.setServer( config_get_MQTTServer() , 1883 );
  mqttClient.setClientId( config_get_HostName() );
  mqttClient.setCredentials( config_get_MQTTUser(), config_get_MQTTPass() );

  Serial.printf( "Start on Core: %d\r\n", xPortGetCoreID() );

  delay(1000);

  while( true ) {
    delay(10);

    if ( WiFi.isConnected() ) {
      if ( !mqttClient.connected() ) {
        mqttClient.connect();
      }
      else {
        /*
        *  send N seconds an msg to MQTT
        */
        if ( NextMillis < millis() ) {
          NextMillis += atoi( config_get_MQTTInterval() ) * 1000;
          char value[32] = "";
          char topic[64] = "";

          for ( int channel = 0 ; channel < MEASURE_CHANELS ; channel++ ) {
            snprintf( value, sizeof( value ), "%.3f", measure_get_power( channel ) / 1000 );
            snprintf( topic, sizeof( topic ), "stat/%s/channel%d/power", config_get_MQTTTopic(), channel );
            mqtt_client_publish( topic , value );
            snprintf( value, sizeof( value ), "%.3f", measure_get_poweroverall( channel ) / 1000 );
            snprintf( topic, sizeof( topic ), "stat/%s/channel%d/poweroverall", config_get_MQTTTopic(), channel );
            mqtt_client_publish( topic , value );
          }

          double poweroverall = 0;
          for ( int channel = 0 ; channel < MEASURE_CHANELS ; channel++ ) {
            poweroverall =+ measure_get_poweroverall( channel );
          }
          snprintf( value, sizeof( value ), "%.2f", ( poweroverall / 1000 ) * atof( config_get_MeasureCost() ) );
          snprintf( topic, sizeof( topic ), "stat/%s/cost", config_get_MQTTTopic() );
          mqtt_client_publish( topic , value );      
        }
      }
    }
  }
}
