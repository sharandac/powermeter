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
#ifndef _IOPORT_CONFIG_H
    #define _IOPORT_CONFIG_H

    #include "utils/basejsonconfig.h"

    enum {
        IOPORT_TRIGGER_EQUAL = 0,
        IOPORT_TRIGGER_LOWER,
        IOPORT_TRIGGER_HIGHER
    };

    enum {
        IOPORT_INACTIVE = 0,
        IOPORT_ACTIVE,
    };

    enum {
        IOPORT_NORMAL = 0,
        IOPORT_INVERTED,
    };
    
    #define     IOPORT_JSON_CONFIG_FILE     "/ioport.json" /** @brief defines json config file name */
    #define     IOPORT_MAX                  3
    #define     IOPORT_MAX_INFO_TEXT_SIZE   32
    /**
     * @brief 
     */
    typedef struct {
        char name[ IOPORT_MAX_INFO_TEXT_SIZE ] = "";
        bool active;
        bool state;
        bool start_state;
        uint16_t gpio_pin_num;
        uint16_t invert;
        uint16_t value_channel;
        uint16_t trigger;
        float value;
    } ioport_t;
    /**
     * @brief ioport config structure
     */
    class ioport_config_t : public BaseJsonConfig {
        public:
            ioport_config_t();
            int ioport_count = IOPORT_MAX;
            ioport_t ioport[ IOPORT_MAX ];
            
        protected:
            ////////////// Available for overloading: //////////////
            virtual bool onLoad(JsonDocument& document);
            virtual bool onSave(JsonDocument& document);
            virtual bool onDefault( void );
            virtual size_t getJsonBufferSize() { return 2048; }
    };
#endif // _IOPORT_CONFIG_H
