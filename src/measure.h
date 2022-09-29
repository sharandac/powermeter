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

    #define MAX_ADC_CHANNELS        8
    #define VIRTUAL_ADC_CHANNELS    6
    #define VIRTUAL_CHANNELS        13

    #define MAX_GROUPS              6
    #define MAX_MICROCODE_OPS       10

    #define numbersOfSamples        256
    #define numbersOfFFTSamples     32
    #define samplingFrequency       numbersOfSamples*VIRTUAL_ADC_CHANNELS
    #define DELAY                   1000
    #define I2S_PORT                I2S_NUM_0

    #define OPMASK 0xf0
    /**
     * @brief channel type enum
     */
    enum {
        AC_CURRENT = 0,                     /** @brief measured AC current */
        AC_VOLTAGE,                         /** @brief measured AV voltage */
        DC_CURRENT,                         /** @brief measured AC current */
        DC_VOLTAGE,                         /** @brief measured AV voltage */
        AC_POWER,                           /** @brief measured power */
        AC_REACTIVE_POWER,                  /** @brief measured power */
        DC_POWER,                           /** @brief measured power */
        CHANNEL_NOT_USED,                   /** @brief set channel to zero */
        CHANNEL_TYPE_END
    };
    /**
     * @brief opcode enum
     */
    enum {
        BRK = 0x00,                         /** @brief no operation */
        ADD = 0x10,                         /** @brief add value from channel */
        SUB = 0x20,                         /** @brief subtract value from channel */
        MUL = 0x30,                         /** @brief multiply value from channel */
        MUL_RATIO = 0x80,                   /** @brief multiply mit ratio from channel */
        MUL_SIGN = 0x90,                    /** @brief store value into buffer */
        MUL_REACTIVE = 0xb0,                /** @brief multiply with reactive sign from channel */
        ABS = 0xa0,                         /** @brief abs */
        NEG = 0xd0,                         /** @brief change sign of a value */
        PASS_NEGATIVE = 0xe0,                /** @brief only pass negative values, otherwise set to zero */
        PASS_POSITIVE = 0xf0,                /** @brief only pass negative values, otherwise set to zero */
        GET_ADC = 0x50,                     /** @brief get value from adc channel */
        SET_TO = 0x60,                      /** @brief set value to zero */
        FILTER = 0x70,                      /** @brief filter value */
        OPCODE_END
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
        CHANNEL_END
    };
    /**
     * @brief group config structure
     */
    struct groupconfig {
        char        name[32];                           /** @brief group name */
        bool        active;                             /** @brief group output active/inactive */
    };
    /**
     * @brief channel config structure
     */
    struct channelconfig {
        char        name[32];                           /** @brief channel name */
        int         type;                               /** @brief channel type */
        int         phaseshift;                         /** @brief channel adc phaseshift */
        float       ratio;                              /** @brief channel ratio */
        float       offset;                             /** @brief channel offset */
        float       rms;                                /** @brief channel rms */
        float       sum;                                /** @brief channel sum */
        int         report_exp;                         /** @brief channel report exponent */
        int         group_id;                           /** @brief channel group ID for output groups */
        float       sign;                               /** @brief channel reactive power sign */
        uint8_t     operation[ MAX_MICROCODE_OPS ];     /** @brief opcode sequence */
    };
    /**
     * @brief measurement init function
     */
    void measure_init( void );
    /**
     * @brief
     */
    void measure_mes( void );
    void measure_save_settings( void );
    /**
     * @brief set phaseshift correction value for all voltage channels in numbers of samples
     * @param   corr    correction value in sample
     * @return  0 if ok or failed
     */
    int measure_set_phaseshift( int corr );
    /**
     * @brief set the samplerate correction value in numbers of sample 
     * @param   corr    correction value in numbers of samples
     * @note it is very import for precise network frequency to calibrate
     * the samplerate with this value
     */
    void measure_set_samplerate_corr( int samplerate_corr );
    /**
     * @brief get samplerate corr value
     * 
     * @return int 
     */
    int measure_get_samplerate_corr( void );
    /**
     * @brief get voltage frequency
     * 
     * @return int 
     */
    int measure_get_network_frequency( void );
    /**
     * @brief set voltage frequency between 50 and 60Hz
     * 
     * @param voltage_frequency 
     */
    void measure_set_network_frequency( int voltage_frequency );
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
     * @brief start the measurement task
     */
    void measure_StartTask( void );
    /**
     * @brief get the current net frequency
     * 
     * @return double   50.0/60.0 if not measured or the current value in Hz
     */
    double measure_get_max_freq( void );
    /**
     * @brief get the rms from a given channel
     * 
     * @param channel 
     * @return float 
     */
    float measure_get_channel_rms( int channel );
    /**
     * @brief get the name of a given channel
     * 
     * @param channel   channel
     * @return char* 
     */
    char *measure_get_channel_name( uint16_t channel );
    /**
     * @brief  set the name of a given channel
     * 
     * @param channel   channel
     * @param name      pointer to a channel name string
     */
    void measure_set_channel_name( uint16_t channel, char *name );
    /**
     * @brief get the channel type for a channel
     * 
     * @return uint8_t 
     */
    int measure_get_channel_type( uint16_t channel );
    /**
     * @brief set the channel type for a channel
     * 
     * @param channel 
     * @return uint8_t 
     */
    void measure_set_channel_type( uint16_t channel, int value );
    /**
     * @brief get the channel offset
     * 
     * @param channel 
     * @return double 
     */
    double measure_get_channel_offset( uint16_t channel );
    /**
     * @brief set the offset for a channel
     * 
     * @param channel 
     * @param channel_offset
     */    
    void measure_set_channel_offset( uint16_t channel, double channel_offset );
    /**
     * @brief get channel ratio
     * 
     * @param channel 
     * @return double 
     */
    double measure_get_channel_ratio( uint16_t channel );
    /**
     * @brief set channel radio
     * 
     * @param channel 
     * @param channel_ratio 
     */
    void measure_set_channel_ratio( uint16_t channel, double channel_ratio );
    /**
     * @brief get the current phaseshift for a given channel
     * 
     * @param channel 
     * @return int16_t 
     */
    int measure_get_channel_phaseshift( uint16_t channel );
    /**
     * @brief set the phaseshift for a given channel
     * 
     * @param channel       channel
     * @param value         phaseshift in sample
     */
    void measure_set_channel_phaseshift( uint16_t channel, int value );
    /**
     * @brief get group name
     * 
     * @param group 
     * @return const char* 
     */
    const char *measure_get_group_name( uint16_t group );
    /**
     * @brief set group name
     * 
     * @param group 
     * @param name 
     */
    void measure_set_group_name( uint16_t group, const char *name );
    /**
     * @brief get group active/inactive
     * 
     * @param group 
     * @return true 
     * @return false 
     */
    bool measure_get_group_active( uint16_t group );
    /**
     * @brief set group active/inactive
     * 
     * @param group 
     * @param active 
     */
    void measure_set_group_active( uint16_t group, bool active );
    /**
     * @brief get the group id for a given channel
     * 
     * @param channel       channel
     * @return uint16_t 
     */
    int measure_get_channel_group_id( uint16_t channel );
    /**
     * @brief set the group id for a given channel
     * 
     * @param channel       channel
     * @param groupID       group id
     */
    void measure_set_channel_group_id( uint16_t channel, int group_id );
    /**
     * @brief get the numbers of channels with a given group id
     * 
     * @param group_id      group id
     * @return int          number of channel with group id
     */
    int measure_get_channel_group_id_entrys( int group_id );
    /**
     * @brief get the numbers of channels with a given group id and type
     * 
     * @param group_id      group id
     * @param type          channel type
     * @return int          number of channel with group id
     */
    int measure_get_channel_group_id_entrys_with_type( int group_id, int type );
    /**
     * @brief get the numbers of channels with a given group id and type
     * 
     * @param group_id      group id
     * @param type          channel type
     * @return int          number of channel with group id and type or -1
     */
    int measure_get_channel_with_group_id_and_type( uint16_t group_id, int type );
    int measure_get_channel_report_exp( uint16_t channel );
    float measure_get_channel_report_exp_mul( uint16_t channel );
    void measure_set_channel_report_exp( uint16_t channel, int report_in );
    const char *measure_get_channel_report_unit( uint16_t channel );
    bool measure_get_measurement_valid( void );
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
