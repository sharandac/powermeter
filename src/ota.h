/****************************************************************************
 *            ota.cpp
 *
 *  Sa April 27 12:17:32 2019
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
#ifndef _OTA_H

    #define _OTA_H
    /*
     * Returnwerte für OTA-Wlan scan
     */
    #define OTA_WLAN_OK           0
    #define OTA_WLAN_FAILED       1
    
    /**
     * @brief start ota background task
     */
    void ota_StartTask( void );
    /**
     * @brief scan wifi networks and print it out
     * 
     * @param SSID      a given network SSID scan for
     * @return int      OTA_WLAN_OK or OTA_WLAN_FAILED if SSID network name found
     */
    int ota_scan( char * SSID );
  
#endif // _OTA_H
