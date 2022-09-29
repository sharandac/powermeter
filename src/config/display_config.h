/****************************************************************************
 *   Tu May 22 21:23:51 2020
 *   Copyright  2020  Dirk Brosswick
 *   Email: dirk.brosswick@googlemail.com
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
#ifndef _DISPLAY_CONFIG_H
    #define _DISPLAY_CONFIG_H

    #include "utils/basejsonconfig.h"

    #define     DISPLAY_JSON_CONFIG_FILE    "/display.json" /** @brief defines json config file name */
    #define     DISPLAY_MAX_INFO            6
    #define     DISPLAY_MAX_INFO_TEXT_SIZE  32
    /**
     * @brief 
     * 
     */
    typedef struct {
        char name[ DISPLAY_MAX_INFO_TEXT_SIZE ] = "";
        int x = 0;
        int y = 0;
        uint16_t fontsize = 0;
        uint16_t value_channel = 0;
        char text[ DISPLAY_MAX_INFO_TEXT_SIZE ] = "";
    } displayInfo_t;
    /**
     * @brief display config structure
     */
    class display_config_t : public BaseJsonConfig {
        public:
            display_config_t();
            bool active = false;
            int sda = 5;
            int sck = 18;
            bool flip = false;
            int refreshinterval = 5;
            int infocount = DISPLAY_MAX_INFO;
            displayInfo_t displayinfo[ DISPLAY_MAX_INFO ];
            
        protected:
            ////////////// Available for overloading: //////////////
            virtual bool onLoad(JsonDocument& document);
            virtual bool onSave(JsonDocument& document);
            virtual bool onDefault( void );
            virtual size_t getJsonBufferSize() { return 2048; }
    };
#endif // _DISPLAYCONFIG_H
