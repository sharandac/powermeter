/****************************************************************************
 *   Tu May 22 21:23:51 2020
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
#include "mqtt_config.h"
#include "mqttclient.h"

mqtt_config_t::mqtt_config_t() : BaseJsonConfig( MQTT_JSON_CONFIG_FILE ) {
}

bool mqtt_config_t::onSave(JsonDocument& doc) {

    doc["server"] = server;
    doc["port"] = port;
    doc["username"] = username;
    doc["password"] = password;
    doc["topic"] = topic;
    doc["interval"] = interval;
    doc["realtimestats"] = realtimestats;

    return true;
}

bool mqtt_config_t::onLoad(JsonDocument& doc) {

    strlcpy( server, doc["server"] | "", sizeof( server ) );
    port = doc["port"] | 1883;
    strlcpy( username, doc["username"] | "", sizeof( username ) );
    strlcpy( password, doc["password"] | "", sizeof( password ) );
    strlcpy( topic, doc["topic"] | "", sizeof( topic ) );
    interval = doc["interval"] | 15;
    realtimestats = doc["realtimestats"] | true;
    
    return true;
}

bool mqtt_config_t::onDefault( void ) {

    return true;
}
