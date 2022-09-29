/**
 * @file display.h
 * @author Dirk Broßwick ( dirk.brosswick@uni-jena.de )
 * @brief 
 * @version 0.1
 * @date 2022-01-18
 * 
 * @copyright Copyright (c) 2021
 * 
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
     * @brief 
     * 
     * @return int 
     */
    int display_get_refresh_interval( void );
    /**
     * @brief 
     * 
     * @param refresh_interval 
     */
    int display_get_refresh_interval( void );
    void display_set_refresh_interval( int refresh_interval );
    char *display_get_infotext_name( uint16_t channel );
    void display_set_infotext_name( uint16_t channel, char *name );
    int display_get_infotext_x( uint16_t channel );
    void display_set_infotext_x( uint16_t channel, int x );
    int display_get_infotext_y( uint16_t channel );
    void display_set_infotext_y( uint16_t channel, int y);
    int display_get_infotext_fontsize( uint16_t channel );
    void display_set_infotext_fontsize( uint16_t channel, int fontsize );
    int display_get_infotext_value_channel( uint16_t channel );
    void display_set_infotext_value_channel( uint16_t channel, uint16_t value_channel );
    char *display_get_infotext_text( uint16_t channel );
    void display_set_infotext_text( uint16_t channel, char *text );
    void display_save_settings( void );
#endif // _DISPLAY_H