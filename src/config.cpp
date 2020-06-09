/****************************************************************************
 *            config.cpp
 *
 *  Mo Nov 27 12:01:00 2017
 *  Copyright  2017  Dirk Brosswick
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
#include "config.h"
#include "FS.h"
#include "SPIFFS.h"

struct CFGdata cfgdata;

/*
 * 
 */
char * config_get_HostName( void ) { return( cfgdata.HostName ); }
void config_set_HostName( char * value ) { if ( strlen( value ) <= sizeof( cfgdata.HostName ) ) strcpy( cfgdata.HostName, value ); }

char * config_get_WlanSSID( void ) { return( cfgdata.WlanSSID ); }
void config_set_WlanSSID( char * value ) { if ( strlen( value ) <= sizeof( cfgdata.WlanSSID ) ) strcpy( cfgdata.WlanSSID, value ); }
char * config_get_WlanPassord( void ) { return( cfgdata.WlanPassword ); }
void config_set_WlanPassord( char * value ) { if ( strlen( value ) <= sizeof( cfgdata.WlanPassword ) ) strcpy( cfgdata.WlanPassword, value ); }

char * config_get_OTALocalApSSID( void ) { return( cfgdata.OTALocalApSSID ); }
void config_set_OTALocalApSSID( char * value ) { if ( strlen( value ) <= sizeof( cfgdata.OTALocalApSSID ) ) strcpy( cfgdata.OTALocalApSSID, value ); }
char * config_get_OTALocalApPassword( void ) { return( cfgdata.OTALocalApPassword ); }
void config_set_OTALocalApPassword( char * value ) { if ( strlen( value ) <= sizeof( cfgdata.OTALocalApPassword ) ) strcpy( cfgdata.OTALocalApPassword, value ); }

char * config_get_OTAWlanPin( void ) { return( cfgdata.OTAWlanPin ); }
void config_set_OTAWlanPin( char * value ) { if ( strlen( value ) <= sizeof( cfgdata.OTAWlanPin ) ) strcpy( cfgdata.OTAWlanPin, value ); }

char * config_get_MQTTServer( void ) { return( cfgdata.MQTTServer ); }
void config_set_MQTTServer( char * value ) { if ( strlen( value ) <= sizeof( cfgdata.MQTTServer ) ) strcpy( cfgdata.MQTTServer , value ); }
char * config_get_MQTTUser( void ) { return( cfgdata.MQTTUser ); }
void config_set_MQTTUser( char * value ) { if ( strlen( value ) <= sizeof( cfgdata.MQTTUser ) ) strcpy( cfgdata.MQTTUser , value ); }
char * config_get_MQTTPass( void ) { return( cfgdata.MQTTPass ); }
void config_set_MQTTPass( char * value ) { if ( strlen( value ) <= sizeof( cfgdata.MQTTPass ) ) strcpy( cfgdata.MQTTPass , value ); }
char * config_get_MQTTTopic( void ) { return( cfgdata.MQTTTopic ); }
void config_set_MQTTTopic( char * value ) { if ( strlen( value ) <= sizeof( cfgdata.MQTTTopic ) ) strcpy( cfgdata.MQTTTopic , value ); }
char * config_get_MQTTInterval( void ) { return( cfgdata.MQTTInterval ); }
void config_set_MQTTInterval( char * value ) { if ( strlen( value ) <= sizeof( cfgdata.MQTTInterval ) && atoi( value ) > 0 ) strcpy( cfgdata.MQTTInterval , value ); }

char * config_get_MeasureBurdenResistor( void ) { return( cfgdata.MeasureBurdenResistor ); }
void config_set_MeasureBurdenResistor( char * value ) { if ( strlen( value ) <= sizeof( cfgdata.MeasureBurdenResistor ) && atoi( value ) > 0 ) strcpy( cfgdata.MeasureBurdenResistor , value ); }
char * config_get_MeasureCoilTurns( void ) { return( cfgdata.MeasureCoilTurns ); }
void config_set_MeasureCoilTurns( char * value ) { if ( strlen( value ) <= sizeof( cfgdata.MeasureCoilTurns ) && atoi( value ) > 0 ) strcpy( cfgdata.MeasureCoilTurns , value ); }
char * config_get_MeasureVoltage( void ) { return( cfgdata.MeasureVoltage ); }
void config_set_MeasureVoltage( char * value ) { if ( strlen( value ) <= sizeof( cfgdata.MeasureVoltage ) && atoi( value ) > 0 ) strcpy( cfgdata.MeasureVoltage , value ); }
char * config_get_MeasureVoltageFrequency( void ) { return( cfgdata.MeasureVoltageFrequency ); }
void config_set_MeasureVoltageFrequency( char * value ) { if ( strlen( value ) <= sizeof( cfgdata.MeasureVoltageFrequency ) && atoi( value ) > 49 && atoi( value ) < 61 ) strcpy( cfgdata.MeasureVoltageFrequency , value ); }
char * config_get_MeasureOffset( void ) { return( cfgdata.MeasureOffset ); }
void config_set_MeasureOffset( char * value ) { if ( strlen( value ) <= sizeof( cfgdata.MeasureOffset ) ) strcpy( cfgdata.MeasureOffset , value ); }
char * config_get_MeasureChannels( void ) { return( cfgdata.MeasureChannels ); }
void config_set_MeasureChannels( char * value ) { if ( strlen( value ) <= sizeof( cfgdata.MeasureChannels ) && atoi( value ) > 0 && atoi( value ) <= 3 ) strcpy( cfgdata.MeasureChannels , value ); }
char * config_get_MeasureSamplerate( void ) { return( cfgdata.MeasureSamplerate ); }
void config_set_MeasureSamplerate( char * value ) { if ( strlen( value ) <= sizeof( cfgdata.MeasureSamplerate ) && atoi( value ) > -1000 && atoi( value ) <= 1000 ) strcpy( cfgdata.MeasureSamplerate , value ); }

/*
 * 
 */
void config_setup( void ) {
  uint8_t mac[6];
  
  if ( !SPIFFS.begin() ) {        
    /*
     * format SPIFFS if not aviable
     */
    SPIFFS.format();
    Serial.printf("formating SPIFFS\r\n");
  }

  Serial.printf("Read config from SPIFFS\r\n");
  if ( !config_read( sizeof( cfgdata ), &cfgdata, CONFIGNAME ) ) {
    Serial.printf("Write first config to SPIFFS\r\n");    
    /*
     * make an uniqe Hostname an SoftAp SSID
     */
    WiFi.macAddress( mac );
    snprintf( cfgdata.HostName, sizeof( cfgdata.HostName ), "powermeter_%02x%02x%02x", mac[3], mac[4], mac[5] );
    config_set_OTALocalApSSID( cfgdata.HostName );

    /*
     * 
     */
    config_save( sizeof ( cfgdata ), &cfgdata, CONFIGNAME );
  }
}

/*
 * 
 */
int config_save( int len, struct CFGdata *buf, const char * name ) {
  int retval = 0;

  File file = SPIFFS.open( name, "w" );

  if ( !file ) {
    Serial.printf("Can't open file: %s\r\n", name );
  }
  else {
    file.write( ( unsigned char *) buf, len );
    file.close();
    retval = len;
  }
  return( retval );
}

/*
 * 
 */
int config_read ( int len, struct CFGdata *buf, const char * name ) {
  int retval = 0;
  
  File file = SPIFFS.open( name, "r" );

  if (!file) {
    Serial.printf("Can't open '%s'!\r\n", name );
  }
  else {
    int filesize = file.size();
    if ( filesize > len ) {
      Serial.printf("Failed to read configfile. Wrong filesize!\r\n" );
    }
    else {
      file.read( ( unsigned char *) buf, filesize );
      retval = filesize;
    }
    file.close();
  }
  return( retval );
}

/*
 * 
 */
void config_saveall( void ) {
  config_save( sizeof ( cfgdata ), &cfgdata, CONFIGNAME );  
}
