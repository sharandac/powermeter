/**
 * @file wificlient.h
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
#ifndef _WIFICLIENT_H
    #define _WIFICLIENT_H
    /**
     * @brief init wifi client
     */
    void wificlient_init( void );
    /**
     * @brief get hostname
     * 
     * @return const char* 
     */
    const char *wificlient_get_hostname( void );
    /**
     * @brief set hostname
     * 
     * @param hostname 
     */
    void wificlient_set_hostname( const char * hostname );
    /**
     * @brief get wifi ssid
     * 
     * @return const char* 
     */
    const char *wificlient_get_ssid( void );
    /**
     * @brief set wifi ssid
     * 
     * @param ssid 
     */
    void wificlient_set_ssid( const char * ssid );
    /**
     * @brief get wifi ssid password
     * 
     * @return const char* 
     */
    const char *wificlient_get_password( void );
    /**
     * @brief set wifi ssid password
     * 
     * @param password 
     */
    void wificlient_set_password( const char * password );
    /**
     * @brief get wifi softAP ssid
     * 
     * @return const char* 
     */
    const char *wificlient_get_softap_ssid( void );
    /**
     * @brief set wifi softAP ssid
     * 
     * @param softap_ssid 
     */
    void wificlient_set_softap_ssid( const char * softap_ssid );
    /**
     * @brief get wifi softAP ssid password 
     * 
     * @return const char* 
     */
    const char *wificlient_get_softap_password( void );
    /**
     * @brief set wifi softAP ssid password
     * 
     * @param softap_password 
     */
    void wificlient_set_softap_password( const char * softap_password );
    /**
     * @brief get wifi softAP enable state
     * 
     * @return true 
     * @return false 
     */
    bool wificlient_get_enable_softap( void );
    /**
     * @brief set wifi softAP enable/disable
     * 
     * @param enable_softap 
     */
    void wificlient_set_enable_softap( bool enable_softap );
    /**
     * @brief get wifi connect timeout before softAP start
     * 
     * @return int 
     */
    int wificlient_get_timeout( void );
    /**
     * @brief set wifi connect timeout before softAP start
     * 
     * @param timeout 
     */
    void wificlient_set_timeout( int timeout );
    /**
     * @brief get wifi client low bandwidth
     * 
     * @return true     20MHz bandwidth
     * @return false    40MHz bandwidth
     */
    bool wificlient_get_low_bandwidth( void );
    /**
     * @brief set wifi lient low bandwidth
     * 
     * @param low_bandwidth     true means 20MHz bandwidth, false means 40MHz bandwidth
     */
    void wificlient_set_low_bandwidth( bool low_bandwidth );
    /**
     * @brief get wifi client low power
     * 
     * @return true         low power enabled
     * @return false        low power disabled
     */
    bool wificlient_get_low_power( void );
    /**
     * @brief set wifi client low power
     * 
     * @param low_power     true means low power, false means normals power
     */
    void wificlient_set_low_power( bool low_power );
    /**
     * @brief store current settings to wifi.json
     */
    void wificlient_save_settings( void );
#endif // _IOPORT_H