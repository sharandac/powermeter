/**
 * @file mqttclient.h
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
#ifndef _MQTTCLIENT_H
    #define _MQTTCLIENT_H

    /**
     * @brief start the mqqt client background task
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
    /**
     * @brief get mqtt server address
     * 
     * @return  server address as char array
     */
    const char *mqtt_client_get_server( void );
    /**
     * @brief set mqtt server
     * 
     * @param   server  serveraddress as char array
     */
    void mqtt_client_set_server( const char *server );
    /**
     * @brief get mqtt username
     * 
     * @return  username as char array
     */
    const char *mqtt_client_get_username( void );
    /**
     * @brief set mqtt username
     * 
     * @param   username    username as char array
     */
    void mqtt_client_set_username( const char *username );
    /**
     * @brief get mqtt password
     * 
     * @return  password as char array
     */
    const char *mqtt_client_get_password( void );
    /**
     * @brief set mqtt password
     * 
     * @param   password    password as char array
     */
    void mqtt_client_set_password( const char *password );
    /**
     * @brief get mqtt topic prefix
     * 
     * @return  mqtt topix prefix as char array
     */
    const char *mqtt_client_get_topic( void );
    /**
     * @brief set mqtt topix prefix
     * 
     * @param topic topix as char array
     */
    void mqtt_client_set_topic( const char *topic );
    /**
     * @brief get mqtt server port
     * 
     * @return  serverport
     */
    int mqtt_client_get_port( void );
    /**
     * @brief set server port
     * 
     * @param   port    mqtt server port
     */
    void mqtt_client_set_port( int port );
    /**
     * @brief get mqtt msg interval
     *
     * @return  mqtt msg interval in sec
     */
    int mqtt_client_get_interval( void );
    /**
     * @brief set mqtt msg interval
     * 
     * @param   interval    mqtt msg interval in sec
     */
    void mqtt_client_set_interval( int interval );
    bool mqtt_client_get_realtimestats( void );
    void mqtt_client_set_realtimestats( bool realtimestats );
    /**
     * @brief store mqtt settings as .json
     * 
     * @note all settings has a direct effect but was not stored, only here the a new json is written
     */
    void mqtt_save_settings( void );
#endif // _MQTTCLIENT_H
