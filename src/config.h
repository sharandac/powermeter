/****************************************************************************
              config.h

    Tu May 29 21:23:51 2017
    Copyright  2017  Dirk Brosswick
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
#ifndef _CONFIG_H

  #define _CONFIG_H
  /*
   * 
   */
  struct CFGdata {
    char Firmware[16]="";
    char HostName[64]="";
    char WlanSSID[64]="";
    char WlanPassword[64]="";
    char OTALocalApSSID[64]="";
    char OTALocalApPassword[64]="powermeter";
    char OTAWlanPin[16]="12345678";
    char MQTTServer[32]="";
    char MQTTUser[32]="";
    char MQTTPass[32]="";
    char MQTTTopic[32]="test";
    char MQTTInterval[32]="15";
    char MeasureBurdenResistor[16]="62";
    char MeasureCoilTurns[16]="1800";
    char MeasureVoltage[16]="230";
    char MeasureOffset[16]="0";
    char MeasureChannels[16]="1";
    char MeasureSamplerate[16]="0";
    char MeasureVoltageFrequency[16]="50";
  };

  #if CONFIG_FREERTOS_UNICORE
  
    #define _MEASURE_TASKCORE    1
    #define _MQTT_TASKCORE       1
    #define _WEBSOCK_TASKCORE    1
    #define _OTA_TASKCORE        1
    #define _WEBSERVER_TASKCORE  1
    #define _NTP_TASKCORE        1  

  #else
  
    #define _MEASURE_TASKCORE    0
    #define _MQTT_TASKCORE       1
    #define _WEBSOCK_TASKCORE    1
    #define _OTA_TASKCORE        1
    #define _WEBSERVER_TASKCORE  1
    #define _NTP_TASKCORE        1  
  
  #endif // CONFIG_FREERTOS_UNICORE

  void config_setup( void );
  int config_save( int len, struct CFGdata *buf, const char * name );
  int config_read( int len, struct CFGdata *buf, const char * name );
  void config_saveall( void );

  char * config_get_HostName( void );
  void config_set_HostName( char * value );

  char * config_get_WlanSSID( void );
  void config_set_WlanSSID( char * value );
  char * config_get_WlanPassord( void );
  void config_set_WlanPassord( char * value );

  char * config_get_OTAWlanPin( void );
  void config_set_OTAWlanPin( char * value );

  char * config_get_OTALocalApSSID( void );
  void config_set_OTALocalApSSID( char * value );
  char * config_get_OTALocalApPassword( void );
  void config_set_OTALocalApPassword( char * value );

  char * config_get_MQTTServer( void );
  void config_set_MQTTServer( char * value );
  char * config_get_MQTTUser( void );
  void config_set_MQTTUser( char * value );
  char * config_get_MQTTPass( void );
  void config_set_MQTTPass( char * value );
  char * config_get_MQTTTopic( void );
  void config_set_MQTTTopic( char * value );
  char * config_get_MQTTInterval( void );
  void config_set_MQTTInterval( char * value );

  char * config_get_MeasureBurdenResistor( void );
  void config_set_MeasureBurdenResistor( char * value );
  char * config_get_MeasureCoilTurns( void );
  void config_set_MeasureCoilTurns( char * value );
  char * config_get_MeasureVoltage( void );
  void config_set_MeasureVoltage( char * value );
  char * config_get_MeasureVoltageFrequency( void );
  void config_set_MeasureVoltageFrequency( char * value );
  char * config_get_MeasureOffset( void );
  void config_set_MeasureOffset( char * value );
  char * config_get_MeasureChannels( void );
  void config_set_MeasureChannels( char * value );
  char * config_get_MeasureSamplerate( void );
  void config_set_MeasureSamplerate( char * value );
  
  char * config_get_FirmwareURL( void );
  void config_set_FirmwareURL( char * value );

  char * config_get_SPIFFSURL( void );
  void config_set_SPIFFSURL( char * value );

  /*
   * firmewareversion string
   */
  #define __FIRMWARE__            "2020060801"

  /*
   * Config filename
   */
  #define CONFIGNAME              "/config.cfg"

  /*
   *  WLAN-Daten
   */
  #define WLAN_CONNECT_TIMEOUT    15      /* zeit zum verbinden in Sekunden */

#endif // _CONFIG_H
