/****************************************************************************
 *            wificlient.h
 *
 *  Sa April 27 10:17:37 2019
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

#ifndef _WIFICLIENT_H
    #define _WIFICLIENT_H
    
    void wificlient_init( void );
    void wificlient_loop( void );

    const char *wificlient_get_hostname( void );
    void wificlient_set_hostname( const char * hostname );
    const char *wificlient_get_ssid( void );
    void wificlient_set_ssid( const char * ssid );
    const char *wificlient_get_password( void );
    void wificlient_set_password( const char * password );
    const char *wificlient_get_softap_ssid( void );
    void wificlient_set_softap_ssid( const char * softap_ssid );
    const char *wificlient_get_softap_password( void );
    void wificlient_set_softap_password( const char * softap_password );
    bool wificlient_get_enable_softap( void );
    void wificlient_set_enable_softap( bool enable_softap );
    int wificlient_get_timeout( void );
    void wificlient_set_timeout( int timeout );
    bool wificlient_get_low_bandwidth( void );
    void wificlient_set_low_bandwidth( bool low_bandwidth );
    bool wificlient_get_low_power( void );
    void wificlient_set_low_power( bool low_power );
    void wificlient_save_settings( void );
#endif // _IOPORT_H