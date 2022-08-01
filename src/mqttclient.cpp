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
#include <ArduinoJson.h>

#include "measure.h"
#include "mqttclient.h"
#include "config.h"

TaskHandle_t _ASYNCMQTT_Task;
AsyncMqttClient mqttClient;
bool disable_MQTT = false;

#define TERMOSTAT_CMND                  "cmnd/%s/powermeter"
#define TERMOSTAT_CONFIG                "stat/%s/config"
#define TERMOSTAT_DATA                  "stat/%s/data"
#define TERMOSTAT_STAT_POWER            "stat/%s/power"
#define TERMOSTAT_STAT_REALTIMEPOWER    "stat/%s/realtimepower"

char powermeter_cmnd_topic[128] = "";
char powermeter_config_topic[128] = "";
char powermeter_data_topic[128] = "";
char powermeter_stat_power_topic[128] = "";
char powermeter_stat_realtimepower_topic[128] = "";

static float measure_power[ VIRTUAL_CHANNELS ];
static float measure_voltage[ VIRTUAL_CHANNELS ];
static float measure_current[ VIRTUAL_CHANNELS ];

void mqtt_client_Task( void * pvParameters );
void mqtt_client_send_realtimepower( void );
void mqtt_client_send_power( void );

/**
 * @brief mqqt on connect call back routine
 * 
 * @param sessionPresent 
 */
void mqtt_client_on_connect( bool sessionPresent ) {
    log_i( "MQTT-Client: connected to %s\r\n", config_get_MQTTServer() );
    /**
     * setup all mqtt topic
     */
    snprintf( powermeter_cmnd_topic, sizeof( powermeter_cmnd_topic ), TERMOSTAT_CMND, config_get_MQTTTopic() );
    snprintf( powermeter_config_topic, sizeof( powermeter_config_topic ), TERMOSTAT_CONFIG, config_get_MQTTTopic() );
    snprintf( powermeter_data_topic, sizeof( powermeter_data_topic ), TERMOSTAT_DATA, config_get_MQTTTopic() );
    snprintf( powermeter_stat_power_topic, sizeof( powermeter_stat_power_topic ), TERMOSTAT_STAT_POWER, config_get_MQTTTopic() );
    snprintf( powermeter_stat_realtimepower_topic, sizeof( powermeter_stat_realtimepower_topic ), TERMOSTAT_STAT_REALTIMEPOWER, config_get_MQTTTopic() );
    /**
     * subscripe cmnd topic for remote cmnd
     */
    mqttClient.subscribe( powermeter_cmnd_topic, 0 );
}

/**
 * @brief mqtt on disconnect call back routine
 * 
 * @param reason 
 */
void mqtt_client_on_disconnect(AsyncMqttClientDisconnectReason reason) {
    log_i( "MQTT-Client: Disconnected from MQTT\r\n" );
    /**
     * check connection state and reconnect if possible/allowed
     */
    if ( WiFi.isConnected() ) {
        if ( strlen( config_get_MQTTServer() ) >= 1 && disable_MQTT == false ) {
            mqtt_client_enable();
        }
    }
}

/**
 * @brief mqtt on message call back routine
 * 
 * @param topic             mqtt topic
 * @param payload           payload buffer, not zero terminated
 * @param properties        
 * @param len               size of the payload buffer
 * @param index             
 * @param total
 */
void mqtt_client_on_message(char* topic, char* payload, AsyncMqttClientMessageProperties properties, size_t len, size_t index, size_t total) {
    /**
     * copy no zero terminate payload buffer into a terminated buffer
     */    
    char *msg = NULL;
    if( !(msg = (char *)calloc( 1, len + 1 ) ) ) {
        log_e("error while allocate playload buffer");
        while( true );
    }
    memcpy( (void*)msg, payload, len );
    log_d("MQTT-Client: Publish received, topic: [%s] , payload: [%s]", topic, msg );
    /**
     * check if the msg come from cmnd topc
     */
    if( !strcmp( topic, powermeter_cmnd_topic ) ) {
        /**
         * serialize json document
         */
        StaticJsonDocument<1024> doc;
        DeserializationError error = deserializeJson( doc, msg );
        /**
         * if success, check contains keys
         */
        if ( !error) {
            if ( doc.containsKey("BurdenResistor") ) {
                config_set_MeasureBurdenResistor( (char*) doc["BurdenResistor"].as<String>().c_str() );
            }
            if ( doc.containsKey("CoilTurns") ) {
                config_set_MeasureCoilTurns( (char*) doc["CoilTurns"].as<String>().c_str() );
            }
            if ( doc.containsKey("Voltage") ) {
                config_set_MeasureVoltage( (char*) doc["Voltage"].as<String>().c_str() );
            }
            if ( doc.containsKey("Channels") ) {
                config_set_MeasureChannels( (char*) doc["Channels"].as<String>().c_str() );
            }
            if ( doc.containsKey("Samplerate") ) {
                config_set_MeasureSamplerate( (char*) doc["Samplerate"].as<String>().c_str() );
            }
            if ( doc.containsKey("VoltageFrequency") ) {
                config_set_MeasureVoltageFrequency( (char*) doc["VoltageFrequency"].as<String>().c_str() );
            }
            if ( doc.containsKey("CurrentOffset") ) {
                config_set_MeasureCurrentOffset( (char*) doc["CurrentOffset"].as<String>().c_str() );
            }
            if ( doc.containsKey("store") ) {
                if( doc["store"] )
                    config_save();
            }
        }
    }
    /**
     * free msg buffer
     */
    free( msg );
}

void mqtt_client_publish( char * topic, char * payload ) {
    mqttClient.publish( topic , 1, false, payload, strlen( payload ), false, 0 );
}

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

void mqtt_client_Task( void * pvParameters ) {
    /*
     * set the timerevent for sending MQTT and reconnect
     */
    static uint64_t NextMeasureMillis = millis();
    static uint64_t NextMillis = millis() + 15000;
    /**
     * clear measure buffers
     */
    memset( measure_power, 0, sizeof( measure_power ) );
    memset( measure_voltage, 0, sizeof( measure_voltage ) );
    memset( measure_current, 0, sizeof( measure_current ) );
    /**
     * set call back functions
     */
    mqttClient.onConnect( mqtt_client_on_connect );
    mqttClient.onDisconnect( mqtt_client_on_disconnect );
    mqttClient.onMessage( mqtt_client_on_message );
    /**
     * enable mqtt connection
     */
    mqtt_client_enable();

    log_i( "Start MQTT-Client on Core: %d\r\n", xPortGetCoreID() );

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
                /**
                 *  send every second an json msg to MQTT
                 */
                if ( NextMeasureMillis < millis() ) {
                    NextMeasureMillis += 1000l;
                    /*
                     * add measure values to each measure buffer every second
                     */
                    for ( int channel = 0 ; channel < atoi(config_get_MeasureChannels()) ; channel++ ) {
                        measure_power[ channel ] += measure_get_power( channel );
                        measure_voltage[ channel ] += measure_get_Vrms( channel );
                        measure_current[ channel ] += measure_get_Irms( channel );
                    }
                    mqtt_client_send_realtimepower();
                }
                /**
                 *  send every MQTTInterval seconds a json msg to MQTT
                 */
                if ( NextMillis < millis() ) {
                    NextMillis += atol( config_get_MQTTInterval() ) * 1000l;
                    mqtt_client_send_power();
                    /**
                     * clear measure buffers
                     */
                    for ( int channel = 0 ; channel < atoi(config_get_MeasureChannels()) ; channel++ ) {
                        measure_power[ channel ] = 0;
                        measure_voltage[ channel ] = 0;
                        measure_current[ channel ] = 0;
                    }
                }
            }
        }
    }
}

/**
 * @brief send a realtime power json msg over mqtt
 */
void mqtt_client_send_realtimepower( void ) {
    char value[1024] = "";
    char temp[128]="";
    int virtualchannel = 0;
    float powersum = 0;

    if ( atoi(config_get_MeasureChannels()) == 3 ) virtualchannel = 1;

    for( int channel=0 ; channel<atoi(config_get_MeasureChannels()) ; channel++ ) {
        powersum += measure_get_power( channel ) / 1000;
    }

    snprintf( value, sizeof( value ), "{\"id\":\"%s\",\"all\":{\"power\":\"%.3f\",\"PowerUnit\":\"kWs\"},", config_get_HostName(), powersum );

    for ( int channel = 0 ; channel < atoi(config_get_MeasureChannels()) ; channel++ ) {
        if ( channel != 0 )
            strncat( value, ",", sizeof(value) );

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

    mqtt_client_publish( powermeter_stat_realtimepower_topic , value );

    return;
}

/**
 * @brief send a power json msg over mqtt
 */
void mqtt_client_send_power( void ) {
    char value[1024] = "";
    char temp[128] = "";
    float powersum = 0;

    for( int channel=0 ; channel<atoi(config_get_MeasureChannels()) ; channel++ ) {
        powersum += measure_power[ channel ] / atof( config_get_MQTTInterval() );
    }

    snprintf( value, sizeof( value ), "{\"id\":\"%s\",\"all\":{\"power\":\"%.3f\",\"PowerUnit\":\"kWs\"},", config_get_HostName(), powersum );

    for ( int channel = 0 ; channel < atoi(config_get_MeasureChannels()) ; channel++ ) {
        if ( channel != 0 )
            strncat( value, ",", sizeof(value) );

        snprintf( temp, sizeof( temp ), "\"channel%d\":{\"power\":\"%.3f\",\"voltage\":\"%.3f\",\"current\":\"%.3f\",\"frequence\":\"%.3f\"}"   , channel
                                                                                                                                                , measure_power[ channel ] / atof( config_get_MQTTInterval() ) / 1000
                                                                                                                                                , measure_voltage[ channel ] / atof( config_get_MQTTInterval() )
                                                                                                                                                , measure_current[ channel ] / atof( config_get_MQTTInterval() )
                                                                                                                                                , measure_get_max_freq() );
        strncat( value, temp, sizeof(value) );
    }
    
    snprintf( temp, sizeof( temp ), ",\"Interval\":\"%s\"", config_get_MQTTInterval() );
    strncat( value, temp, sizeof(value) );
    strncat( value, ",\"PowerUnit\":\"kWs\", \"VoltageUnit\":\"V\", \"CurrentUnit\":\"A\",\"FrequenceUnit\":\"Hz\"}", sizeof(value) );

    mqtt_client_publish( powermeter_stat_power_topic , value );

    return;
}

void mqtt_client_disable( void ) {
    disable_MQTT = true;
    mqttClient.disconnect();
}

void mqtt_client_enable( void ) {
    disable_MQTT = false;
    /**
     * reload all connection settings
     */
    mqttClient.setServer( config_get_MQTTServer() , 1883 );
    mqttClient.setClientId( config_get_HostName() );
    mqttClient.setCredentials( config_get_MQTTUser(), config_get_MQTTPass() );
    /**
     * start connection
     */
    mqttClient.connect();
}
