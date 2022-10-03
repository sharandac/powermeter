/**
 * @file ioport.h
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
#ifndef _IOPORT_H
    #define _IOPORT_H
    
    #include "config/ioport_config.h"
    /**
     * @brief 
     */
    void ioport_init( void );
    /**
     * 
     */
    void ioport_loop( void );
    /**
     * @brief get ioport channel name
     * 
     * @param channel   ioport channel number
     * @return char*    ioport channel name as char array
     */
    char *ioport_get_name( uint16_t channel );
    /**
     * @brief set ioport channel name
     * 
     * @param channel   ioport channel number
     * @param name      ioport channel name as char array
     */
    void ioport_set_name( uint16_t channel, char *name );
    /**
     * @brief get ioport channel start state after reset/start
     * 
     * @param channel   ioport channel number
     * @return true     state after reset/start is set
     * @return false    state after reset/start is clear
     * 
     * @note the real state depends on invert config
     */
    bool ioport_get_start_state( uint16_t channel );
    /**
     * @brief get ioport start state after reset/start
     * 
     * @param channel           ioport channel number
     * @param start_state       start state after reset/start
     * 
     * @note the real state depends on invert config
     */
    void ioport_set_start_state( uint16_t channel, uint16_t start_state );
    /**
     * @brief get ioport channel is active or inactive
     * 
     * @param channel   ioport channel
     * @return true     means ioport channel is active
     * @return false    means ioport channel is inactive
     */
    ioport_active_t ioport_get_active( uint16_t channel );
    /**
     * @brief set ioport channel active or inactive
     * 
     * @param channel   ioport channel number
     * @param active    true means active, false means inactive
     */
    void ioport_set_active( uint16_t channel, ioport_active_t active );
    /**
     * @brief get ioport channel output pin
     * 
     * @param channel   ioport channel number
     * @return uint16_t ioport channel output pin number
     */
    uint16_t ioport_get_gpio_pin_num( uint16_t channel );
    /**
     * @brief set ioport channel output pin
     * 
     * @param channel       ioport channel number
     * @param gpio_pin_num  ioport channel output pin number
     */
    void ioport_set_gpio_pin_num( uint16_t channel, uint16_t gpio_pin_num );
    /**
     * @brief get ioport channel output pin is inverted
     * 
     * @param channel   ioport channel number
     * @return uint16_t 0 means normal, 1 means inverted
     */
    ioport_output_t ioport_get_invert( uint16_t channel );
    /**
     * @brief set ioport channel output is inverted
     * 
     * @param channel   ioport channel number
     * @param invert    0 means normal, 1 means inverted
     */
    void ioport_set_invert( uint16_t channel, ioport_output_t invert );
    /**
     * @brief get channel output value source  
     * 
     * @param channel       ioport channel number
     * @return uint16_t     value source
     */
    uint16_t ioport_get_value_channel( uint16_t channel );
    /**
     * @brief set channel output value source
     * 
     * @param channel       ioport channel number
     * @param value_channel value source
     */
    void ioport_set_value_channel( uint16_t channel, uint16_t value_channel );
    /**
     * @brief get channel output is set trigger type
     *  
     * @param channel       ioport channel number
     * @return uint16_t     
     */
    ioport_trigger_t ioport_get_trigger( uint16_t channel );
    /**
     * @brief set channel output is set trigger type
     * 
     * @param channel       ioport channel number 
     * @param trigger       
     */
    void ioport_set_trigger( uint16_t channel, ioport_trigger_t trigger );
    /**
     * @brief get channl output trigger value
     * 
     * @param channel       ioport channel number
     * @return float 
     */
    float ioport_get_value( uint16_t channel );
    /**
     * @brief set ioport channel trigger value
     * 
     * @param channel   ioport channel number
     * @param value     ioport channel output trigger value
     */
    void ioport_set_value( uint16_t channel, float value );
    /**
     * @brief get channel output state
     * 
     * @param channel   ioport channel value
     * @return true     
     * @return false 
     */
    bool ioport_get_state( uint16_t channel );
    void ioport_save_settings( void );
#endif // _IOPORT_H