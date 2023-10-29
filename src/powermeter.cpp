/**
 * @file powermeter.cpp
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
#include <Arduino.h>
#include <FS.h>
#include <SPIFFS.h>

#include "config.h"
#include "display.h"
#include "ioport.h"
#include "measure.h"
#include "mqttclient.h"
#include "ntp.h"
#include "webserver.h"
#include "wificlient.h"
/**
 * @brief arduino setup function
 */
void setup( void ) {
    setCpuFrequencyMhz( 240 );
    Serial.begin(115200);
    /*
     * hardware stuff and file system
     */
    log_i("Start Main Task on Core: %d", xPortGetCoreID() );
    if ( !SPIFFS.begin() ) {
        log_i("format SPIFFS ..." );
        SPIFFS.format();
    }
    ioport_init();
    display_init();
    wificlient_init();
    /*
     * Setup Tasks
     */
    measure_StartTask();
    mqtt_client_StartTask();
    asyncwebserver_StartTask();
    ntp_StartTask();
}

/**
 * @brief arduino main loop
 */
void loop() {
    display_loop();
    ioport_loop();
}
