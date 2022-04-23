/****************************************************************************
 *            mqtt_client.h
 *
 *  Sa April 27 12:01:00 2019
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
#ifndef _MQTTCLIENT_H
    #define _MQTTCLIENT_H

    /**
     * @brief start the mqqt client background task
     * 
     */
    void mqtt_client_StartTask( void );
    /**
     * @brief publish a mqqt msg to a given topic
     * 
     * @param topic     topic
     * @param payload   msg to send
     */
    void mqtt_client_publish( char * topic, char * payload );
    /**
     * @brief disable all mqtt connections
     */
    void mqtt_client_disable( void );
    /**
     * @brief enable all mqtt connections and reload all connection settings
     */
    void mqtt_client_enable( void );

#endif // _MQTTCLIENT_H
