  /****************************************************************************
 *            powermeter.cpp
 *
 *  Sa April 27 09:23:14 2019
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
#include <Arduino.h>
#include "FS.h"
#include "SPIFFS.h"

#include "measure.h"
#include "ioport.h"
#include "config.h"
#include "webserver.h"
#include "mqttclient.h"
#include "wificlient.h"
#include "ntp.h"
#include "display.h"

/**
 * @brief arduino setup function
 */
void setup( void ) {
    if ( !SPIFFS.begin() )       
        SPIFFS.format();
    /*
     * doing setup Serial an config
     */
    Serial.begin(115200);
    ioport_init();
    display_init();
    wificlient_init();
    /*
     * Setup Tasks
     */
    log_i("Start Main Task on Core: %d", xPortGetCoreID() );
    ntp_StartTask();
    mqtt_client_StartTask();
    asyncwebserver_StartTask();
    measure_StartTask();
}

/**
 * @brief arduino main loop
 */
void loop() {
    wificlient_loop();
    display_loop();
    ioport_loop();
    delay(10);
}
