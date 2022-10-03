/**
 * @file wifi_config.h
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
#ifndef _WIFICLIENT_CONFIG_H
    #define _WIFICLIENT_CONFIG_H

    #include "utils/basejsonconfig.h"
    
    #define     WIFICLIENT_JSON_CONFIG_FILE     "/wifi.json"    /** @brief defines json config file name */
    /**
     * @brief 
     */
    #define     WIFICLIENT_MAX_TEXT_SIZE   64
    /**
     * @brief ioport config structure
     */
    class wificlient_config_t : public BaseJsonConfig {
        public:
            wificlient_config_t();
            char hostname[ WIFICLIENT_MAX_TEXT_SIZE ] = "powermeter";
            char ssid[ WIFICLIENT_MAX_TEXT_SIZE ] = "";
            char password[ WIFICLIENT_MAX_TEXT_SIZE ] = "";
            bool enable_softap = true;
            char softap_ssid[ WIFICLIENT_MAX_TEXT_SIZE ] = "powermeter";
            char softap_password[ WIFICLIENT_MAX_TEXT_SIZE ] = "powermeter";
            int timeout = 15;
            bool low_bandwidth = false;
            bool low_power = false;
            
        protected:
            ////////////// Available for overloading: //////////////
            virtual bool onLoad(JsonDocument& document);
            virtual bool onSave(JsonDocument& document);
            virtual bool onDefault( void );
            virtual size_t getJsonBufferSize() { return 8192; }
    };
#endif // _WIFICLIENT_CONFIG_H
