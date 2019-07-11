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

/**
 *
 * \author Dirk Broßwick
 *
 */
#ifndef _OTA_H

  #define _OTA_H

  /*
   * Returnwerte für OTA-Wlan scan
   */
  #define OTA_WLAN_OK           0
  #define OTA_WLAN_FAILED       1
    
  void ota_setup( void );
  void ota_loop( void );
  int ota_update( void );
  int ota_scan( char * SSID );
  int ota_spiffupdate( void );
  void ota_Task( void * pvParameters );
  void ota_StartTask( void );
  
#endif // _OTA_H
