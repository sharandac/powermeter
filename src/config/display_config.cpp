/**
 * @file display_config.cpp
 * @author Dirk Broßwick (dirk.brosswick@googlemail.com)
 * @brief 
 * @version 1.0
 * @date 2022-10-03
 * 
 * @copyright Copyright (c) 2022
 * 
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
#include "display_config.h"
#include "measure.h"

display_config_t::display_config_t() : BaseJsonConfig(DISPLAY_JSON_CONFIG_FILE) {
}

bool display_config_t::onSave(JsonDocument& doc) {
    doc["hardware"]["active"] = active;
    doc["hardware"]["sda"] = sda;
    doc["hardware"]["sck"] = sck;
    doc["hardware"]["flip"] = flip;
    doc["hardware"]["refreshinterval"] = refreshinterval;
    doc["infocount"] = DISPLAY_MAX_INFO;

    for( int i = 0 ; i < DISPLAY_MAX_INFO ; i++ ) {
        doc["info"][ i ]["info"] = i ;
        doc["info"][ i ]["name"] = displayinfo[ i ].name;
        doc["info"][ i ]["x"] = displayinfo[ i ].x;
        doc["info"][ i ]["y"] = displayinfo[ i ].y;
        doc["info"][ i ]["fontsize"] = displayinfo[ i ].fontsize;
        doc["info"][ i ]["value_channel"] = displayinfo[ i ].value_channel;
        doc["info"][ i ]["text"] = displayinfo[ i ].text;
    }

    return true;
}

bool display_config_t::onLoad(JsonDocument& doc) {
    active = doc["hardware"]["active"] | false;
    sda = doc["hardware"]["sda"] | 5;
    sck = doc["hardware"]["sck"] | 18;
    flip = doc["hardware"]["flip"] | false;
    refreshinterval = doc["hardware"]["refreshinterval"] | 5;
    infocount = doc["infocount"] | DISPLAY_MAX_INFO;

    for( int i = 0 ; i < DISPLAY_MAX_INFO ; i++ ) {

        int infonumber = doc["info"][ i ]["info"];
        if ( infonumber >= DISPLAY_MAX_INFO )
            continue;

        strncpy( displayinfo[ i ].name, doc["info"][ i ]["name"] | "", DISPLAY_MAX_INFO_TEXT_SIZE );
        displayinfo[ infonumber ].x = doc["info"][ i ]["x"] | 0;
        displayinfo[ infonumber ].y = doc["info"][ i ]["y"] | 0;
        displayinfo[ infonumber ].fontsize = doc["info"][ i ]["fontsize"] | 0;
        displayinfo[ infonumber ].value_channel = doc["info"][ i ]["value_channel"] | 0;
        strncpy( displayinfo[ i ].text, doc["info"][ i ]["text"] | "", DISPLAY_MAX_INFO_TEXT_SIZE );
    }

    return true;
}

bool display_config_t::onDefault( void ) {
/*
    sda = 5;
    sck = 18;
    flip = false;
    refreshrate = 5;
    infocount = DISPLAY_MAX_INFO;

    for( int i = 0 ; i < DISPLAY_MAX_INFO ; i++ )
        displayinfo[ i ].type = CHANNEL_NOT_USED;
*/
    return true;
}
