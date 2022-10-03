/**
 * @file ioport.cpp
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
#include <Arduino.h>
#include "ioport.h"
#include "measure.h"

ioport_config_t ioport_config;

void ioport_init( void ) {
    /**
     * load config from json
     */
    ioport_config.load();
    /**
     * crawl all ioports and setup pin config
     */
    for( int channel = 0 ; channel < IOPORT_MAX ; channel++ ) {
        if( ioport_config.ioport[ channel ].active ) {
            pinMode( ioport_config.ioport[ channel ].gpio_pin_num, OUTPUT );
            digitalWrite( ioport_config.ioport[ channel ].gpio_pin_num, ioport_config.ioport[ channel ].start_state );
            ioport_config.ioport[ channel ].state = ioport_config.ioport[ channel ].start_state;
        }
    }
}

void ioport_loop( void ) {
    static uint64_t NextIOPortMillis = 0;
    /**
     * on first run, init NextIOPortMillis
     */
    if( NextIOPortMillis == 0 ) {
        NextIOPortMillis = millis();
    }
    /**
     * check next NextIOPortMillis
     */
    if( NextIOPortMillis <= millis() ) {
        NextIOPortMillis = millis() + 100l;  
        /**
         * check if measurement was valid
         */
        if( !measure_get_measurement_valid() )
            return;
        /**
         * crawl ioports
         */
        for( int ioport = 0 ; ioport < IOPORT_MAX ; ioport++ ) {
            if( ioport_config.ioport[ ioport ].active ) {
                bool trigger = false;
                /**
                 * check if group id and type exist
                 */
                if( measure_get_channel_type( ioport_config.ioport[ ioport ].value_channel ) == CHANNEL_NOT_USED )
                    continue;
                /**
                 * get group id and type value
                 */
                float value = measure_get_channel_rms( ioport_config.ioport[ ioport ].value_channel );
                /**
                 * check trigger type
                 */
                switch ( ioport_config.ioport[ ioport ].trigger ) {
                    case IOPORT_TRIGGER_EQUAL:
                        if( value == ioport_config.ioport[ ioport ].value )
                            trigger = true;
                        break;
                    case IOPORT_TRIGGER_LOWER:
                        if( value < ioport_config.ioport[ ioport ].value )
                            trigger = true;
                        break;
                    case IOPORT_TRIGGER_HIGHER:
                        if( value > ioport_config.ioport[ ioport ].value )
                            trigger = true;
                        break;
                    default:
                        continue;
                }
                /**
                 * invert trigger if configured
                 */
                ioport_config.ioport[ ioport ].state = trigger;
                if( ioport_config.ioport[ ioport ].invert == IOPORT_INVERTED ) {
                    if( trigger )
                        trigger = false;
                    else
                        trigger = true;
                }
                /**
                 * write out trigger
                 */
                digitalWrite( ioport_config.ioport[ ioport ].gpio_pin_num, trigger ? HIGH : LOW );
            }
        }
    }  
}

char *ioport_get_name( uint16_t channel ) {
    return( ioport_config.ioport[ channel ].name );
}
void ioport_set_name( uint16_t channel, char *name ) {
    strlcpy( ioport_config.ioport[ channel ].name, name, IOPORT_MAX_INFO_TEXT_SIZE );
}

bool ioport_get_start_state( uint16_t channel ) {
    return( ioport_config.ioport[ channel ].start_state );
}
void ioport_set_start_state( uint16_t channel, uint16_t start_state ) {
    ioport_config.ioport[ channel ].start_state = start_state;
}

ioport_active_t ioport_get_active( uint16_t channel ) {
    return( ioport_config.ioport[ channel ].active );
}
void ioport_set_active( uint16_t channel, ioport_active_t active ) {
    ioport_config.ioport[ channel ].active = active;
}

uint16_t ioport_get_gpio_pin_num( uint16_t channel ) {
    return( ioport_config.ioport[ channel ].gpio_pin_num );
}
void ioport_set_gpio_pin_num( uint16_t channel, uint16_t gpio_pin_num ) {
    /**
     * clear old gpio pin if active and activate new gpio pin
     */
    if( ioport_config.ioport[ channel ].active == IOPORT_ACTIVE ) {
        pinMode( ioport_config.ioport[ channel ].gpio_pin_num, INPUT );
        pinMode( gpio_pin_num, OUTPUT );
    }
    ioport_config.ioport[ channel ].gpio_pin_num = gpio_pin_num;
}

ioport_output_t ioport_get_invert( uint16_t channel ) {
    return( ioport_config.ioport[ channel ].invert );
}
void ioport_set_invert( uint16_t channel, ioport_output_t invert ) {
    ioport_config.ioport[ channel ].invert = invert;
}

uint16_t ioport_get_value_channel( uint16_t channel ) {
    return( ioport_config.ioport[ channel ].value_channel );
}
void ioport_set_value_channel( uint16_t channel, uint16_t value_channel ) {
    ioport_config.ioport[ channel ].value_channel = value_channel;
}

ioport_trigger_t ioport_get_trigger( uint16_t channel ) {
    return( ioport_config.ioport[ channel ].trigger );
}
void ioport_set_trigger( uint16_t channel, ioport_trigger_t trigger ) {
    ioport_config.ioport[ channel ].trigger = trigger;
}

float ioport_get_value( uint16_t channel ) {
    return( ioport_config.ioport[ channel ].value );
}
void ioport_set_value( uint16_t channel, float value ) {
    ioport_config.ioport[ channel ].value = value;
}

bool ioport_get_state( uint16_t channel ) {
    return( ioport_config.ioport[ channel ].state );
}

void ioport_save_settings( void ) {
    ioport_config.save();
}