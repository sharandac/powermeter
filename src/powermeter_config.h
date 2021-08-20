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
#ifndef _POWERMETER_CONFIG_H
    #define _POWERMETER_CONFIG_H

    #include "utils/basejsonconfig.h"

    #define POWERMETER_JSON_CONFIG_FILE        "/powermeter.json"
    
    /**
     * @brief blectl config structure
     */
    class powermeter_config_t : public BaseJsonConfig {
        public:
        powermeter_config_t();
        char Firmware[16]="1";
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
        char MeasureBurdenResistor[16]="60";
        char MeasureCoilTurns[16]="2000";
        char MeasureVoltage[16]="230";
        char MeasurePhaseshift[16]="0";
        char MeasureChannels[16]="1";
        char MeasureSamplerate[16]="0";
        char MeasureVoltageFrequency[16]="50";
        char MeasureCurrentOffset[16]="0";

        protected:
        ////////////// Available for overloading: //////////////
        virtual bool onLoad(JsonDocument& document);
        virtual bool onSave(JsonDocument& document);
        virtual bool onDefault( void );
        virtual size_t getJsonBufferSize() { return 8192; }
    } ;

#endif // _POWERMETER_CONFIG_H