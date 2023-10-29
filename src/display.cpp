/**
 * @file display.cpp
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
#include <Wire.h>
#include <Adafruit_GFX.h>
// #include <Adafruit_SSD1306.h>
#include <Adafruit_SH110X.h>
#include <Fonts/FreeSans18pt7b.h>
#include <Fonts/FreeSans12pt7b.h>
#include <Fonts/FreeSans9pt7b.h>

#include "config.h"
#include "config/display_config.h"
#include "display.h"
#include "measure.h"

display_config_t display_config;

#if defined( _Adafruit_SSD1306_H_ )
    Adafruit_SSD1306 display( SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1 );
#elif defined( _Adafruit_SH110X_H_ )
    Adafruit_SH1106G display( SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1 );
#else
    #error "no display driver found"
#endif

static bool displayinit = false;

int display_get_string_width( const char *fmt );

void display_init( void ) {
    int scan_hit = 0;
    log_i("init display" );
    /**
     * load json config file
     */
    display_config.load();
    /**
     * if display working, block reinit or inactive
     */
    if( displayinit )
        return;    
    if( !display_config.active )
        return;
    /**
     * init wire interface and scan
     */
    log_i("scan i2c bus" );
    Wire.begin( display_config.sda, display_config.sck, 400000l );
    for( int i = 0; i < 0x7f; i++ ) {
        Wire.beginTransmission( i );
        if( !Wire.endTransmission() ) {
            scan_hit++;
            log_i("device found at %x", i );
        }
    }
    log_i("scan i2c done, found %d devices", scan_hit );
    if( scan_hit == 0 ) {
        log_e("no i2c devices found");
        return;
    }
    /**
     * init display
     */
#if defined( _Adafruit_SSD1306_H_ )
    if( !display.begin( SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS ) ) { 
        displayinit = false;
        log_e("SSD1306 allocation failed");
        return;
    }
#elif defined( _Adafruit_SH110X_H_ )
    if( !display.begin( SCREEN_ADDRESS ) ) { 
        displayinit = false;
        log_e("SH110X allocation failed");
        return;
    }
#else
    #error "no display driver found"
#endif

    /**
     * clear display and set startup message
     */
    display.display();
    display.setRotation( display_config.flip ? 2 : 0 );
    display.clearDisplay();
#if defined( _Adafruit_SSD1306_H_ )
    display.setTextColor( SSD1306_WHITE );
#elif defined( _Adafruit_SH110X_H_ )
    display.setTextColor( SH110X_WHITE );
#else
    #error "no display driver found"
#endif
    display.setFont( NULL );
    display.setTextSize( 1 );
    display.setCursor( 0, 0 );
    display.printf("start ...");
    display.display();

    displayinit = true;
}
/**
 * @brief get the width in pixel of a given string
 * 
 * @param fmt   pointer to a \0 termated string
 * @return int  width in pixel
 */
int display_get_string_width( const char *fmt ) {
    String msg = fmt;
    int16_t y=0,x=0,minx=0,miny=0;
    uint16_t maxx=SCREEN_WIDTH,maxy=SCREEN_HEIGHT;

    display.getTextBounds( msg, x, y, &minx, &miny, &maxx, &maxy );

    return( maxx - minx );
}

void display_loop( void ) {
    static uint64_t NextDisplayMillis = 0;
    /**
     * abort if display not init or active
     */
    if( !displayinit )
        return;
    if( !display_config.active )
        return;
    /**
     * on first run, set next display update time
     */
    if( NextDisplayMillis == 0 )
        NextDisplayMillis = millis();
    /**
     * if display update time coming, update
     */
    if( NextDisplayMillis <= millis() ) {
        NextDisplayMillis = millis() + display_config.refreshinterval * 1000l;
        /**
         * clear display
         */
        display.clearDisplay();
        display.setRotation( display_config.flip ? 2 : 0 );
        /**
         * crawl all display info config items
         */
        for( int i = 0 ; i < DISPLAY_MAX_INFO ; i++ ) {
            /**
             * if channel not used, get next
             */
            if( measure_get_channel_type( display_config.displayinfo[ i ].value_channel ) == NO_CHANNEL_TYPE )
                continue;
            /**
             * set fontsize
             */
            switch( display_config.displayinfo[ i ].fontsize ) {
                case 0:
                    display.setFont( NULL );
                    display.setTextSize( 1 );
                    break;
                case 1:
                    display.setFont( &FreeSans9pt7b );
                    display.setTextSize( 1 );
                    break;
                case 2:
                    display.setFont( &FreeSans18pt7b );
                    display.setTextSize( 1 );
                    break;
                default:
                    display.setFont( NULL );
                    display.setTextSize( 1 );
                    break;
            }
            /**
             * set position
             */
            display.setCursor( display_config.displayinfo[ i ].x, display_config.displayinfo[ i ].y );
            /**
             * get channel rms and unit
             */
            display.printf("%s%.2f%s", display_config.displayinfo[ i ].text, measure_get_channel_rms( display_config.displayinfo[ i ].value_channel ), measure_get_channel_report_unit( display_config.displayinfo[ i ].value_channel ) );
        }
        /**
         * push out to the display
         */
        display.display();
    }
}

void display_set_active( bool active ) {
    display_config.active = active;
}

bool display_get_active( void ) {
    return( display_config.active );
}

void display_set_sda_pin( int pin ) {
    display_config.sda = pin;
}

int display_get_sda_pin( void ) {
    return( display_config.sda );
}

void display_set_sck_pin( int pin ) {
    display_config.sck = pin;
}

int display_get_sck_pin( void ) {
    return( display_config.sck );
}

void display_set_flip( bool flip ) {
    display_config.flip = flip;
}

bool display_get_flip( void ) {
    return( display_config.flip );
}

int display_get_refresh_interval( void ) {
    return( display_config.refreshinterval );
}

void display_set_refresh_interval( int refresh_interval ) {
    display_config.refreshinterval = refresh_interval;
}

char *display_get_infotext_name( uint16_t channel ) {
    return( display_config.displayinfo[ channel ].name );
}

void display_set_infotext_name( uint16_t channel, char *name ) {
    strlcpy( display_config.displayinfo[ channel ].name, name, DISPLAY_MAX_INFO_TEXT_SIZE );
}

int display_get_infotext_x( uint16_t channel ) {
    return( display_config.displayinfo[ channel ].x );
}

void display_set_infotext_x( uint16_t channel, int x ) {
    display_config.displayinfo[ channel ].x = x;
}

int display_get_infotext_y( uint16_t channel ) {
    return( display_config.displayinfo[ channel ].y );
}

void display_set_infotext_y( uint16_t channel, int y) {
    display_config.displayinfo[ channel ].y = y;
}

int display_get_infotext_fontsize( uint16_t channel ) {
    return( display_config.displayinfo[ channel ].fontsize );
}

void display_set_infotext_fontsize( uint16_t channel, int fontsize ) {
    display_config.displayinfo[ channel ].fontsize = fontsize;
}

int display_get_infotext_value_channel( uint16_t channel ) {
    return( display_config.displayinfo[ channel ].value_channel );
}

void display_set_infotext_value_channel( uint16_t channel, uint16_t value_channel ) {
    display_config.displayinfo[ channel ].value_channel = value_channel;
}

char *display_get_infotext_text( uint16_t channel ) {
    return( display_config.displayinfo[ channel ].text );
}

void display_set_infotext_text( uint16_t channel, char *text ) {
    strlcpy( display_config.displayinfo[ channel ].text, text, DISPLAY_MAX_INFO_TEXT_SIZE );
}

void display_save_settings( void ) {
    display_config.save();
    display_init();
}