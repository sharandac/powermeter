/**
 * @file config.h
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
#ifndef _CONFIG_H
    #define _CONFIG_H

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
    /*
     * firmewareversion string
     */
    #define __FIRMWARE__            "2022092901"

#endif // _CONFIG_H
