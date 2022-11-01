/**
 * @file display.h
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
#ifndef _DISPLAY_H
    #define _DISPLAY_H

    #define SCREEN_WIDTH    128
    #define SCREEN_HEIGHT   64
    #define SCREEN_ADDRESS  0x3C
    /**
     * @brief SSD1306 display setup function
     */
    void display_init( void );
    /**
     * @brief display loop function
     */
    void display_loop( void );
    /**
     * @brief set display active/inactive
     * 
     * @param active    true measn active, false means inactive
     */
    void display_set_active( bool active );
    /**
     * @brief get display active/inactive state
     * 
     * @return true         active
     * @return false        inactive
     */
    bool display_get_active( void );
    /**
     * @brief set display sda pin number
     * 
     * @param pin   sda pin number
     */
    void display_set_sda_pin( int pin );
    /**
     * @brief get display sda pin number
     * 
     * @return pin number
     */
    int display_get_sda_pin( void );
    /**
     * @brief set display sck pin number
     * 
     * @param pin   sck pin number
     */
    void display_set_sck_pin( int pin );
    /**
     * @brief get display sck pin number
     * 
     * @return pin number
     */
    int display_get_sck_pin( void );
    /**
     * @brief set display flip (rotate 180 degree)
     * 
     * @param flip      false no flip, true flip the display
     */
    void display_set_flip( bool flip );
    /**
     * @brief get display flip (rotate 180 degree)
     * 
     * @return  false no flip, true flip the display
     */
    bool display_get_flip( void );
    /**
     * @brief get display refresh interval
     * 
     * @return int 
     */
    int display_get_refresh_interval( void );
    /**
     * @brief set display refresh interval
     * 
     * @param refresh_interval 
     */
    void display_set_refresh_interval( int refresh_interval );
    /**
     * @brief get display infotext name
     * 
     * @param channel  display channel name
     * @return char* 
     */
    char *display_get_infotext_name( uint16_t channel );
    /**
     * @brief set display infotext name
     * 
     * @param channel   display channel name
     * @param name 
     */
    void display_set_infotext_name( uint16_t channel, char *name );
    /**
     * @brief get display infotext x coor
     * 
     * @param channel   dispplay channel number
     * @return int 
     */
    int display_get_infotext_x( uint16_t channel );
    /**
     * @brief set display infotext x coor
     * 
     * @param channel   display channel number
     * @param x 
     */
    void display_set_infotext_x( uint16_t channel, int x );
    /**
     * @brief get display infotext y coor
     * 
     * @param channel   dispplay channel number
     * @return int 
     */
    int display_get_infotext_y( uint16_t channel );
    /**
     * @brief set display infotext y coor
     * 
     * @param channel   display channel number
     * @param x 
     */
    void display_set_infotext_y( uint16_t channel, int y);
    /**
     * @brief get infotext font size
     * 
     * @param channel   display channel number
     * @return int 
     */
    int display_get_infotext_fontsize( uint16_t channel );
    /**
     * @brief set display infotext font size
     * 
     * @param channel   display channel number
     * @param fontsize  fontsize from 0-2
     */
    void display_set_infotext_fontsize( uint16_t channel, int fontsize );
    /**
     * @brief get display infotext value channel
     * 
     * @param channel   display channel number
     * @return int      value channel number
     */
    int display_get_infotext_value_channel( uint16_t channel );
    /**
     * @brief set display infotext value channel
     * 
     * @param channel   display channel number
     * @param value_channel     value channel number
     */
    void display_set_infotext_value_channel( uint16_t channel, uint16_t value_channel );
    /**
     * @brief get display infotext prefix text
     * 
     * @param channel   display channel number
     * @return char* 
     */
    char *display_get_infotext_text( uint16_t channel );
    /**
     * @brief set display infotext prefix text
     * 
     * @param channel   display channel number
     * @param text 
     */
    void display_set_infotext_text( uint16_t channel, char *text );
    /**
     * @brief store all settings to display.json
     */
    void display_save_settings( void );
#endif // _DISPLAY_H