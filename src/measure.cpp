/**
 * @file measure.cpp
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
#include <FreeRTOS.h>
#include <driver/i2s.h>
#include <arduinoFFT.h>
#include <math.h>

#include "config/measure_config.h"
#include "measure.h"
#include "config.h"

extern "C" {
    #include "soc/syscon_reg.h"
    #include "soc/syscon_struct.h"
}

measure_config_t measure_config;
/*
 * map 8 real adc channels to adc virtual channels
 * -1 means not mapped
 * 
 * by example: channel 5 map to virtual channel 0, 6 to 1 and 7 to 2
 */ 
int8_t channelmapping[ MAX_ADC_CHANNELS ] = { CHANNEL_3, CHANNEL_NOP, CHANNEL_NOP, CHANNEL_4, CHANNEL_5, CHANNEL_0, CHANNEL_1, CHANNEL_2 };
/**
 * 
 */
struct groupconfig groupconfig[ MAX_GROUPS ] = {
    { "L1", true },
    { "L2", false },
    { "L3", false },
    { "all power", false },
    { "unused", false },
    { "unused", false }
};
/*
 * define virtual channel type and their mathematics
 */
struct channelconfig channelconfig[ VIRTUAL_CHANNELS ] = { 
    { "L1 current"          , AC_CURRENT        , 0  , 0.025989   , 0.0, 0.0, true , 0.0, 0, 0, 0.0, GET_ADC|CHANNEL_0, FILTER|0, BRK, BRK, BRK, BRK, BRK, BRK, BRK, BRK },
    { "L1 voltage"          , AC_VOLTAGE        , 30 , 0.324000   , 0.0, 0.0, true , 0.0, 0, 0, 0.0, GET_ADC|CHANNEL_3, FILTER|0, BRK, BRK, BRK, BRK, BRK, BRK, BRK, BRK },
    { "L1 power"            , AC_POWER          , 0  , 1.0        , 0.0, 0.0, false, 0.0, 3, 0, 0.0, SET_TO|1, MUL|CHANNEL_0, MUL|CHANNEL_1, MUL_RATIO|CHANNEL_0, MUL_RATIO|CHANNEL_1, ABS, BRK, BRK, BRK, BRK },
    { "L1 reactive power"   , AC_REACTIVE_POWER , 0  , 1.0        , 0.0, 0.0, false, 0.0, 3, 0, 0.0, SET_TO|1, MUL|CHANNEL_0, MUL|CHANNEL_1, MUL_RATIO|CHANNEL_0, MUL_RATIO|CHANNEL_1, PASS_NEGATIVE, MUL_REACTIVE|CHANNEL_1, ABS, NEG, BRK },
    { "L2 current"          , AC_CURRENT        , 0  , 0.025989   , 0.0, 0.0, true , 0.0, 0, 1, 0.0, GET_ADC|CHANNEL_1, FILTER|0, BRK, BRK, BRK, BRK, BRK, BRK, BRK, BRK },
    { "L2 voltage"          , AC_VOLTAGE        , 30 , 0.324000   , 0.0, 0.0, true , 0.0, 0, 1, 0.0, GET_ADC|CHANNEL_4, FILTER|0, BRK, BRK, BRK, BRK, BRK, BRK, BRK, BRK },
    { "L2 power"            , AC_POWER          , 0  , 1.0        , 0.0, 0.0, false, 0.0, 3, 1, 0.0, SET_TO|1, MUL|CHANNEL_2, MUL|CHANNEL_3, MUL_RATIO|CHANNEL_2, MUL_RATIO|CHANNEL_3, ABS, BRK, BRK, BRK, BRK },
    { "L2 reactive power"   , AC_REACTIVE_POWER , 0  , 1.0        , 0.0, 0.0, false, 0.0, 3, 1, 0.0, SET_TO|1, MUL|CHANNEL_2, MUL|CHANNEL_3, MUL_RATIO|CHANNEL_2, MUL_RATIO|CHANNEL_3, PASS_NEGATIVE, MUL_REACTIVE|CHANNEL_5, ABS, NEG, BRK },
    { "L3 current"          , AC_CURRENT        , 0  , 0.025989   , 0.0, 0.0, true , 0.0, 0, 2, 0.0, GET_ADC|CHANNEL_2, FILTER|0, BRK, BRK, BRK, BRK, BRK, BRK, BRK, BRK },
    { "L3 voltage"          , AC_VOLTAGE        , 30 , 0.324000   , 0.0, 0.0, true , 0.0, 0, 2, 0.0, GET_ADC|CHANNEL_5, FILTER|0, BRK, BRK, BRK, BRK, BRK, BRK, BRK, BRK },
    { "L3 power"            , AC_POWER          , 0  , 1.0        , 0.0, 0.0, false, 0.0, 3, 2, 0.0, SET_TO|1, MUL|CHANNEL_4, MUL|CHANNEL_5, MUL_RATIO|CHANNEL_4, MUL_RATIO|CHANNEL_5, ABS, BRK, BRK, BRK, BRK },
    { "L3 reactive power"   , AC_REACTIVE_POWER , 0  , 1.0        , 0.0, 0.0, false, 0.0, 3, 2, 0.0, SET_TO|1, MUL|CHANNEL_4, MUL|CHANNEL_5, MUL_RATIO|CHANNEL_4, MUL_RATIO|CHANNEL_5, PASS_NEGATIVE, MUL_REACTIVE|CHANNEL_9, ABS, NEG, BRK },
    { "all power"           , AC_POWER          , 0  , 1.0        , 0.0, 0.0, false, 0.0, 3, 3, 0.0, SET_TO|0, ADD|CHANNEL_0, ADD|CHANNEL_2, ADD|CHANNEL_4, BRK, BRK, BRK, BRK, BRK, BRK }
};

/* taske handle */
TaskHandle_t _MEASURE_Task;

volatile int TX_buffer = -1;
uint16_t buffer[ VIRTUAL_CHANNELS ][ numbersOfSamples ];
uint16_t buffer_fft[ VIRTUAL_CHANNELS ][ numbersOfFFTSamples ];
uint16_t buffer_probe[ VIRTUAL_CHANNELS ][ numbersOfSamples ];

double HerzvReal[ numbersOfSamples * 4 ];
double HerzvImag[ numbersOfSamples * 4 ];
double netfrequency_phaseshift, netfrequency_oldphaseshift;
double netfrequency;

static int measurement_valid = 3; /** @brief startcounter to prevent trash in first run */

void measure_Task( void * pvParameters ) {
    static uint64_t NextMillis = millis();

    log_i("Start Measurement Task on Core: %d", xPortGetCoreID() );

    measure_init();
    i2s_start(I2S_PORT);

    while( true ) {

        vTaskDelay( 1 );
    
        if ( millis() - NextMillis > DELAY ) {
            NextMillis += DELAY;
            measure_mes();
        }
    }
}
/**
 * 
 */
void measure_StartTask( void ) {
    xTaskCreatePinnedToCore(
                    measure_Task,               /* Function to implement the task */
                    "measure measurement Task", /* Name of the task */
                    10000,                      /* Stack size in words */
                    NULL,                       /* Task input parameter */
                    2,                          /* Priority of the task */
                    &_MEASURE_Task,             /* Task handle. */
                    _MEASURE_TASKCORE );        /* Core where the task should run */  
}
/*
 * 
 */
void measure_init( void ) {
    /**
     * load config from json 
     */
    measure_config.load();
    int sample_rate = ( samplingFrequency * measure_config.network_frequency / 2 ) + measure_config.samplerate_corr;
    /**
     * config i2s to capture data from internal adc
     */
    const i2s_config_t i2s_config = {
        .mode = i2s_mode_t(I2S_MODE_MASTER | I2S_MODE_RX | I2S_MODE_ADC_BUILT_IN ),
        .sample_rate = sample_rate,
        .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
        .channel_format = I2S_CHANNEL_FMT_ONLY_RIGHT,
        .communication_format = i2s_comm_format_t( I2S_COMM_FORMAT_I2S ),
        .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,     
        .dma_buf_count = VIRTUAL_ADC_CHANNELS * 4,                          
        .dma_buf_len = numbersOfSamples,
    };
    /**
     * set i2s config
     */
    esp_err_t err = i2s_driver_install(I2S_PORT, &i2s_config, 0, NULL);
    if ( err != ESP_OK ) { 
        log_e("Failed installing driver: %d", err );
        while ( true );
    }
    /**
     * set adc mode
     */
    err = i2s_set_adc_mode( ADC_UNIT_1, (adc1_channel_t)6 );
    if (err != ESP_OK ) {
        log_e("Failed installing driver: %d", err );
        while ( true );
    }

    i2s_stop( I2S_PORT );

    vTaskDelay( 100 );
    /**
     * 6 Items in pattern table
     */
    SYSCON.saradc_ctrl.sar1_patt_len = VIRTUAL_ADC_CHANNELS - 1;
    /**
     * [7:4] Channel
     * [3:2] Bit Width; 3=12bit, 2=11bit, 1=10bit, 0=9bit
     * [1:0] Attenuation; 3=11dB, 2=6dB, 1=2.5dB, 0=0dB
     */
    SYSCON.saradc_sar1_patt_tab[0] = 0x0f3f4f5f;
    SYSCON.saradc_sar1_patt_tab[1] = 0x6f7f0000;
    SYSCON.saradc_ctrl.sar_clk_div = 5;

    log_i("Measurement: I2S driver ready");

    netfrequency = measure_config.network_frequency;
}

/*
 * get as much adc buffer as possible in one second and calculate some stuff
 */
void measure_mes( void ) {
    int round = 0;                    /** @brief round counter */
    /**
     * clear sum for each channel
     */
    for( int i = 0 ; i < VIRTUAL_CHANNELS ; i++ )
        channelconfig[ i ].sum = 0.0;
    /**
     * get the current timer and calculate the exit time for capture ADC-buffer
     */
    uint64_t NextMillis = millis() + 1000l;
    /**
     * get numbersOfSamples * VIRTUAL_ADC_CHANNELS each round
     * one round is 40/33.3ms (depending on network freuency) or two network frecuency periods
     * after 950 ms calculate each channel value like Vrms/Irms or network frequency/paseshift
     */
    while( millis() < NextMillis ) {

        static float adc_sample[ VIRTUAL_CHANNELS ], last_adc_sample[ VIRTUAL_CHANNELS ];                  /** @brief runtime varibales for lowpass filter stuff */
        static float temp_adc_sample[ VIRTUAL_CHANNELS ];
        static float ac_filtered[ VIRTUAL_CHANNELS ], last_ac_filtered[ VIRTUAL_CHANNELS ];                /** @brief runtime varibales for lowpass filter stuff */
        static float dc_filtered[ VIRTUAL_CHANNELS ][ 64 ];                                                /** @brief runtime varibales for lowpass filter stuff */
        uint16_t *channel[ VIRTUAL_ADC_CHANNELS ];                                                          /** @brief channel pointer list */
        uint16_t adc_tempsamples[ numbersOfSamples ];                                                       /** @brief unsort sampelbuffer from ADC */
        uint16_t adc_samples[ VIRTUAL_ADC_CHANNELS ][ numbersOfSamples ];                                   /** @brief channel sorted samplebuffer */
        int phaseshift_0_degree;
        int phaseshift_90_degree;
        /**
         * create a channel pointer list for later and faster use
         */
        for( int i = 0 ; i < VIRTUAL_ADC_CHANNELS ; i++ ) {
            channel[ i ] = &adc_samples[ i ][ 0 ];
        }
        /**
         * get an chunk of data from adc and sort it in
         */
        for( int adc_chunck = 0 ; adc_chunck < VIRTUAL_ADC_CHANNELS ; adc_chunck++ ) {
            /**
             * get an ADC buffer 
             */
            size_t num_bytes_read = 0;
            esp_err_t err;
            err = i2s_read( I2S_PORT,
                            (char *)adc_tempsamples,      /* pointer to the adc_tempsample */
                            sizeof( adc_tempsamples ),    /* the doc says elements, but its bytes */
                            &num_bytes_read,              /* pointer to an size_t variable to store number of bytes read */
                            100 );      /* no timeout */
            /**
             * handle error
             */
            if ( err != ESP_OK ) {
                log_e("Error while reading DMA Buffer: %d", err );
                while( true );
            }/**
             * check blocksize
             */
            num_bytes_read /= 2;
            if ( num_bytes_read != numbersOfSamples ) {
                log_e("block size != numberOfSamples, num_bytes_read = %d", num_bytes_read );
                while( true );
            }
            /**
             * sort the ADC-buffer into virtual channels
             * adc sample buffer contains the following data scheme with 16bit words
             * 
             * [CH6][CH6][CH7][CH7][CH5][CH5][CH6][CH6][CH7].....
             *
             * each 16bit word contains the following bitscheme
             *
             * [15..12]     channel
             * [11..0]      12-bit sample
             */
            for( int i = 0 ; i < num_bytes_read ; i++ ) {
                /**
                 * get the right channel number from the sample and store it
                 */
                int8_t chan = ( adc_tempsamples[ i ] >> 12 ) & 0xf;
                /**
                 * assigns the sample to the right virtual channel by use channel pointer list
                 */
                if ( channelmapping[ chan ] != CHANNEL_NOP && chan < MAX_ADC_CHANNELS ) {
                    *channel[ channelmapping[ chan ] ] = adc_tempsamples[ i ] & 0x0fff;
                    channel[ channelmapping[ chan ] ]++;
                }
            }
        }
        /**
         * compute each sample for each virtual channel
         */
        for ( int n = 0; n < numbersOfSamples; n++ ) {
            for ( int i = 0 ; i < VIRTUAL_CHANNELS ; i++ ) {
                /**
                 * abort if group not active or channel not used
                 */
                if( !groupconfig[ channelconfig[ i ].group_id ].active || channelconfig[ i ].type == NO_CHANNEL_TYPE ) {
                    buffer[ i ][ n ] = 2048;
                    channelconfig[ i ].sum = 0.0;
                    adc_sample[ i ] = 0.0; 
                    continue;               
                }
                /**
                 * get right sample or compute it
                 */
                for( int operation = 0 ; operation < MAX_MICROCODE_OPS ; operation++ ) {
                    /**
                     * precalc the phaseshift from 360 degree to number of samples
                     * and prevent a channel clipping issue
                     */
                    phaseshift_0_degree = ( ( numbersOfSamples / 2 ) / 360.0 ) * ( ( channelconfig[ i ].phaseshift ) %360 );
                    phaseshift_0_degree = ( n + phaseshift_0_degree ) % numbersOfSamples;
                    if( phaseshift_0_degree == numbersOfSamples - 1 )
                        phaseshift_0_degree = 0;                    
                    phaseshift_90_degree = ( ( numbersOfSamples / 2 ) / 360.0 ) * ( ( channelconfig[ i ].phaseshift + 90 ) %360 );
                    phaseshift_90_degree = ( n + phaseshift_90_degree ) % numbersOfSamples;
                    if( phaseshift_90_degree == numbersOfSamples - 1 )
                        phaseshift_90_degree = 0;  
                    /**
                     * mask the channel out and store it in a separate variable for later use
                     * and check if op_channel valid
                     */
                    int op_channel = channelconfig[ i ].operation[ operation ] & ~OPMASK;
                    if( op_channel >= VIRTUAL_CHANNELS )
                        continue;
                    /**
                     * oparate the current channel operateion
                     */
                    switch( channelconfig[i].operation[ operation ] & OPMASK ) {
                        case ADD:                   adc_sample[ i ] += adc_sample[ op_channel ];
                                                    break;
                        case SUB:                   adc_sample[ i ] -= adc_sample[ op_channel ];
                                                    break;
                        case MUL:                   adc_sample[ i ] = adc_sample[ i ] * adc_sample[ op_channel ];
                                                    break;
                        case MUL_RATIO:             adc_sample[ i ] *= channelconfig[ op_channel ].ratio;
                                                    break;
                        case MUL_SIGN:              if( adc_sample[ op_channel ] > 0.0 )
                                                        adc_sample[ i ] *= 1.0;
                                                    else if( adc_sample[ op_channel ] < 0.0 )
                                                        adc_sample[ i ] *= -1.0;

                                                    break;
                        case MUL_REACTIVE:          temp_adc_sample[ i ] = buffer[ op_channel ][ phaseshift_90_degree ] - 2048.0;

                                                    if( adc_sample[ op_channel ] > 0.0 )
                                                        temp_adc_sample[ i ] =  temp_adc_sample[ i ] * 1.0;
                                                    else if( adc_sample[ op_channel ] < 0.0 )
                                                        temp_adc_sample[ i ] =  temp_adc_sample[ i ] * -1.0;                                                

                                                    if( temp_adc_sample[ i ] > 0.0 )
                                                        channelconfig[ i ].sign = 1.0;
                                                    else if( temp_adc_sample[ i ] < 0.0 )
                                                        channelconfig[ i ].sign = -1.0;

                                                    adc_sample[ i ] *= channelconfig[ i ].sign;

                                                    break;
                        case ABS:                   adc_sample[ i ] = fabs( adc_sample[ i ] );
                                                    break;
                        case NEG:                   adc_sample[ i ] = adc_sample[ i ] * -1.0;
                                                    break;
                        case PASS_NEGATIVE:         if( adc_sample[ i ] > 0.0 )
                                                        adc_sample[ i ] = 0.0;
                                                    break;
                        case PASS_POSITIVE:         if( adc_sample[ i ] < 0.0 )
                                                        adc_sample[ i ] = 0.0;
                                                    break;
                        case GET_ADC:               if( op_channel >= 0 && op_channel < VIRTUAL_ADC_CHANNELS )
                                                        adc_sample[ i ] = adc_samples[ op_channel ][ phaseshift_0_degree ] + channelconfig[ i ].offset;
                                                    else
                                                        continue;
                                                    break;
                        case SET_TO:                adc_sample[ i ] = op_channel;
                                                    break;
                        case FILTER:                switch( channelconfig[ i ].type ) {
                                                        case AC_CURRENT:
                                                        case AC_VOLTAGE:
                                                        case AC_POWER:
                                                        case AC_REACTIVE_POWER:
                                                            /**
                                                             * filter out DC component
                                                             */
                                                            ac_filtered[ i ] = ( 0.9989 ) * ( last_ac_filtered[ i ] + adc_sample[ i ] - last_adc_sample[ i ] );
                                                            last_ac_filtered[ i ] = ac_filtered[ i ];
                                                            last_adc_sample[ i ] = adc_sample[ i ];
                                                            adc_sample[ i ] = ac_filtered[ i ];
                                                            /**
                                                             * do simple mean value filtering, low pass filtering
                                                             */
                                                            if( op_channel ) {
                                                                int mul = 1;
                                                                if( op_channel <= 6 )
                                                                    mul = 1 << op_channel;
                                                                else
                                                                    mul = 1 << 6;
                                                                    
                                                                dc_filtered[ i ][ n % mul ] = adc_sample[ i ];
                                                                
                                                                adc_sample[ i ] = 0.0;
                                                                for( int a = 0; a < mul ; a++ ) 
                                                                    adc_sample[ i ] += dc_filtered[ i ][ a ];
                                                                adc_sample[ i ] /= mul;
                                                            }
                                                            else {
                                                                adc_sample[ i ] = adc_sample[ i ];
                                                            }
                                                            break;
                                                        case DC_CURRENT:
                                                        case DC_VOLTAGE:
                                                            if( op_channel ) {
                                                                int mul = 1;
                                                                if( op_channel <= 6 )
                                                                    mul = 1 << op_channel;
                                                                else
                                                                    mul = 1 << 6;
                                                                    
                                                                dc_filtered[ i ][ n % mul ] = adc_sample[ i ];
                                                                
                                                                adc_sample[ i ] = 0.0;
                                                                for( int a = 0; a < mul ; a++ ) 
                                                                    adc_sample[ i ] += dc_filtered[ i ][ a ];
                                                                adc_sample[ i ] /= mul;
                                                            }
                                                            else {
                                                                adc_sample[ i ] = adc_sample[ i ];
                                                            }
                                                            break;
                                                        default:
                                                            adc_sample[ i ] = adc_sample[ i ];
                                                            break;

                                                    }
                                                    break;
                        case NOP:                   break;
                        default:                    operation = MAX_MICROCODE_OPS;
                                                    break;
                    }
                }
                /**
                 * Store sum and data if channelgroup is active
                 */
                if ( channelconfig[ i ].type == AC_VOLTAGE && measure_get_channel_ratio( i ) > 5 )
                    buffer[ i ][ n ] = 2048;
                else
                    buffer[ i ][ n ] = ( adc_sample[ i ] + 2048 ) < 0.0 ? 0 : adc_sample[ i ] + 2048 ;
                
                switch( channelconfig[ i ].type ) {
                    case AC_CURRENT:
                    case AC_VOLTAGE:
                        if( channelconfig[ i ].true_rms )
                            channelconfig[ i ].sum += adc_sample[ i ] * adc_sample[ i ];
                        else 
                            channelconfig[ i ].sum += fabs( adc_sample[ i ] );
                        break;
                    case AC_POWER:
                    case AC_REACTIVE_POWER:
                    case DC_CURRENT:
                    case DC_VOLTAGE:
                    case DC_POWER:
                        if( channelconfig[ i ].true_rms )
                            channelconfig[ i ].sum += adc_sample[ i ] * adc_sample[ i ];
                        else 
                            channelconfig[ i ].sum += adc_sample[ i ];
                        break;
                    case NO_CHANNEL_TYPE:
                        break;
                }
            }
        }
        /**
         * get fill probebuffer if requensted
         */
        if ( TX_buffer != -1 ) {
            memcpy( &buffer_probe[0][0], &buffer[0][0] , sizeof( buffer ) );
            TX_buffer = -1;
        }
        /**
         * copy the first AC_VOLTAGE channel into fft buffer to get network frequency
         */
        if ( round < 4 ) {
            for( int i = 0 ; i < VIRTUAL_CHANNELS ; i++ ) {
                if( channelconfig[ i ].type == AC_VOLTAGE ) {
                    for( int sample = 0 ; sample < numbersOfSamples ; sample++ ) {
                        HerzvReal[ ( round * numbersOfSamples ) + sample ] = buffer[ i ][ sample ];
                        HerzvImag[ ( round * numbersOfSamples ) + sample ] = 0;
                    }
                    break;
                }
            }
        }
        /**
         * loop into next round and count it
         */
        round++;
    }
    /**
     * if netfrequency monitoring channel monitors voltage
     * else set netfrequency to 50 or 60hz
     */
    if ( channelconfig[ 1 ].type == AC_VOLTAGE && measure_get_channel_ratio( 1 ) <= 5.0 ) {
        /**
         * calculate the phaseshift between last phaseshift
         */
        arduinoFFT FFT = arduinoFFT( HerzvReal, HerzvImag, numbersOfSamples * 4, ( numbersOfSamples ) * measure_get_network_frequency() );
        FFT.Windowing( FFT_WIN_TYP_HAMMING, FFT_REVERSE);
        FFT.Compute( FFT_REVERSE );
        netfrequency_oldphaseshift = netfrequency_phaseshift;
        netfrequency_phaseshift = atan2( HerzvReal[ 8 ], HerzvImag[ 8 ] ) * ( 180.0 / PI ) + 180;
        /**
         * prevent chaotic phaseshift values higher then 180 degrees
         */
        if ( ( netfrequency_phaseshift - netfrequency_oldphaseshift ) < 180 && ( netfrequency_phaseshift - netfrequency_oldphaseshift ) > -180 ) {
            static float netfrequency_filter[ 16 ];
            static int index = 0;
            static bool netfrequency_firstrun = true;

            if( netfrequency_firstrun ) {
                for( int i = 0 ; i < 16 ; i++ )
                    netfrequency_filter[ i ] = measure_get_network_frequency();
                netfrequency_firstrun = false;
            }

            netfrequency_filter[ index ] = ( netfrequency_phaseshift - netfrequency_oldphaseshift ) * ( ( 1.0f / PI ) / 90 ) + measure_get_network_frequency();
            
            netfrequency = 0.0;
            for( int i = 0 ; i < 16 ; i++ )
                netfrequency += netfrequency_filter[ i ];
            netfrequency /= 16.0;

            if( index < 16 )
                index++;
            else
                index = 0;
        }
    }
    else {
        netfrequency = measure_get_network_frequency();
    }

    for ( int i = 0 ; i < VIRTUAL_CHANNELS ; i++ ) {
        /*
         * Calibration coeficients applied. 
         */
        switch( channelconfig[ i ].type ) {
            case AC_VOLTAGE:
            case AC_CURRENT:
            case DC_VOLTAGE:
            case DC_CURRENT:
            case DC_POWER:
            case AC_POWER:
            case AC_REACTIVE_POWER:
                if ( channelconfig[ i ].ratio > 5 ) {
                    channelconfig[ i ].rms = channelconfig[ i ].ratio;                
                }
                else {
                    if( channelconfig[ i ].true_rms )
                        channelconfig[ i ].rms = channelconfig[ i ].ratio * sqrt( channelconfig[ i ].sum / ( numbersOfSamples * round ) );
                    else 
                        channelconfig[ i ].rms = channelconfig[ i ].ratio * ( channelconfig[ i ].sum / ( numbersOfSamples * round ) );
                }
                break;
            case NO_CHANNEL_TYPE:
                break;
        }

        if ( measurement_valid > 0 )
            channelconfig[ i ].rms = 0.0;

        if ( measurement_valid > 0 )
            measurement_valid--;
    }
}

int measure_get_samplerate_corr( void ) {
    return( measure_config.samplerate_corr );
}

void measure_set_samplerate_corr( int samplerate_corr ) {
    measure_config.samplerate_corr = samplerate_corr;

    esp_err_t err;
    err = i2s_set_sample_rates( I2S_PORT, ( samplingFrequency * measure_config.network_frequency / 2 ) + measure_config.samplerate_corr );
    if ( err != ESP_OK ) {
        log_e("Failed set samplerate: %d", err);
        while ( true );
    }
}

float measure_get_network_frequency( void ) {
    return( measure_config.network_frequency );
}

void measure_set_network_frequency( float network_frequency ) {
    if( network_frequency >= 16.0 && network_frequency <= 120.0 ) {
        measure_config.network_frequency = network_frequency;
        esp_err_t err;
        err = i2s_set_sample_rates( I2S_PORT, ( samplingFrequency * measure_config.network_frequency / 2 ) + measure_config.samplerate_corr );
        if ( err != ESP_OK ) {
            log_e("Failed set samplerate: %d", err);
            while ( true );
        }
    }
}

uint16_t * measure_get_buffer( void ) {
    TX_buffer = 0;
    while ( TX_buffer == 0 ) {}
    return( &buffer_probe[0][0] ); 
}

uint16_t * measure_get_fft( void ) {
  double vReal[ numbersOfFFTSamples * 2 ];
  double vImag[ numbersOfFFTSamples * 2 ];
  arduinoFFT FFT = arduinoFFT( vReal, vImag, numbersOfFFTSamples * 2, samplingFrequency * measure_get_network_frequency() / 4 );

    for ( int channel = 0 ; channel < VIRTUAL_CHANNELS ; channel++ ) {
        for ( int i = 0 ; i < numbersOfFFTSamples * 2 ; i++ ) {
            vReal[i] = buffer_probe[channel][ (i*6)%numbersOfSamples ];
            vImag[i] = 0;
        }

        FFT.Windowing( FFT_WIN_TYP_RECTANGLE, FFT_REVERSE);
        FFT.Compute( FFT_REVERSE );
        FFT.ComplexToMagnitude();
        
        for( int i = 1 ; i < numbersOfFFTSamples; i++ )
            buffer_fft[channel][i] = vReal[i];
    }
    return( &buffer_fft[0][0] );
}

double measure_get_max_freq( void ) {
    return( netfrequency );
}

float measure_get_channel_rms( int channel ) {
    return( channelconfig[ channel ].rms * measure_get_channel_report_exp_mul( channel ) );
}

char *measure_get_channel_name( uint16_t channel ) {
    if ( channel >= VIRTUAL_CHANNELS )
        return( 0 );

    return( channelconfig[ channel ].name );    
}

void measure_set_channel_name( uint16_t channel, char *name ) {
    if ( channel >= VIRTUAL_CHANNELS )
        return;

    strlcpy( channelconfig[ channel ].name, name, sizeof( channelconfig[ channel ].name ) );
}

bool measure_get_channel_true_rms( int channel ) {
    return( channelconfig[ channel ].true_rms );
}

void measure_set_channel_true_rms( int channel, bool true_rms ) {
    channelconfig[ channel ].true_rms = true_rms;
}

channel_type_t measure_get_channel_type( uint16_t channel ){
    if ( channel >= VIRTUAL_CHANNELS )
        return( NO_CHANNEL_TYPE );
        
    return( channelconfig[ channel ].type );
}

void measure_set_channel_type( uint16_t channel, channel_type_t type ){
    if ( channel >= VIRTUAL_CHANNELS )
        return;
        
    channelconfig[ channel ].type = type;
}

double measure_get_channel_offset( uint16_t channel ){
    if ( channel >= VIRTUAL_CHANNELS )
        return( 0.0 );
        
    return( channelconfig[ channel ].offset );
}

void measure_set_channel_offset( uint16_t channel, double channel_offset ) {
    if ( channel >= VIRTUAL_CHANNELS )
        return;

    channelconfig[ channel ].offset = channel_offset;
}

double measure_get_channel_ratio( uint16_t channel ){
    if ( channel >= VIRTUAL_CHANNELS )
        return( 0.0 );
        
    return( channelconfig[ channel ].ratio );
}

void measure_set_channel_ratio( uint16_t channel, double channel_ratio ) {
    if ( channel >= VIRTUAL_CHANNELS )
        return;

    channelconfig[ channel ].ratio = channel_ratio;
}

int measure_get_channel_phaseshift( uint16_t channel ) {
    if ( channel >= VIRTUAL_CHANNELS )
        return( 0 );

    return( channelconfig[ channel ].phaseshift % 360 );
}

void measure_set_channel_phaseshift( uint16_t channel, int value ) {
    if ( channel >= VIRTUAL_CHANNELS )
        return;

    channelconfig[ channel ].phaseshift = ( value % 360 );
}

const char *measure_get_group_name( uint16_t group ) {
    if ( group >= MAX_GROUPS )
        return( "" );

    return( (const char*) groupconfig[ group ].name );
}

void measure_set_group_name( uint16_t group, const char *name ) {
    if ( group >= MAX_GROUPS )
        return;

    strlcpy( groupconfig[ group ].name, name, sizeof( groupconfig[ group ].name ) );
}

bool measure_get_group_active( uint16_t group ) {
    if ( group >= MAX_GROUPS )
        return( false );
        
    return( groupconfig[ group ].active );
}

void measure_set_group_active( uint16_t group, bool active ) {
    if ( group >= MAX_GROUPS )
        return;

    groupconfig[ group ].active = active;
}

int measure_get_channel_group_id( uint16_t channel ) {
    if ( channel >= VIRTUAL_CHANNELS )
        return( 0 );

    return( channelconfig[ channel ].group_id );
}

void measure_set_channel_group_id( uint16_t channel, int group_id ) {
    if ( channel >= VIRTUAL_CHANNELS )
        return;

    channelconfig[ channel ].group_id = group_id;
}

int measure_get_channel_group_id_entrys( int group_id ) {
    int groupd_id_entrys = 0;
    
    for( int i = 0 ; i < VIRTUAL_CHANNELS ; i++ )
        if( channelconfig[ i ].group_id == group_id && channelconfig[ i ].type != NO_CHANNEL_TYPE )
            groupd_id_entrys++;

    return( groupd_id_entrys );
}

int measure_get_channel_group_id_entrys_with_type( int group_id, int type ) {
    int groupd_id_entrys = 0;
    
    for( int i = 0 ; i < VIRTUAL_CHANNELS ; i++ )
        if( channelconfig[ i ].group_id == group_id && channelconfig[ i ].type == type )
            groupd_id_entrys++;

    return( groupd_id_entrys );
}

int measure_get_channel_with_group_id_and_type( int group_id, int type ) {
    for( int i = 0 ; i < VIRTUAL_CHANNELS ; i++ )
        if( channelconfig[ i ].group_id == group_id && channelconfig[ i ].type == type )
            return( i );

    return( -1 );
}

int measure_get_channel_report_exp( uint16_t channel ) {
    if ( channel >= VIRTUAL_CHANNELS )
        return( false );
        
    return( channelconfig[ channel ].report_exp );
}

float measure_get_channel_report_exp_mul( uint16_t channel ) {
    if ( channel >= VIRTUAL_CHANNELS )
        return( 1.0 );

    return( 1.0 / pow( 10, channelconfig[ channel ].report_exp ) );
}

void measure_set_channel_report_exp( uint16_t channel, int report_exp ) {
    if ( channel >= VIRTUAL_CHANNELS )
        return;

    channelconfig[ channel ].report_exp = report_exp;
}

const char *measure_get_channel_report_unit( uint16_t channel ) {
    switch( channelconfig[ channel ].type ) {
        case AC_CURRENT:
        case DC_CURRENT:
            if( channelconfig[ channel ].report_exp == -3 )
                    return( "mA" );
            else if( channelconfig[ channel ].report_exp == 0 )
                    return( "A" );
            else if( channelconfig[ channel ].report_exp == 3 )
                    return( "kA" );
            return( "-" );
        case AC_VOLTAGE:
        case DC_VOLTAGE:
            if( channelconfig[ channel ].report_exp == -3 )
                    return( "mV" );
            else if( channelconfig[ channel ].report_exp == 0 )
                    return( "V" );
            else if( channelconfig[ channel ].report_exp == 3 )
                    return( "kV" );
            return( "-" );
        case AC_POWER:
        case DC_POWER:
            if( channelconfig[ channel ].report_exp == -3 )
                    return( "mW" );
            else if( channelconfig[ channel ].report_exp == 0 )
                    return( "W" );
            else if( channelconfig[ channel ].report_exp == 3 )
                    return( "kW" );
            return( "-" );
        case AC_REACTIVE_POWER:
            if( channelconfig[ channel ].report_exp == -3 )
                    return( "mVAr" );
            else if( channelconfig[ channel ].report_exp == 0 )
                    return( "VAr" );
            else if( channelconfig[ channel ].report_exp == 3 )
                    return( "kVAr" );
            return( "-" );
        default:
            return( "-" );
    }
}

bool measure_get_measurement_valid( void ) {
    if( measurement_valid == 0 )
        return( true );
    return( false );
}

void measure_set_measurement_invalid( int sec ) {
    measurement_valid = sec;
}


uint8_t * measure_get_channel_opcodeseq( uint16_t channel ) {
    if ( channel >= VIRTUAL_CHANNELS )
        return( NULL );
    return( channelconfig[ channel ].operation );
}

char * measure_get_channel_opcodeseq_str( uint16_t channel, uint16_t len, char *dest ) {
    char microcode_tmp[ 3 ] = "";
    uint8_t *opcode = channelconfig[ channel ].operation;
    *dest = '\0';

    if ( channel >= VIRTUAL_CHANNELS )
        return( NULL );

    for( int a = 0 ; a < MAX_MICROCODE_OPS ; a++ ) {
        snprintf( microcode_tmp, sizeof( microcode_tmp ), "%02x", *opcode );
        strncat( dest, microcode_tmp, len );
        opcode++;
    }
    return( dest );
}

void measure_save_settings( void ) {
    measure_config.save();
}

void measure_set_channel_opcodeseq( uint16_t channel, uint8_t *value ) {
    char microcode[ VIRTUAL_CHANNELS * 3 ] = "";
    char microcode_tmp[ 3 ] = "";
    uint8_t *opcode = value;

    if ( channel >= VIRTUAL_CHANNELS )
        return;

    if( channel >= CHANNEL_END )
        return;

    memcpy( channelconfig[ channel ].operation, value, MAX_MICROCODE_OPS );

    for( int a = 0 ; a < MAX_MICROCODE_OPS ; a++ ) {
        snprintf( microcode_tmp, sizeof( microcode_tmp ), "%02x", *opcode );
        strncat( microcode, microcode_tmp, sizeof( microcode ) );
        opcode++;
    }
}

void measure_set_channel_opcodeseq_str( uint16_t channel, const char *value ) {
    char spanset[] = "0123456789ABCDEFabcdef";
    char *ptr = (char *)value;
    uint8_t opcode_binary = 0;
    int opcode_pos = 0;

    if ( channel >= VIRTUAL_CHANNELS )
        return;

    if( channel >= CHANNEL_END )
        return;
    /**
     * check string for illigal format
     */
    while( *ptr ) {
        /**
         * get first nibble of opcode
         */
        ptr = strpbrk( ptr, spanset );
        if ( !ptr ) {
            log_e("abort, wrong format");
            break;
        }
        opcode_binary = ( ( *ptr <= '9') ? *ptr - '0' : ( *ptr & 0x7) + 9 ) << 4;
        ptr++;
        /**
         * get 2'nd nibble of opcode
         */
        ptr = strpbrk( ptr, spanset );
        if ( !ptr ) {
            log_e("abort, wrong format");
            break;
        }
        opcode_binary = opcode_binary + ( ( *ptr <= '9') ? *ptr - '0' : ( *ptr & 0x7) + 9 );
        ptr++;
        /**
         * check if we have space for next opcode
         */
        if ( opcode_pos < MAX_MICROCODE_OPS ) {
            channelconfig[ channel ].operation[ opcode_pos ] = opcode_binary;
            opcode_pos++;
        }
        else {
            log_e("no more space for opcodes");
            break;
        }
    }
    return;
}
