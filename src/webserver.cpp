/****************************************************************************
 *            AsyncWebserver.cpp
 *
 *  May 23 00:05:23 2019
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
#include <WiFi.h>
#include <WiFiClient.h>
#include <Update.h>
#include <SPIFFS.h>

#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>

#include <SPIFFSEditor.h>

#include "mqttclient.h"
#include "webserver.h"
#include "ota.h"
#include "config.h"
#include "measure.h"

AsyncWebServer asyncserver( WEBSERVERPORT );
AsyncWebSocket ws("/ws");
TaskHandle_t _WEBSERVER_Task;

/*
 * websocket-eventroutine
 */
void onWsEvent(AsyncWebSocket * server, AsyncWebSocketClient * client, AwsEventType type, void * arg, uint8_t *data, size_t len) {
  /*
   * queue the event-type
   */
  switch (type) {
    case WS_EVT_CONNECT: { break; }
    case WS_EVT_ERROR: { break; }
    case WS_EVT_PONG: { break; }
    case WS_EVT_DISCONNECT: { break; }
    case WS_EVT_DATA: {
      /*
       * copy data into an separate allocated buffer and terminate it with \0
       */
      char * cmd = (char*)calloc( len+1, sizeof(uint8_t) );
      for ( int i = 0 ; i < len ; i++ ) {
        cmd[i] = data[i];
      }
      cmd[len]='\0';
      /*
       * separate command and his correspondening value
       */
      char * value = cmd;

      while( *value ) {
        if ( *value == '\\' ) {
          *value = '\0';
          value++;
          break;
        }
        value++;
      }
      
      /*
       * queue commands
       */
      /* store all values into SPIFFFS */
      if ( !strcmp("SAV", cmd ) ) {
        config_saveall();
        client->printf("status\\Save" );
      }
      /* get samplebuffer */
      else if ( !strcmp("OSC", cmd ) ) {
        char stringbuffer[ numbersOfSamples * 6 + 96 ]="";
        char tmp[64]="";
        uint16_t * mybuffer;
        int scale=1;
        int virtualchannel = 0;

        if ( atoi(config_get_MeasureChannels()) > 1 ) scale = 3;
        if ( atoi(config_get_MeasureChannels()) > 2 ) scale = 6;
        if ( atoi(config_get_MeasureChannels()) == MEASURE_CHANELS ) virtualchannel = 1;

        sprintf( tmp,"OScopeProbe\\%d\\%d\\%d\\%f\\", atoi(config_get_MeasureChannels()) + virtualchannel , numbersOfSamples/scale, numbersOfFFTSamples, measure_get_Iratio() );
        strcat( stringbuffer, tmp );

        mybuffer = measure_get_buffer();

        for( int i = 0 ; i < numbersOfSamples * ( atoi(config_get_MeasureChannels()) + virtualchannel ) ; i=i+scale ) {
          sprintf(tmp, "%03x", mybuffer[ i ] > 0x0fff?0x0fff:mybuffer[ i ] );
          strcat( stringbuffer, tmp );
        }

        strcat( stringbuffer, "\\" );
        mybuffer = measure_get_fft();

        for( int i = 0 ; i < numbersOfFFTSamples * ( atoi(config_get_MeasureChannels()) + virtualchannel ) ; i++ ) {
          sprintf(tmp, "%03x", mybuffer[ i ] > 0x0fff?0x0fff:mybuffer[ i ] );
          strcat( stringbuffer, tmp );
        }

        client->text( stringbuffer );
      }
      /* get all store values */
      else if ( !strcmp("STA", cmd ) ) {
        client->printf( "status\\online" );
        client->printf("WSS\\%s", config_get_WlanSSID() );
        client->printf("WPS\\%s", "********" );
        client->printf("OPP\\%s", "********" );
        client->printf("ASS\\%s", config_get_OTALocalApSSID() );
        client->printf("APS\\%s", "********" );
        client->printf("HST\\%s", config_get_HostName() );
        client->printf("MQS\\%s", config_get_MQTTServer() );
        client->printf("MQU\\%s", config_get_MQTTUser() );
        client->printf("MQP\\%s", "********" );
        client->printf("MQT\\%s", config_get_MQTTTopic() );
        client->printf("MQI\\%s", config_get_MQTTInterval() );
        client->printf("BUR\\%s", config_get_MeasureBurdenResistor() );
        client->printf("COI\\%s", config_get_MeasureCoilTurns() );
        client->printf("VOL\\%s", config_get_MeasureVoltage() );
        client->printf("HRZ\\%s", config_get_MeasureVoltageFrequency() );
        client->printf("OFF\\%s", config_get_MeasureOffset() );
        client->printf("CHS\\%s", config_get_MeasureChannels() );
        client->printf("RAT\\%s", config_get_MeasureSamplerate() );
      }
      /* get status-line */
      else if ( !strcmp("STS", cmd ) ) {
        
        char request[256]="";
        char tmp[128]="";
        float powersum=0;

        for( int line=0 ; line<atoi(config_get_MeasureChannels()) ; line++ ) {
          powersum += measure_get_power( line );
        }
        sprintf( request, "status\\online ( Psum = %.3fkW"  , powersum / 1000 );

        for( int line=0 ; line<atoi(config_get_MeasureChannels()) ; line++ ) {
          powersum += measure_get_power( line );
          sprintf( tmp, " / P%d = %.3fkW", line+1, measure_get_power( line ) / 1000 );
          strcat( request, tmp );
        }
        for( int line=0 ; line<atoi(config_get_MeasureChannels()) ; line++ ) {
          sprintf( tmp, " / Irms%d = %.3fA", line+1, measure_get_Irms( line ) );
          strcat( request, tmp );
        }
        if ( atoi(config_get_MeasureChannels()) == MEASURE_CHANELS ) {
          sprintf( tmp, " / Irmsn = %.3fA", measure_get_Irms( MEASURE_CHANELS ) );
          strcat( request, tmp );
        }
        strcat( request, " )");

        client->printf( request );
      }
      /* Wlan SSID */
      else if ( !strcmp("HST", cmd ) ) {
        config_set_HostName( value );
      }
      /* WlanAP SSID */
      else if ( !strcmp("ASS", cmd ) ) {
        config_set_OTALocalApSSID( value );
      }
      /* WlanAP Passwort */
      else if ( !strcmp("APS", cmd ) ) {
        if ( strcmp( "********", value ) )
          config_set_OTALocalApPassword( value );
      }
      /* Wlan SSID */
      else if ( !strcmp("WSS", cmd ) ) {
        config_set_WlanSSID( value );
      }
      /* Wlan Passwort */
      else if ( !strcmp("WPS", cmd ) ) {
        if ( strcmp( "********", value ) )
          config_set_WlanPassord( value );
      }
      /* Wlan Passwort */
      else if ( !strcmp("OPP", cmd ) ) {
        if ( strcmp( "********", value ) )
          config_set_OTAWlanPin( value );
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
        if ( strcmp( "********", value ) )
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
      /* store burden resistor value */
      else if ( !strcmp("BUR", cmd ) ) {
        config_set_MeasureBurdenResistor( value );
      }
      /* store coil turns */
      else if ( !strcmp("COI", cmd ) ) {
        config_set_MeasureCoilTurns( value );
      }
      /* store AC-voltage */
      else if ( !strcmp("VOL", cmd ) ) {
        config_set_MeasureVoltage( value );
      }
      /* store AC-main voltage frequency */
      else if ( !strcmp("HRZ", cmd ) ) {
        config_set_MeasureVoltageFrequency( value );
        int samplerate = atoi( config_get_MeasureSamplerate() );
        measure_set_samplerate( samplerate );
      }
      /* store offset */
      else if ( !strcmp("OFF", cmd ) ) {
        config_set_MeasureOffset( value );
      }
      /* store numbers of channels */
      else if ( !strcmp("CHS", cmd ) ) {
        config_set_MeasureChannels( value );
      }
      /* store smaple-rate */
      else if ( !strcmp("RAT", cmd ) ) {
        config_set_MeasureSamplerate( value );
        measure_set_samplerate( atoi( value ) );
      }
      /* sample-rate +1Hz */
      else if ( !strcmp("FQ+", cmd ) ) {
        char sampleratestring[16]="0";
        int samplerate = atoi( config_get_MeasureSamplerate() );
        samplerate += 1;
        snprintf( sampleratestring, sizeof( sampleratestring ), "%d", samplerate );
        config_set_MeasureSamplerate( sampleratestring );
        measure_set_samplerate( samplerate );
      }
      /* sample-rate -1Hz */
      else if ( !strcmp("FQ-", cmd ) ) {
        char sampleratestring[16]="0";
        int samplerate = atoi( config_get_MeasureSamplerate() );
        samplerate -= 1;
        snprintf( sampleratestring, sizeof( sampleratestring ), "%d", samplerate );
        config_set_MeasureSamplerate( sampleratestring );
        measure_set_samplerate( samplerate );
      }
    free( cmd );
    }
  }
}

/*
 * based on: https://github.com/lbernstone/asyncUpdate/blob/master/AsyncUpdate.ino
 */
void handleUpdate( AsyncWebServerRequest *request, const String& filename, size_t index, uint8_t *data, size_t len, bool final) {
  mqtt_client_disable();

  if (!index){
    /*
     * if filename includes spiffs, update the spiffs partition
     */
    int cmd = (filename.indexOf("spiffs") > 0) ? U_SPIFFS : U_FLASH;
    if (!Update.begin(UPDATE_SIZE_UNKNOWN, cmd)) {
      Update.printError(Serial);
    }
  }

  /*
   * Write Data an type message if fail
   */
  if (Update.write(data, len) != len) {
    Update.printError(Serial);
  }

  /*
   * After write Update restart
   */
  if (final) {
    AsyncWebServerResponse *response = request->beginResponse(302, "text/plain", "Please wait while the switch reboots");
    response->addHeader("Refresh", "20");  
    response->addHeader("Location", "/");
    request->send(response);
    if (!Update.end(true)){
      Update.printError(Serial);
    } else {
      Serial.println("Update complete");
      Serial.flush();
      config_saveall();
      ESP.restart();
    }
  }
  void mqtt_client_enable();
}

/*
 *
 */
void asyncwebserver_setup(void){

  asyncserver.on("/info", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send(200, "text/plain", "Firmwarestand: " __DATE__ " " __TIME__ "\r\nGCC-Version: " __VERSION__ "\r\nVersion: " __FIRMWARE__ "\r\n" );
  });

  asyncserver.addHandler(new SPIFFSEditor(SPIFFS));
  asyncserver.serveStatic("/", SPIFFS, "/").setDefaultFile("index.htm");

  asyncserver.onNotFound([](AsyncWebServerRequest *request){
    Serial.printf( "NOT_FOUND: ");
    if(request->method() == HTTP_GET)
      Serial.printf( "GET");
    else if(request->method() == HTTP_POST)
      Serial.printf( "POST");
    else if(request->method() == HTTP_DELETE)
      Serial.printf( "DELETE");
    else if(request->method() == HTTP_PUT)
      Serial.printf( "PUT");
    else if(request->method() == HTTP_PATCH)
      Serial.printf( "PATCH");
    else if(request->method() == HTTP_HEAD)
      Serial.printf( "HEAD");
    else if(request->method() == HTTP_OPTIONS)
      Serial.printf( "OPTIONS");
    else
      Serial.printf( "UNKNOWN");
    Serial.printf( " http://%s%s\n", request->host().c_str(), request->url().c_str());

    if(request->contentLength()){
      Serial.printf( "_CONTENT_TYPE: %s\n", request->contentType().c_str());
      Serial.printf( "_CONTENT_LENGTH: %u\n", request->contentLength());
    }

    int headers = request->headers();
    int i;
    for(i=0;i<headers;i++){
      AsyncWebHeader* h = request->getHeader(i);
      Serial.printf( "_HEADER[%s]: %s\n", h->name().c_str(), h->value().c_str());
    }

    int params = request->params();
    for(i=0;i<params;i++){
      AsyncWebParameter* p = request->getParam(i);
      if(p->isFile()){
        Serial.printf( "_FILE[%s]: %s, size: %u\n", p->name().c_str(), p->value().c_str(), p->size());
      } else if(p->isPost()){
        Serial.printf( "_POST[%s]: %s\n", p->name().c_str(), p->value().c_str());
      } else {
        Serial.printf( "_GET[%s]: %s\n", p->name().c_str(), p->value().c_str());
      }
    }
    request->send(404);
  });

  asyncserver.onFileUpload([](AsyncWebServerRequest *request, const String& filename, size_t index, uint8_t *data, size_t len, bool final){
    if(!index)
      Serial.printf( "UploadStart: %s\n", filename.c_str());
    Serial.printf("%s", (const char*)data);
    if(final)
      Serial.printf( "UploadEnd: %s (%u)\n", filename.c_str(), index+len);
  });

  asyncserver.onRequestBody([](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total){
    if(!index) {
      Serial.printf( "BodyStart: %u\n", total);
    }
    Serial.printf( "%s", (const char*)data);
    if(index + len == total) {
      Serial.printf( "BodyEnd: %u\n", total);
    }
  });

  asyncserver.on("/reset", HTTP_GET, []( AsyncWebServerRequest * request ) {
    request->send(200, "text/plain", "Reset\r\n" );
    config_saveall();
    delay(3000);
    ESP.restart();    
  });

  asyncserver.on("/update", HTTP_POST,
    [](AsyncWebServerRequest *request) {},
    [](AsyncWebServerRequest *request, const String& filename, size_t index, uint8_t *data, size_t len, bool final) { handleUpdate(request, filename, index, data, len, final); }
  );

  ws.onEvent(onWsEvent);
  asyncserver.addHandler(&ws);
  asyncserver.begin();
}

/*
 * 
 */
void asyncwebserver_StartTask ( void ) {

  xTaskCreatePinnedToCore(
                            asyncwebserver_Task,    /* Function to implement the task */
                            "webserver Task",       /* Name of the task */
                            10000,                  /* Stack size in words */
                            NULL,                   /* Task input parameter */
                            1,                      /* Priority of the task */
                            &_WEBSERVER_Task,       /* Task handle. */
                            _WEBSERVER_TASKCORE );  /* Core where the task should run */  
}

/*
 * 
 */
void asyncwebserver_Task( void * pvParameters ) {

  Serial.printf( "Start Webserver on Core: %d\r\n", xPortGetCoreID() );

  asyncwebserver_setup();

  while( true ) {
    vTaskDelay( 10 );
    ws.cleanupClients(); 
  }
}