/****************************************************************************
 *            webserver.cpp
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
#include <WiFi.h>
#include <WiFiClient.h>
#include <Update.h>
#include <SPIFFS.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <SPIFFSEditor.h>
#include "config.h"
#include "mqttclient.h"
#include "webserver.h"
#include "measure.h"
#include "display.h"
#include "ioport.h"
#include "wificlient.h"
#include "config/ioport_config.h"
#include "config/display_config.h"
#include "config/mqtt_config.h"
#include "config/wifi_config.h"

AsyncWebServer asyncserver( WEBSERVERPORT );
AsyncWebSocket ws("/ws");
TaskHandle_t _WEBSERVER_Task;

    static const char* serverIndex =
        "<!DOCTYPE html>\n <html><head>\n <script src='https://ajax.googleapis.com/ajax/libs/jquery/3.2.1/jquery.min.js'></script>"
        "\n <script src='/jquery.min.js'></script>"

        "\n <style>"
        "\n #progressbarfull {"
        "\n background-color: #20201F;"
        "\n border-radius: 20px;"
        "\n width: 320px;"
        "\n padding: 4px;"
        "\n}"
        "\n #progressbar {"
        "\n background-color: #20CC00;"
        "\n width: 3%;"
        "\n height: 16px;"
        "\n border-radius: 10px;"
        "\n}"
        "\n</style>"
        "\n </head><body>"
        "<h2>Update by Browser</h2>"
        "\n <form method='POST' action='#' enctype='multipart/form-data' id='upload_form'>"
        "\n <input type='file' name='update'>"
        "\n <br><br><input type='submit' value='Update'>"
        "\n </form>"
        "\n <div id='prg'>Progress: 0%</div>"
        "\n <div id=\"progressbarfull\"><div id=\"progressbar\"></div></div>"
        "\n <script>"
        "\n $('form').submit(function(e){"
        "\n e.preventDefault();"
        "\n var form = $('#upload_form')[0];"
        "\n var data = new FormData(form);"
        "\n $.ajax({"
        "\n url: '/update',"
        "\n type: 'POST',"
        "\n data: data,"
        "\n contentType: false,"
        "\n processData:false,"
        "\n xhr: function() {"
        "\n var xhr = new window.XMLHttpRequest();"
        "\n xhr.upload.addEventListener('progress', function(evt) {"
        "\n if (evt.lengthComputable) {"
        "\n var per = evt.loaded / evt.total;"
        "\n document.getElementById(\"prg\").innerHTML = 'Progress: ' + Math.round(per*100) + '%';"
        "\n document.getElementById(\"progressbar\").style.width=Math.round(per*100)+ '%';"
        "}"
        "}, false);"
        "return xhr;"
        "},"
        "\n success:function(d, s) {"
        "\n document.getElementById(\"prg\").innerHTML = 'Progress: success';"
        "\n console.log('success!')"
        "},"
        "\n error: function (a, b, c) {"
        "\n document.getElementById(\"prg\").innerHTML = 'Progress: error';"
        "}"
        "});"
        "});"
        "\n </script>"
        "\n </body></html>";

void asyncwebserver_Task( void * pvParameters );
/*
 * websocket-eventroutine
 */
void onWsEvent(AsyncWebSocket * server, AsyncWebSocketClient * client, AwsEventType type, void * arg, uint8_t *data, size_t len) {
    static int selectedchannel = 0;
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
        char *cmd = (char*)calloc( len+1, sizeof(uint8_t) );
        memcpy( cmd, data, len );
        /*
         * separate command and his correspondening value
         */
        char *value = cmd;
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
            display_save_settings();
            ioport_save_settings();
            measure_save_settings();
            mqtt_save_settings();
            wificlient_save_settings();
            client->printf("status\\Save" );
        }
        /**
         * send channel name list
         */
        if ( !strcmp("get_channel_list", cmd ) ) {
            for( int i = 0 ; i < VIRTUAL_CHANNELS ; i++ ) {
                client->printf("get_channel_list\\channel_%d_name\\%s", i, measure_get_channel_name( i ) );
                client->printf("get_channel_use_list\\channel%d\\%s\\channel\\%d\\%s", i, ( measure_get_channel_type( i ) != CHANNEL_NOT_USED ) && measure_get_group_active( measure_get_channel_group_id( i ) ) ? "true" : "false", i, measure_get_channel_name( i ) );
            }
        }
        else if ( !strcmp("get_channel_config", cmd ) ) {
            char tmp[64]="";
            if( selectedchannel >= VIRTUAL_CHANNELS )
                selectedchannel = 0;
            client->printf("channel\\%d", selectedchannel );
            client->printf("channel_type\\%01x", measure_get_channel_type( selectedchannel ) );
            client->printf("channel_report_exp\\%d", measure_get_channel_report_exp( selectedchannel ) );
            client->printf("channel_phaseshift\\%d", measure_get_channel_phaseshift( selectedchannel ) );
            client->printf("channel_opcodeseq_str\\%s", measure_get_channel_opcodeseq_str( selectedchannel, sizeof( tmp ), tmp ) );
            client->printf("old_channel_opcodeseq_str\\%s", measure_get_channel_opcodeseq_str( selectedchannel, sizeof( tmp ), tmp ) );
            client->printf("channel_offset\\%f", measure_get_channel_offset( selectedchannel ) );
            client->printf("channel_ratio\\%f", measure_get_channel_ratio( selectedchannel ) );
            client->printf("channel_name\\%s", measure_get_channel_name( selectedchannel ) );
            client->printf("channel_group_id\\%d", measure_get_channel_group_id( selectedchannel ) );
            for( int i = 0 ; i < VIRTUAL_CHANNELS ; i++ ) {
                if( strlen( measure_get_channel_name( i ) ) )
                    client->printf("option\\channel\\%d\\%s", i, measure_get_channel_name( i ) );                    
            }
            for( int i = 0 ; i < MAX_GROUPS ; i++ ) {
                if( strlen( measure_get_group_name( i ) ) )
                    client->printf("option\\channel_group_id\\%d\\%s", i, measure_get_group_name( i ) );                    
            }

        }
        else if ( !strcmp("get_wlan_settings", cmd ) ) {
            client->printf("ssid\\%s", wificlient_get_ssid() );
            client->printf("password\\%s", "********" );
            client->printf("checkbox\\enable_softap\\%s", wificlient_get_enable_softap() ? "true" : "false " );
            client->printf("softap_ssid\\%s", wificlient_get_softap_ssid() );
            client->printf("softap_password\\%s", "********" );
            client->printf("checkbox\\low_power\\%s", wificlient_get_low_power() ? "true" : "false ");
            client->printf("checkbox\\low_bandwidth\\%s", wificlient_get_low_bandwidth() ? "true" : "false ");
        }
        else if ( !strcmp("get_mqtt_settings", cmd ) ) {
            client->printf("mqtt_server\\%s", mqtt_client_get_server() );
            client->printf("mqtt_port\\%d", mqtt_client_get_port() );
            client->printf("mqtt_username\\%s", mqtt_client_get_username() );
            client->printf("mqtt_password\\%s", "********" );
            client->printf("mqtt_topic\\%s", mqtt_client_get_topic() );
            client->printf("mqtt_interval\\%d", mqtt_client_get_interval() );
            client->printf("checkbox\\mqtt_realtimestats\\%s", mqtt_client_get_realtimestats()? "true" : "false" );
        }
        else if ( !strcmp("get_measurement_settings", cmd ) ) {
            client->printf("network_frequency\\%f", measure_get_network_frequency() );
            client->printf("samplerate_corr\\%d", measure_get_samplerate_corr() );
        }
        else if ( !strcmp("get_hostname_settings", cmd ) ) {
            client->printf("hostname\\%s", wificlient_get_hostname() );
        }
        else if ( !strcmp("get_display_settings", cmd ) ) {
            if( selectedchannel >= DISPLAY_MAX_INFO )
                selectedchannel = 0;
            client->printf("channel\\%d", selectedchannel );
            client->printf("display_active\\%s", display_get_active() ? "1" : "0" );
            client->printf("display_sda\\%d", display_get_sda_pin() );
            client->printf("display_sck\\%d", display_get_sck_pin() );
            client->printf("display_flip\\%d", display_get_flip() ? 1 : 0 );
            client->printf("display_refresh_interval\\%d", display_get_refresh_interval() );
            client->printf("display_name\\%s", display_get_infotext_name( selectedchannel ) );
            client->printf("display_x\\%d", display_get_infotext_x( selectedchannel ) );
            client->printf("display_y\\%d", display_get_infotext_y( selectedchannel ) );
            client->printf("display_fontsize\\%d", display_get_infotext_fontsize( selectedchannel ) );
            client->printf("display_value_channel\\%d", display_get_infotext_value_channel( selectedchannel ) );
            client->printf("display_text\\%s", display_get_infotext_text( selectedchannel ) );
            for( int i = 0 ; i < DISPLAY_MAX_INFO ; i++ )
                if( strlen( display_get_infotext_name( i ) ) )
                    client->printf("option\\channel\\%d\\%s", i, display_get_infotext_name( i ) );
            for( int i = 0 ; i < VIRTUAL_CHANNELS ; i++ )
                if( strlen( measure_get_channel_name( i ) ) )
                    client->printf("option\\display_value_channel\\%d\\%s", i, measure_get_channel_name( i ) );
        }

        else if ( !strcmp("get_ioport_settings", cmd ) ) {
            if( selectedchannel >= IOPORT_MAX )
                selectedchannel = 0;
            client->printf("channel\\%d", selectedchannel );
            client->printf("ioport_name\\%s", ioport_get_name( selectedchannel ) );
            client->printf("ioport_start_state\\%d", ioport_get_start_state( selectedchannel ) ? 1 : 0 );
            client->printf("ioport_active\\%d", ioport_get_active( selectedchannel ) ? 1 : 0 );
            client->printf("ioport_gpio_pin_num\\%d", ioport_get_gpio_pin_num( selectedchannel ) );
            client->printf("ioport_invert\\%d", ioport_get_invert( selectedchannel ) );
            client->printf("ioport_value_channel\\%d", ioport_get_value_channel( selectedchannel ) );
            client->printf("ioport_trigger\\%d", ioport_get_trigger( selectedchannel ) );
            client->printf("ioport_trigger_value\\%f", ioport_get_value( selectedchannel ) );
            for( int i = 0 ; i < IOPORT_MAX ; i++ ) {
                if( strlen( ioport_get_name( i ) ) )
                    client->printf("option\\channel\\%d\\%s", i, ioport_get_name( i ) );
            }
            for( int i = 0 ; i < VIRTUAL_CHANNELS ; i++ ) {
                if( strlen( measure_get_channel_name( i ) ) )
                    client->printf("option\\ioport_value_channel\\%d\\%s", i, measure_get_channel_name( i ) );
            }
        }
        else if ( !strcmp("get_group_settings", cmd ) ) {
            if( selectedchannel >= MAX_GROUPS )
                selectedchannel = 0;
            client->printf("channel\\%d", selectedchannel );
            client->printf("group_name\\%s", measure_get_group_name( selectedchannel ) );
            client->printf("group_active\\%d", measure_get_group_active( selectedchannel ) ? 1 : 0 );
            for( int i = 0 ; i < MAX_GROUPS ; i++ ) {
                if( strlen( measure_get_group_name( i ) ) )
                    client->printf("option\\channel\\%d\\%s", i, measure_get_group_name( i ) );
            }
            for( int i = 0 ; i < VIRTUAL_CHANNELS ; i++ )
                client->printf("get_group_use_list\\channel%d_%d\\true\\channel_%d_name\\%s", i, measure_get_channel_group_id( i ), i, measure_get_channel_name( i ) );
            for( int i = 0 ; i < MAX_GROUPS ; i++ )
                client->printf("label\\group_%d_name\\%s", i, measure_get_group_name( i ) );
        }
        else if ( !strcmp("channel_group", cmd ) ) {
            char * group = value;
            int channel_count = 0;
            /**
             * count active channels
             */
            while( *group ) {
                if( *group >= '0' && *group <= '5' ) {
                    int group_id = *group - '0';
                    measure_set_channel_group_id( channel_count, group_id );
                }
                channel_count++;
                group++;
            }       
        }
        /* get samplebuffer */
        else if ( !strcmp("OSC", cmd ) ) {
            char request[ numbersOfSamples * VIRTUAL_CHANNELS + numbersOfFFTSamples * VIRTUAL_CHANNELS + 96 ]="";
            char tmp[64]="";
            uint16_t * mybuffer;
            char * active_channels = value;
            int active_channel_count = 0;
            int SampleScale=4;
            int FFTScale=1;
            /**
             * count active channels
             */
            while( *active_channels ) {
                if( *active_channels == '1' )
                    active_channel_count++;
                active_channels++;
            }
            if( active_channel_count < 8 )
                SampleScale = 2;
            if( active_channel_count < 5 )
                SampleScale = 1;
            /**
             * send first info data
             */
            snprintf( tmp, sizeof( tmp ), "OScopeProbe\\%d\\%d\\%d\\%f\\", active_channel_count , numbersOfSamples / SampleScale, numbersOfFFTSamples / FFTScale, 0.01 );
            strncat( request, tmp, sizeof( request ) );
            /**
             * get sample buffers and build first data block
             */
            mybuffer = measure_get_buffer();
            for( int channel = 0 ; channel < VIRTUAL_CHANNELS ; channel++ ) {
                if( *( value + channel ) == '0' )
                    continue;

                for( int i = 0 ; i < numbersOfSamples ; i = i + SampleScale ) {                    
                    snprintf( tmp, sizeof( tmp ), "%03x", mybuffer[ numbersOfSamples * channel + i ] > 0x0fff ? 0x0fff : mybuffer[ numbersOfSamples * channel + i ] );
                    strncat( request, tmp, sizeof( request ) );
                }
            }
            strncat( request, "\\", sizeof( request ) );
            mybuffer = measure_get_fft();
            /**
             * get fft buffers and build next data block
             */
            for( int channel = 0 ; channel < VIRTUAL_CHANNELS ; channel++ ) {
                if( *( value + channel ) == '0' )
                    continue;

                for( int i = 0 ; i < numbersOfFFTSamples ; i = i + FFTScale ) {
                    snprintf( tmp, sizeof( tmp ), "%03x", mybuffer[ numbersOfFFTSamples * channel + i ] > 0x0fff ? 0x0fff : mybuffer[ numbersOfFFTSamples * channel + i ] );
                    strncat( request, tmp, sizeof( request ) );
                }
            }
            strncat( request, "\\", sizeof( request ) );
            /**
             * get channel data types and build data data block
             */
            for( int channel = 0 ; channel < VIRTUAL_CHANNELS ; channel++ ) {
                if( *( value + channel ) == '0' )
                    continue;

                snprintf( tmp, sizeof( tmp ), "%1x", measure_get_channel_type( channel ) );
                strncat( request, tmp, sizeof( request ) );
            }
            client->text( request );
        }
        /* get status-line */
        else if ( !strcmp("STS", cmd ) ) {
            
            char request[1024]="";
            char tmp[128]="";

            snprintf( request, sizeof( request ), "status\\online (" );
            
            for( int group_id = 0 ; group_id < MAX_GROUPS ; group_id++ ) {
                int power_channel = -1;
                int reactive_power_channel = -1;
                float cos_phi = 1.0f;

                if( !measure_get_group_active( group_id ) )
                    continue;

                if( !measure_get_channel_group_id_entrys( group_id ) )
                    continue;

                snprintf( tmp, sizeof( tmp ), " %s:[ ", measure_get_group_name( group_id ) );
                strncat( request, tmp, sizeof( request ) );

                for( int channel = 0 ; channel < VIRTUAL_CHANNELS ; channel++ ) {
                    if( measure_get_channel_group_id( channel ) == group_id ) {

                        if( measure_get_channel_type( channel ) == AC_POWER )
                            power_channel = channel;
                        if( measure_get_channel_type( channel ) == AC_REACTIVE_POWER )
                            reactive_power_channel = channel;

                        if( power_channel != -1 && reactive_power_channel != -1 ) {
                            float active_power = measure_get_channel_rms( power_channel ) + measure_get_channel_rms( reactive_power_channel );
                            cos_phi = active_power / measure_get_channel_rms( power_channel );
                        }
                    
                        switch( measure_get_channel_type( channel ) ) {
                            case AC_VOLTAGE:
                            case DC_VOLTAGE:
                                snprintf( tmp, sizeof( tmp ), "U=%.3f%s ", measure_get_channel_rms( channel ), measure_get_channel_report_unit( channel ) );
                                break;
                            case AC_CURRENT:
                            case DC_CURRENT:
                                snprintf( tmp, sizeof( tmp ), "I=%.3f%s ", measure_get_channel_rms( channel ), measure_get_channel_report_unit( channel ) );
                                break;
                            case AC_POWER:
                            case DC_POWER:
                                snprintf( tmp, sizeof( tmp ), "P=%.3f%s ", measure_get_channel_rms( channel ), measure_get_channel_report_unit( channel ) );
                                break;
                            case AC_REACTIVE_POWER:
                                snprintf( tmp, sizeof( tmp ), "Pvar=%.3f%s ", measure_get_channel_rms( channel ), measure_get_channel_report_unit( channel ) );
                                break;
                            default:
                                tmp[ 0 ] = '\0';
                        }
                        strncat( request, tmp, sizeof( request ) );
                    }
                }
                if( power_channel != -1 && reactive_power_channel != -1 ) {
                    snprintf( tmp, sizeof( tmp ), "Cos=%.3f ",cos_phi );
                    strncat( request, tmp, sizeof( request ) );
                }
                snprintf( tmp, sizeof( tmp ), "]" );
                strncat( request, tmp, sizeof( request ) );
            }
            snprintf( tmp, sizeof( tmp ), " ; f = %.3fHz", measure_get_max_freq() );
            strncat( request, tmp, sizeof( request ) );
            strncat( request, " )", sizeof( request ));

            client->printf( request );
        }
        /* Wlan SSID */
        else if ( !strcmp("hostname", cmd ) ) {
            wificlient_set_hostname( value );
        }
        /* WlanAP SSID */
        else if ( !strcmp("softap_ssid", cmd ) ) {
            wificlient_set_softap_ssid( value );
        }
        /* WlanAP Passwort */
        else if ( !strcmp("softap_password", cmd ) ) {
            if ( strcmp( "********", value ) )
            wificlient_set_softap_password( value );
        }
        /* Wlan SSID */
        else if ( !strcmp("ssid", cmd ) ) {
            wificlient_set_ssid( value );
        }
        /* Wlan Passwort */
        else if ( !strcmp("password", cmd ) ) {
            if ( strcmp( "********", value ) )
            wificlient_set_password( value );
        }
        /* Wlan Passwort */
        else if ( !strcmp("enable_softap", cmd ) ) {
            wificlient_set_enable_softap( atoi( value ) ? true : false );
        }
        /* MQTT Server */
        else if ( !strcmp("low_power", cmd ) ) {
            wificlient_set_low_power( atoi( value ) ? true : false );
        }
        /* MQTT Server */
        else if ( !strcmp("low_bandwidth", cmd ) ) {
            wificlient_set_low_bandwidth( atoi( value ) ? true : false );
        }
        /* MQTT Server */
        else if ( !strcmp("mqtt_server", cmd ) ) {
            mqtt_client_set_server( value );
        }
        /* MQTT Interval */
        else if ( !strcmp("mqtt_port", cmd ) ) {
            mqtt_client_set_port( atoi( value ) );
        }
        /* MQTT User */
        else if ( !strcmp("mqtt_username", cmd ) ) {
            mqtt_client_set_username( value );
        }
        /* MQTT Pass */
        else if ( !strcmp("mqtt_password", cmd ) ) {
            if ( strcmp( "********", value ) )
                mqtt_client_set_password( value );
        }
        /* MQTT Topic */
        else if ( !strcmp("mqtt_topic", cmd ) ) {
            mqtt_client_set_topic( value );
        }
        /* MQTT Interval */
        else if ( !strcmp("mqtt_interval", cmd ) ) {
            mqtt_client_set_interval( atoi( value ) );
        }
        /* MQTT Interval */
        else if ( !strcmp("mqtt_realtimestats", cmd ) ) {
            mqtt_client_set_realtimestats( atoi( value ) ? true : false );
        }
        /* store AC-main voltage frequency */
        else if ( !strcmp("samplerate_corr", cmd ) ) {
            measure_set_samplerate_corr( atoi( value ) );
        }
        /* store smaple-rate */
        else if ( !strcmp("network_frequency", cmd ) ) {
            measure_set_network_frequency( atof( value ) );
        }
        /* sample-rate +1Hz */
        else if ( !strcmp("FQ+", cmd ) ) {
            measure_set_samplerate_corr( measure_get_samplerate_corr() + 1 );
        }
        /* sample-rate -1Hz */
        else if ( !strcmp("FQ-", cmd ) ) {
            measure_set_samplerate_corr( measure_get_samplerate_corr() - 1 );
        }
        else if ( !strcmp("PS+", cmd ) )
            measure_set_channel_phaseshift( selectedchannel, measure_get_channel_phaseshift( selectedchannel ) + 1 );
        else if ( !strcmp("PS-", cmd ) )
            measure_set_channel_phaseshift( selectedchannel, measure_get_channel_phaseshift( selectedchannel ) - 1 );
        /**
         * channel group
         */
        else if ( !strcmp("channel", cmd ) )
            selectedchannel = atoi( value );
        else if ( !strcmp("channel_type", cmd ) )
            measure_set_channel_type( selectedchannel , atoi( value ) );
        else if ( !strcmp("channel_report_exp", cmd ) )
            measure_set_channel_report_exp( selectedchannel , atoi( value ) );
        else if ( !strcmp("channel_phaseshift", cmd ) )
            measure_set_channel_phaseshift( selectedchannel , atoi( value ) );
        else if ( !strcmp("channel_opcodeseq_str", cmd ) )
            measure_set_channel_opcodeseq_str( selectedchannel ,value );
        else if ( !strcmp("channel_offset", cmd ) )
            measure_set_channel_offset( selectedchannel , atof( value ) );
        else if ( !strcmp("channel_ratio", cmd ) )
            measure_set_channel_ratio( selectedchannel , atof( value ) );
        else if ( !strcmp("channel_name", cmd ) )
            measure_set_channel_name( selectedchannel , value );
        else if ( !strcmp("channel_group_id", cmd ) )
            measure_set_channel_group_id( selectedchannel , atoi( value ) );
        /**
         * groups
         */
        else if ( !strcmp("group_name", cmd ) )
            measure_set_group_name( selectedchannel,  value );
        else if ( !strcmp("group_active", cmd ) )
            measure_set_group_active( selectedchannel, atoi( value ) ? true : false );
        /**
         * display group
         */
        else if ( !strcmp("display_active", cmd ) )
            display_set_active( atoi( value ) ? true : false );
        else if ( !strcmp("display_sda", cmd ) )
            display_set_sda_pin( atoi( value ) );
        else if ( !strcmp("display_sck", cmd ) )
            display_set_sck_pin( atoi( value ) );
        else if ( !strcmp("display_flip", cmd ) )
            display_set_flip( atoi( value ) ? true : false );
        else if ( !strcmp("display_refresh_interval", cmd ) )
            display_set_refresh_interval( atoi( value ) );
        else if ( !strcmp("display_name", cmd ) )
            display_set_infotext_name( selectedchannel, value );
        else if ( !strcmp("display_x", cmd ) )
            display_set_infotext_x( selectedchannel, atoi( value ) );
        else if ( !strcmp("display_y", cmd ) )
            display_set_infotext_y( selectedchannel, atoi( value ) );
        else if ( !strcmp("display_fontsize", cmd ) )
            display_set_infotext_fontsize( selectedchannel, atoi( value ) );
        else if ( !strcmp("display_value_channel", cmd ) )
            display_set_infotext_value_channel( selectedchannel, atoi( value ) );
        else if ( !strcmp("display_text", cmd ) )
            display_set_infotext_text( selectedchannel, value );
        /**
         * ioport group
         */
        else if ( !strcmp("ioport_name", cmd ) )
            ioport_set_name( selectedchannel, value );
        else if ( !strcmp("ioport_start_state", cmd ) )
            ioport_set_start_state( selectedchannel, atoi( value ) );
        else if ( !strcmp("ioport_active", cmd ) )
            ioport_set_active( selectedchannel, atoi( value ) );
        else if ( !strcmp("ioport_gpio_pin_num", cmd ) )
            ioport_set_gpio_pin_num( selectedchannel, atoi( value ) );
        else if ( !strcmp("ioport_gpio_pin_num", cmd ) )
            ioport_set_gpio_pin_num( selectedchannel, atoi( value ) );
        else if ( !strcmp("ioport_invert", cmd ) )
            ioport_set_invert( selectedchannel, atoi( value ) );
        else if ( !strcmp("ioport_value_channel", cmd ) )
            ioport_set_value_channel( selectedchannel, atoi( value ) );
        else if ( !strcmp("ioport_trigger", cmd ) )
            ioport_set_trigger( selectedchannel, atoi( value ) );
        else if ( !strcmp("ioport_trigger_value", cmd ) )
            ioport_set_value( selectedchannel, atof( value ) );

        free( cmd );
    }
  }
}

/*
 * based on: https://github.com/lbernstone/asyncUpdate/blob/master/AsyncUpdate.ino
 */
void handleUpdate( AsyncWebServerRequest *request, const String& filename, size_t index, uint8_t *data, size_t len, bool final) {
    mqtt_client_disable();

    if ( !index ) {
        /*
         * if filename includes spiffs, update the spiffs partition
         */
        int cmd = ( filename.indexOf("spiffs") > 0 ) ? U_SPIFFS : U_FLASH;
        if (!Update.begin( UPDATE_SIZE_UNKNOWN, cmd ) )
            Update.printError(Serial);
    }
    /*
     * Write Data an type message if fail
     */
    if ( Update.write( data, len ) != len )
        Update.printError( Serial );
    /*
     * After write Update restart
     */
    if (final) {
        AsyncWebServerResponse *response = request->beginResponse( 302, "text/plain", "Please wait while the powermeter reboots" );
        response->addHeader( "Refresh", "20" );  
        response->addHeader( "Location", "/" );
        request->send(response);

        if( !Update.end( true ) )
            Update.printError(Serial);
        else {
            log_i( "Update complete" );
            display_save_settings();
            ioport_save_settings();
            measure_save_settings();
            mqtt_save_settings();
            wificlient_save_settings();
            ESP.restart();
        }
    }
    void mqtt_client_enable();
}

void asyncwebserver_setup(void){
    asyncserver.on("/info", HTTP_GET, [](AsyncWebServerRequest * request) { request->send(200, "text/plain", "Firmwarestand: " __DATE__ " " __TIME__ "\r\nGCC-Version: " __VERSION__ "\r\nVersion: " __FIRMWARE__ "\r\n" ); } );
    asyncserver.addHandler( new SPIFFSEditor( SPIFFS ) );
    asyncserver.serveStatic( "/", SPIFFS, "/" ).setDefaultFile( "index.htm" );
    asyncserver.onNotFound([](AsyncWebServerRequest *request) { request->send(404); } );
    asyncserver.onFileUpload([](AsyncWebServerRequest *request, const String& filename, size_t index, uint8_t *data, size_t len, bool final){
        if( !index )
            log_i( "UploadStart: %s", filename.c_str());

        log_i("%s", (const char*)data );
        if( final )
            log_i( "UploadEnd: %s (%u)", filename.c_str(), index + len );
    });
    asyncserver.onRequestBody([](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total){
        if( !index )
        log_i( "BodyStart: %u", total );

        log_i( "%s", (const char*)data );
        if( index + len == total )
        log_i( "BodyEnd: %u\n", total);
    });
    asyncserver.on( "/default", HTTP_GET, []( AsyncWebServerRequest * request ) {
        AsyncWebServerResponse *response = request->beginResponse( 302, "text/plain", "Reset to default. Please wait while the powermeter reboots\r\n" );
        response->addHeader( "Refresh", "20" );  
        response->addHeader( "Location", "/" );
        request->send(response);
        SPIFFS.remove( "powermeter.json" );
        SPIFFS.remove( "display.json" );
        SPIFFS.remove( "ioport.json" );
        SPIFFS.remove( "measure.json" );
        delay(3000);
        ESP.restart();    
    });
    asyncserver.on("/memory", HTTP_GET, [](AsyncWebServerRequest *request) {
        String html = (String) "<html><head><meta charset=\"utf-8\"></head><body><h3>Memory Details</h3>" +
                    "<b>Heap size: </b>" + ESP.getHeapSize() + "<br>" +
                    "<b>Heap free: </b>" + ESP.getFreeHeap() + "<br>" +
                    "<b>Heap free min: </b>" + ESP.getMinFreeHeap() + "<br>" +
                    "<b>Psram size: </b>" + ESP.getPsramSize() + "<br>" +
                    "<b>Psram free: </b>" + ESP.getFreePsram() + "<br>" +

                    "<br><b><u>System</u></b><br>" +
                    "<b>Uptime: </b>" + millis() / 1000 + "<br>" +
                    "</body></html>";
        request->send(200, "text/html", html);
    });
    asyncserver.on("/reset", HTTP_GET, []( AsyncWebServerRequest * request ) {
        AsyncWebServerResponse *response = request->beginResponse( 302, "text/plain", "Please wait while the powermeter reboots" );
        response->addHeader( "Refresh", "20" );  
        response->addHeader( "Location", "/" );
        request->send(response);
        delay(3000);
        ESP.restart();    
    });
    asyncserver.on("/update", HTTP_GET, [](AsyncWebServerRequest * request) {
        request->send(200, "text/html", serverIndex);
    });
    asyncserver.on("/update", HTTP_POST,
        [](AsyncWebServerRequest *request) {},
        [](AsyncWebServerRequest *request, const String& filename, size_t index, uint8_t *data, size_t len, bool final) { handleUpdate(request, filename, index, data, len, final); }
    );

    ws.onEvent( onWsEvent );
    asyncserver.addHandler( &ws );
    asyncserver.begin();
}

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

void asyncwebserver_Task( void * pvParameters ) {
    log_i( "Start Webserver on Core: %d", xPortGetCoreID() );

    asyncwebserver_setup();

    while( true ) {
        vTaskDelay( 10 );
        ws.cleanupClients(); 
    }
}