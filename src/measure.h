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

    /**
     * @brief channel config structure
     * 
     */
    struct channelconfig {
        uint8_t type;                               /** @brief channel type */
        int16_t phaseshift;                         /** @brief phaseshift */
        uint8_t operation[ MAX_MICROCODE_OPS ];     /** @brief opcode sequence */
    };

    /**
     * @brief channel type enum
     */
    enum {
        CURRENT = 0,                        /** @brief measured current */
        VIRTUALCURRENT,                     /** @brief calculated virtual current */
        VOLTAGE,                            /** @brief measured voltage */
        VIRTUALVOLTAGE,                     /** @brief calculated virtual voltage */
        POWER,                              /** @brief measured power */
        VIRTUALPOWER                        /** @brief calculated virtual power */
    };

    /**
     * @brief opcode enum
     */
    enum {
        NOP = 0x00,                         /** @brief no operation */
        ADD = 0x10,                         /** @brief add value from channel */
        SUB = 0x20,                         /** @brief subtract value from channel */
        MUL = 0x30,                         /** @brief multiply value from channel */
        DIV = 0x40,                         /** @brief divide value from channel */
        GET_ADC = 0x50,                     /** @brief get value from adc channel */
        SET_ZERO = 0x60,                    /** @brief set value to zero */
        FILTER = 0x70,                      /** @brief filter value */
        NOFILTER = 0x80,                    /** @brief no filter value */
        STORE_INTO_BUFFER = 0x90,           /** @brief store value into buffer */
        STORE_SQUARE_SUM = 0xa0,            /** @brief square value and add it to sum */
        STORE_SUM = 0xb0,                   /** @brief store sum value */
        DIV_4096 = 0xc0,                    /** @brief divide value by 4096 */
        NEG = 0xd0                          /** @brief change sign of a value */
    };

    /**
     * @brief channel enum
     */
    enum {
        CHANNEL_NOP = -1,
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
     * @brief measurement init function
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
    /**
     * @brief get the current sample buffer with a size of  VIRTUAL_CHANNELS * numbersOfSamples
     * 
     * @return uint16_t*    pointer to a uint16_t [VIRTUAL_CHANNELS][numbersOfSamples] array
     */
    uint16_t * measure_get_buffer( void );
    /**
     * @brief get the current fft buffer with a size of VIRTUAL_CHANNELS * numbersOfFFTSamples;
     * 
     * @return uint16_t*    pointer to a uint16_t [ VIRTUAL_CHANNELS ][ numbersOfFFTSamples ] array
     */
    uint16_t * measure_get_fft( void );
    /**
     * @brief get the current net frequency
     * 
     * @return double   50.0/60.0 if not measured or the current value in Hz
     */
    double measure_get_max_freq( void );
    /**
     * @brief get the Irms from a given line
     * 
     * @param line 
     * @return float 
     */
    float measure_get_Irms( int line );
    /**
     * @brief get the Vrms from a given line
     * 
     * @param line 
     * @return float 
     */
    float measure_get_Vrms( int line );
    /**
     * @brief get the power from a given line
     * 
     * @param line 
     * @return float 
     */
    float measure_get_power( int line );
    /**
     * @brief get the current Iratio
     * 
     * @return float 
     */
    float measure_get_Iratio( void );
    /**
     * @brief start the measurement task
     * 
     */
    void measure_StartTask( void );
    /**
     * @brief get the channel type for a channel
     * 
     * @return uint8_t 
     */
    uint8_t measure_get_channel_type( uint16_t channel );
    /**
     * @brief set the channel type for a channel
     * 
     * @param channel 
     * @return uint8_t 
     */
    void measure_set_channel_type( uint16_t channel, uint8_t value );
    /**
     * @brief get the current phaseshift for a given channel
     * 
     * @param channel 
     * @return int16_t 
     */
    int16_t measure_get_channel_phaseshift( uint16_t channel );
    /**
     * @brief set the phaseshift for a given channel
     * 
     * @param channel       
     * @param value         phaseshift in sample
     */
    void measure_set_channel_phaseshift( uint16_t channel, int16_t value );
    /**
     * @brief get the opcode dequence for a given channel
     * 
     * @param channel 
     * @return uint8_t*     pointer to a opcode sequnce with a size of MAX_MICROCODE_OPS
     */
    uint8_t * measure_get_channel_opcodeseq( uint16_t channel );
    /**
     * @brief get a opcode sequence as char array terminate by a tero value
     * 
     * @param channel       channel to get a opcode sequence
     * @param len           max size in len bytes
     * @param dest          pointer to a char array to store the opcode sequence
     * @return char*        NULL if failed or a valid pointer
     */
    char * measure_get_channel_opcodeseq_str( uint16_t channel, uint16_t len, char *dest );
    /**
     * @brief set the opcode sequence for a given channel
     * 
     * @param channel       a give channel
     * @param value         pointer to a opcode sequence as byte array with a size of MAX_MICROCODE_OPS
     */
    void measure_set_channel_opcodeseq( uint16_t channel, uint8_t *value );
    /**
     * @brief set the opcode sequence for a given channel from a string
     * 
     * @param channel       a given channel
     * @param value         pointer to a opcode sequence as char array terminate with zero
     */
    void measure_set_channel_opcodeseq_str( uint16_t channel, const char *value );

#endif // _MEASURE_H
