/****************************************************************************
 *   Aug 11 17:13:51 2020
 *   Copyright  2020  Dirk Brosswick
 *   Email: dirk.brosswick@googlemail.com
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
#include "powermeter_config.h"
#include "measure.h"

powermeter_config_t::powermeter_config_t() : BaseJsonConfig( POWERMETER_JSON_CONFIG_FILE ) {}

bool powermeter_config_t::onSave(JsonDocument& doc) {    
    log_i("save config");

    doc["powermeter"]["firmware"] = Firmware;
    doc["powermeter"]["hostName"] = HostName;
    doc["powermeter"]["wifi"]["SSID"] = WlanSSID;
    doc["powermeter"]["wifi"]["Password"] = WlanPassword;
    doc["powermeter"]["wifi"]["OTALocalApSSID"] = OTALocalApSSID;
    doc["powermeter"]["wifi"]["OTALocalApPassword"] = OTALocalApPassword;
    doc["powermeter"]["wifi"]["OTAWlanPin"] = OTAWlanPin;
    doc["powermeter"]["mqtt"]["Server"] = MQTTServer;
    doc["powermeter"]["mqtt"]["User"] = MQTTUser;
    doc["powermeter"]["mqtt"]["Pass"] = MQTTPass;
    doc["powermeter"]["mqtt"]["Topic"] = MQTTTopic;
    doc["powermeter"]["mqtt"]["Interval"] = MQTTInterval;
    doc["powermeter"]["measure"]["BurdenResistor"] = MeasureBurdenResistor;
    doc["powermeter"]["measure"]["CoilTurns"] = MeasureCoilTurns;
    doc["powermeter"]["measure"]["Voltage"] = MeasureVoltage;
    doc["powermeter"]["measure"]["Phaseshift"] = MeasurePhaseshift;
    doc["powermeter"]["measure"]["Channels"] = MeasureChannels;
    doc["powermeter"]["measure"]["Samplerate"] = MeasureSamplerate;
    doc["powermeter"]["measure"]["VoltageFrequency"] = MeasureVoltageFrequency;
    doc["powermeter"]["measure"]["CurrentOffset"] = MeasureCurrentOffset;
    doc["powermeter"]["measure"]["VirtualChannels"] = VIRTUAL_CHANNELS;
    for( int i = 0 ; i < VIRTUAL_CHANNELS ; i++ ) {
        char microcode[ VIRTUAL_CHANNELS * 3 ] = "";
        doc["powermeter"]["measure"]["VirtualChannel"][ i ]["channel"] = i;
        doc["powermeter"]["measure"]["VirtualChannel"][ i ]["type"] = measure_get_channel_type( i );
        doc["powermeter"]["measure"]["VirtualChannel"][ i ]["phaseshift"] = measure_get_channel_phaseshift( i );
        doc["powermeter"]["measure"]["VirtualChannel"][ i ]["mircocode"] = measure_get_channel_opcodeseq_str( i, sizeof( microcode ), microcode );
    }

    return true;
}

bool powermeter_config_t::onLoad(JsonDocument& doc) {
    int virtualchannels = 0;

    strncpy( Firmware, doc["powermeter"]["firmware"] | "1", sizeof( Firmware ) );
    strncpy( HostName, doc["powermeter"]["hostName"] | "powermeter", sizeof( HostName ) );
    strncpy( WlanSSID, doc["powermeter"]["wifi"]["SSID"] | "", sizeof( WlanSSID ) );
    strncpy( WlanPassword, doc["powermeter"]["wifi"]["Password"] | "", sizeof( WlanPassword ) );
    strncpy( OTALocalApSSID, doc["powermeter"]["wifi"]["OTALocalApSSID"] | "powermeter", sizeof( OTALocalApSSID ) );
    strncpy( OTALocalApPassword, doc["powermeter"]["wifi"]["OTALocalApPassword"] | "powermeter", sizeof( OTALocalApPassword ) );
    strncpy( OTAWlanPin, doc["powermeter"]["wifi"]["OTAWlanPin"] | "12345678", sizeof( OTAWlanPin ) );
    strncpy( MQTTServer, doc["powermeter"]["mqtt"]["Server"] | "", sizeof( MQTTServer ) );
    strncpy( MQTTUser, doc["powermeter"]["mqtt"]["User"] | "", sizeof( MQTTUser ) );
    strncpy( MQTTPass, doc["powermeter"]["mqtt"]["Pass"] | "", sizeof( MQTTPass ) );
    strncpy( MQTTTopic, doc["powermeter"]["mqtt"]["Topic"] | "", sizeof( MQTTTopic ) );
    strncpy( MQTTInterval, doc["powermeter"]["mqtt"]["Interval"] | "15", sizeof( MQTTInterval ) );
    strncpy( MeasureBurdenResistor, doc["powermeter"]["measure"]["BurdenResistor"] | "60", sizeof( MeasureBurdenResistor ) );
    strncpy( MeasureCoilTurns, doc["powermeter"]["measure"]["CoilTurns"] | "2000", sizeof( MeasureCoilTurns ) );
    strncpy( MeasureVoltage, doc["powermeter"]["measure"]["Voltage"] | "230", sizeof( MeasureVoltage ) );
    strncpy( MeasurePhaseshift, doc["powermeter"]["measure"]["Phaseshift"] | "0", sizeof( MeasurePhaseshift ) );
    strncpy( MeasureChannels, doc["powermeter"]["measure"]["Channels"] | "1", sizeof( MeasureChannels ) );
    strncpy( MeasureSamplerate, doc["powermeter"]["measure"]["Samplerate"] | "0", sizeof( MeasureSamplerate ) );
    strncpy( MeasureVoltageFrequency, doc["powermeter"]["measure"]["VoltageFrequency"] | "50", sizeof( MeasureVoltageFrequency ) );
    strncpy( MeasureCurrentOffset, doc["powermeter"]["measure"]["CurrentOffset"] | "0", sizeof( MeasureCurrentOffset ) );

    virtualchannels = doc["powermeter"]["measure"]["VirtualChannels"] | VIRTUAL_CHANNELS;
    for( int i = 0 ; i < virtualchannels ; i++ ) {
        int channel = doc["powermeter"]["measure"]["VirtualChannel"][ i ]["channel"];
        const char *opcodeseq_str = doc["powermeter"]["measure"]["VirtualChannel"][ i ]["mircocode"];

        measure_set_channel_type( channel, doc["powermeter"]["measure"]["VirtualChannel"][ i ]["type"].as<uint8_t>() );
        measure_set_channel_phaseshift( channel, doc["powermeter"]["measure"]["VirtualChannel"][ i ]["phaseshift"].as<int16_t>() );
        measure_set_channel_opcodeseq_str( channel, opcodeseq_str );        
    }

    return true;
}

bool powermeter_config_t::onDefault( void ) {
    uint8_t mac[6];
    log_i("Write first config to SPIFFS\r\n");    
    /*
     * make an uniqe Hostname for the SoftAp SSID
     */
    WiFi.macAddress( mac );
    snprintf( HostName, sizeof( HostName ), "powermeter_%02x%02x%02x", mac[3], mac[4], mac[5] );
    snprintf( OTALocalApSSID, sizeof( OTALocalApSSID ), "powermeter_%02x%02x%02x", mac[3], mac[4], mac[5] );
    return false;
}