/****************************************************************************
 *            ioport.h
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

#ifndef _IOPORT_H
    #define _IOPORT_H
    
    void ioport_init( void );
    void ioport_loop( void );
    char *ioport_get_name( uint16_t channel );
    void ioport_set_name( uint16_t channel, char *name );
    bool ioport_get_start_state( uint16_t channel );
    void ioport_set_start_state( uint16_t channel, uint16_t start_state );
    bool ioport_get_active( uint16_t channel );
    void ioport_set_active( uint16_t channel, uint16_t active );
    uint16_t ioport_get_gpio_pin_num( uint16_t channel );
    void ioport_set_gpio_pin_num( uint16_t channel, uint16_t gpio_pin_num );
    uint16_t ioport_get_invert( uint16_t channel );
    void ioport_set_invert( uint16_t channel, uint16_t invert );
    uint16_t ioport_get_value_channel( uint16_t channel );
    void ioport_set_value_channel( uint16_t channel, uint16_t value_channel );
    uint16_t ioport_get_trigger( uint16_t channel );
    void ioport_set_trigger( uint16_t channel, uint16_t trigger );
    float ioport_get_value( uint16_t channel );
    void ioport_set_value( uint16_t channel, float value );
    bool ioport_get_state( uint16_t channel );
    void ioport_save_settings( void );
#endif // _IOPORT_H