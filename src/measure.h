/****************************************************************************
 *            measure.h
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

/**
 *
 * \author Dirk Broßwick
 *
 */
#ifndef _MEASURE_H

    #define _MEASURE_H

    #define MAX_ADC_CHANNELS      8
    #define VIRTUAL_ADC_CHANNELS  6
    #define VIRTUAL_CHANNELS      7

    #define MAX_MICROCODE_OPS     8

    #define numbersOfSamples      256
    #define numbersOfFFTSamples   32
    #define samplingFrequency     numbersOfSamples*VIRTUAL_ADC_CHANNELS
    #define DELAY                 1000
    #define I2S_PORT              I2S_NUM_0

    #define OPMASK 0xf0

    struct channelconfig {
        uint8_t type;
        int16_t phaseshift;
        uint8_t operation[ MAX_MICROCODE_OPS ];
    };

    enum {
        CHANNEL_NOP = -1,
        CURRENT,
        VIRTUALCURRENT,
        VOLTAGE,
        VIRTUALVOLTAGE,
        POWER,
        VIRTUALPOWER
    };

    enum {
        NOP = 0x00,
        ADD = 0x10,
        SUB = 0x20,
        MUL = 0x30,
        DIV = 0x40,
        GET_ADC = 0x50,
        SET_ZERO = 0x60,
        FILTER = 0x70,
        NOFILTER = 0x80,
        STORE_INTO_BUFFER = 0x90,
        STORE_SQUARE_SUM = 0xa0,
        STORE_SUM = 0xb0,
        DIV_4096 = 0xc0,
        NEG = 0xd0
    };

    enum {
        CHANNEL_0,
        CHANNEL_1,
        CHANNEL_2,
        CHANNEL_3,
        CHANNEL_4,
        CHANNEL_5,
        CHANNEL_6,
        CHANNEL_7,
        CHANNEL_8,
        CHANNEL_9,
        CHANNEL_10,
        CHANNEL_11,
        CHANNEL_12,
        CHANNEL_13,
        CHANNEL_14,
        CHANNEL_15,
    };

    /**
     * @brief 
     */
    void measure_init( void );
    /**
     * @brief
     */
    void measure_mes( void );
    /**
     * @brief set phaseshift correction value for all voltage channels in numbers of samples
     * @param   corr    correction value in sample
     * @return  0 if ok or failed
     */
    int measure_set_phaseshift( int corr );
    /**
     * @brief set the samplerate correction value in numbers of sample 
     * @param   corr    correction value in numbers of samples
     * @return  0 if ok or failed
     * @note it is very import for precise network frequency to calibrate
     * the samplerate with this value
     */
    int measure_set_samplerate( int corr );
    uint16_t * measure_get_buffer( void );
    uint16_t * measure_get_fft( void );
    double measure_get_max_freq( void );
    float measure_get_Irms( int line );
    float measure_get_Vrms( int line );
    float measure_get_power( int line );
    float measure_get_Iratio( void );
    void measure_StartTask( void );
    void measure_StartSignalGenTask( void );
    void measure_Task( void * pvParameters );
    void measure_SignalGenTask( void * pvParameters );
    uint8_t measure_get_channel_type( uint16_t channel );
    void measure_set_channel_type( uint16_t channel, uint8_t value );
    int16_t measure_get_channel_phaseshift( uint16_t channel );
    void measure_set_channel_phaseshift( uint16_t channel, int16_t value );
    uint8_t * measure_get_channel_opcodeseq( uint16_t channel );
    char * measure_get_channel_opcodeseq_str( uint16_t channel, uint16_t len, char *dest );
    void measure_set_channel_opcodeseq( uint16_t channel, uint8_t *value );
    void measure_set_channel_opcodeseq_str( uint16_t channel, const char *value );

#endif // _MEASURE_H
