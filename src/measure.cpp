/****************************************************************************
 *            measure.cpp
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
#include <FreeRTOS.h>
#include <driver/periph_ctrl.h>
#include <driver/timer.h>
#include <driver/i2s.h>
#include <driver/ledc.h>
#include <arduinoFFT.h>
#include <math.h>

#include "measure.h"
#include "config.h"

extern "C" {
    #include "soc/syscon_reg.h"
    #include "soc/syscon_struct.h"
}

/*
 * map 8 real adc channels to adc virtual channels
 * -1 means not mapped
 * 
 * by example: channel 5 map to virtual channel 0, 6 to 1 and 7 to 2
 */ 
int8_t channelmapping[ MAX_ADC_CHANNELS ] = { CHANNEL_3, CHANNEL_NOP, CHANNEL_NOP, CHANNEL_4, CHANNEL_5, CHANNEL_0, CHANNEL_1, CHANNEL_2 };
/*
 * define virtual channel type and their mathematics
 */
struct channelconfig channelconfig[ VIRTUAL_CHANNELS ] = { 
    { CURRENT, 0, GET_ADC|CHANNEL_0, NOP, NOP, NOP, NOP, FILTER, STORE_INTO_BUFFER|CHANNEL_0, STORE_SQUARE_SUM },
    { VOLTAGE, 0, GET_ADC|CHANNEL_3, NOP, NOP, NOP, NOP, FILTER, STORE_INTO_BUFFER|CHANNEL_1, STORE_SQUARE_SUM },
    { CURRENT, 0, GET_ADC|CHANNEL_1, NOP, NOP, NOP, NOP, FILTER, STORE_INTO_BUFFER|CHANNEL_2, STORE_SQUARE_SUM },
    { VOLTAGE, 0, GET_ADC|CHANNEL_4, NOP, NOP, NOP, NOP, FILTER, STORE_INTO_BUFFER|CHANNEL_3, STORE_SQUARE_SUM },
    { CURRENT, 0, GET_ADC|CHANNEL_2, NOP, NOP, NOP, NOP, FILTER, STORE_INTO_BUFFER|CHANNEL_4, STORE_SQUARE_SUM },
    { VOLTAGE, 0, GET_ADC|CHANNEL_5, NOP, NOP, NOP, NOP, FILTER, STORE_INTO_BUFFER|CHANNEL_5, STORE_SQUARE_SUM },
    { VIRTUALCURRENT, 0, SET_ZERO, ADD|CHANNEL_0, ADD|CHANNEL_2, ADD|CHANNEL_4, NOFILTER, STORE_INTO_BUFFER|CHANNEL_6, STORE_SQUARE_SUM, NOP }
};

/* taske handle */
TaskHandle_t _MEASURE_Task;

float Irms[ VIRTUAL_CHANNELS ];
float Vrms[ VIRTUAL_CHANNELS ];

volatile int TX_buffer = -1;
uint16_t buffer[ VIRTUAL_CHANNELS ][ numbersOfSamples ];
uint16_t buffer_fft[ VIRTUAL_CHANNELS ][ numbersOfFFTSamples ];
uint16_t buffer_probe[ VIRTUAL_CHANNELS ][ numbersOfSamples ];

double HerzvReal[ numbersOfSamples * 4 ];
double HerzvImag[ numbersOfSamples * 4 ];
double netfrequency_phaseshift, netfrequency_oldphaseshift;
double netfrequency;

void measure_Task( void * pvParameters );
/*
 * 
 */
void measure_init( void ) {
    esp_err_t err;

    // config i2s to capture data from internal adc
    const i2s_config_t i2s_config = {
        .mode = i2s_mode_t(I2S_MODE_MASTER | I2S_MODE_RX | I2S_MODE_ADC_BUILT_IN ),
        .sample_rate = ( samplingFrequency * atoi( config_get_MeasureVoltageFrequency() ) / 4 ) + atoi( config_get_MeasureSamplerate()) ,
        .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
        .channel_format = I2S_CHANNEL_FMT_ALL_LEFT,
        .communication_format = i2s_comm_format_t(I2S_COMM_FORMAT_I2S),
        .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,     
        .dma_buf_count = VIRTUAL_ADC_CHANNELS * 2,                          
        .dma_buf_len = numbersOfSamples * 2,
    };

    err = i2s_driver_install(I2S_PORT, &i2s_config, 0, NULL);
    if ( err != ESP_OK ) { 
        log_e("Failed installing driver: %d", err );
        while ( true );
    }

    err = i2s_set_adc_mode( ADC_UNIT_1, (adc1_channel_t)6 );
    if (err != ESP_OK ) {
        log_e("Failed installing driver: %d", err );
        while ( true );
    }

    i2s_stop( I2S_PORT );

    vTaskDelay( 5000 / portTICK_RATE_MS );
    // 3 Items in pattern table
    SYSCON.saradc_ctrl.sar1_patt_len = VIRTUAL_ADC_CHANNELS - 1;
    // [7:4] Channel
    // [3:2] Bit Width; 3=12bit, 2=11bit, 1=10bit, 0=9bit
    // [1:0] Attenuation; 3=11dB, 2=6dB, 1=2.5dB, 0=0dB
    SYSCON.saradc_sar1_patt_tab[0] = 0x0f7f5f6f;
    SYSCON.saradc_sar1_patt_tab[1] = 0x3f4f0000;
    // make the adc conversation more accurate
    SYSCON.saradc_ctrl.sar_clk_div = 5;
    log_i("Measurement: I2S driver ready");

    measure_set_samplerate( atoi( config_get_MeasureSamplerate() ) );
}

/*
 * get as much adc buffer as possible in one second and calculate some stuff
 */
void measure_mes( void ) {

    int round=0;                      /** @brief round counter */
    int noVoltage = 0;                /** @brief set to 1 */
    static int firstrun = 10;         /** @brief startcounter to prevent trash in first run */
    double sum[ VIRTUAL_CHANNELS ];   /** @brief runtime variables for current, voltage or power calculation */
    double I_RATIO = atof( config_get_MeasureCoilTurns() ) / atof( config_get_MeasureBurdenResistor() ) * 3.3 / 4096; /** @brief I ratio value */

    memset( sum, 0, sizeof( sum ) );
    /**
     * get the current timer and calculate the exit time for capture ADC-buffer
     */
    uint64_t NextMillis = millis() + 950;

    if ( atof( config_get_MeasureVoltage() ) >= 5 )
        noVoltage = 1;

    /**
     * get numbersOfSamples * VIRTUAL_ADC_CHANNELS each round
     * one round is 40/33.3ms (depending on network freuency) or two network frecuency periods
     * after 950 ms calculate each channel value like Vrms/Irms or network frequency/paseshift
     */
    while( millis() < NextMillis ) {

        static double sampleI[ VIRTUAL_CHANNELS ], lastSampleI[ VIRTUAL_CHANNELS ];         /** @brief runtime varibales for lowpass filter stuff */
        static double filteredI[ VIRTUAL_CHANNELS ], lastFilteredI[ VIRTUAL_CHANNELS ];     /** @brief runtime varibales for lowpass filter stuff */
        uint16_t *channel[ VIRTUAL_ADC_CHANNELS ];                                          /** @brief channel pointer list */
        uint16_t adc_tempsamples[ numbersOfSamples ];                                       /** @brief unsort sampelbuffer from ADC */
        uint16_t adc_samples[ VIRTUAL_ADC_CHANNELS ][ numbersOfSamples ];                   /** @brief channel sorted samplebuffer */
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
            size_t num_bytes_read=0;
            esp_err_t err;
            err = i2s_read( I2S_PORT,
                            (char *)adc_tempsamples,      /* pointer to the adc_tempsample */
                            sizeof( adc_tempsamples ),    /* the doc says bytes, but its elements */
                            &num_bytes_read,              /* pointer to an size_t variable to store number of bytes read */
                            portMAX_DELAY );              /* no timeout */
            /**
             * handle error
             */
            if ( err != ESP_OK ) {
                log_e("Error while reading DMA Buffer: %d", err );
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
            for( int i = 0 ; i < numbersOfSamples ; i++ ) {
                /**
                 * get the right channel number from die sample and store it
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
                 * store the last sample for filtering
                 */
                lastSampleI[ i ]=sampleI[ i ];
                /**
                 * get right sample or compute it
                 */
                for( int operation = 0 ; operation < sizeof( channelconfig[ 0 ].operation ) ; operation++ ) {
                    /**
                     * mask the channel out and store it in a separate variable for later use
                     */
                    int op_channel = channelconfig[ i ].operation[ operation ] & ~OPMASK;
                    /**
                     * oparate the current channel operateion
                     */
                    switch( channelconfig[i].operation[ operation ] & OPMASK ) {
                        case NOP:                   break;
                        case ADD:                   sampleI[ i ] += filteredI[ op_channel ];
                                                    break;
                        case SUB:                   sampleI[ i ] -= filteredI[ op_channel ];
                                                    break;
                        case MUL:                   sampleI[ i ] = sampleI[ i ] * filteredI[ op_channel ];
                                                    break;
                        case DIV:                   sampleI[ i ] = sampleI[ i ] * filteredI[ op_channel ];
                                                    break;
                        case SET_ZERO:              sampleI[ i ] = 0;
                                                    break;
                        case GET_ADC:               sampleI[ i ] = adc_samples[ op_channel ][ ( numbersOfSamples/2 + n + channelconfig[ i ].phaseshift ) % numbersOfSamples ];
                                                    break;
                        case FILTER:                lastFilteredI[ i ] = filteredI[ i ];
                                                    filteredI[ i ] = ( 0.9989 - ( 0.01 * op_channel ) ) * ( lastFilteredI[ i ] + sampleI[ i ]- lastSampleI[ i ] );
                                                    break;
                        case NOFILTER:              lastFilteredI[ i ] = filteredI[ i ];
                                                    filteredI[ i ] = sampleI[ i ];
                                                    break;
                        case STORE_INTO_BUFFER:     if ( channelconfig[i].type == VOLTAGE && noVoltage == 1 ) {
                                                        buffer[ op_channel ][ n ] = 2048;
                                                    }
                                                    else {
                                                        buffer[ op_channel ][ n ] = filteredI[ i ] + 2048;
                                                    }
                                                    break;
                        case STORE_SQUARE_SUM:      sum[ i ] += filteredI[ i ] * filteredI[ i ];
                                                    break;
                        case STORE_SUM:             sum[ i ] += filteredI[ i ];
                                                    break;
                        case DIV_4096:              sampleI[ i ] = filteredI[ i ] / 4096;
                                                    break;
                        case NEG:                   sampleI[ i ] = filteredI[ i ] * -1.0;
                                                    break;
                        default:                    log_e("channel operation not allowed!");
                                                    while( 1 );
                    }
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
         * copy channel 1 into probebuffer for get network frequency
         */
        if ( round < 4 ) {
            for( int sample = 0 ; sample < numbersOfSamples ; sample++ ) {
                HerzvReal[ ( round * numbersOfSamples ) + sample ] = buffer[ 1 ][ sample ];
                HerzvImag[ ( round * numbersOfSamples ) + sample ] = 0;
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
    if ( channelconfig[ 1 ].type == VOLTAGE && noVoltage == 0 ) {
        /**
         * calculate the phaseshift between last phaseshift
         */
        arduinoFFT FFT = arduinoFFT( HerzvReal, HerzvImag, numbersOfSamples * 4, ( numbersOfSamples ) * atoi( config_get_MeasureVoltageFrequency() ) );
        FFT.Windowing( FFT_WIN_TYP_HAMMING, FFT_REVERSE);
        FFT.Compute( FFT_REVERSE );
        netfrequency_oldphaseshift = netfrequency_phaseshift;
        netfrequency_phaseshift = atan2( HerzvReal[24], HerzvImag[24] ) * ( 180.0 / M_PI ) + 180;
        /**
         * prevent chaotic phaseshift values higher then 180 degrees
         */
        if ( ( netfrequency_phaseshift - netfrequency_oldphaseshift ) < 180 && ( netfrequency_phaseshift - netfrequency_oldphaseshift ) > -180 ) {
            netfrequency = ( netfrequency + ( netfrequency_phaseshift - netfrequency_oldphaseshift ) * ( ( 1.0f / M_PI ) / 360 ) + atoi( config_get_MeasureVoltageFrequency() ) ) / 2;
        }
    }
    else {
        netfrequency = atoi( config_get_MeasureVoltageFrequency() );
    }

    float *Irms_channel=&Irms[0];
    float *Vrms_channel=&Vrms[0];

    for ( int i = 0 ; i < VIRTUAL_CHANNELS ; i++ ) {
        /*
         * Calculation of the root of the mean of the voltage and measure squared (rms)
         * Calibration coeficients applied. 
         */
        switch( channelconfig[ i ].type ) {
            case CURRENT:
                *Irms_channel = ( I_RATIO * sqrt( sum[ i ] / ( numbersOfSamples * round ) ) ) + atof( config_get_MeasureCurrentOffset() );
                if ( firstrun > 0 )
                    *Irms_channel = 0;
                Irms_channel++;
                break;
            case VIRTUALCURRENT:
                *Irms_channel = ( I_RATIO * sqrt( sum[ i ] / ( numbersOfSamples * round ) ) );
                if ( firstrun > 0 )
                    *Irms_channel = 0;
                Irms_channel++;
                break;
            case VOLTAGE:
                if ( atof( config_get_MeasureVoltage() ) < 5 )
                    *Vrms_channel = atof( config_get_MeasureVoltage() ) * sqrt( sum[ i ] / ( numbersOfSamples * round ) );
                else 
                    *Vrms_channel = atof( config_get_MeasureVoltage() );

                if ( firstrun > 0 )
                    *Vrms_channel = 0;
                
                Vrms_channel++;
                break;
        }

        if ( firstrun > 0 )
            firstrun--;
    }
}

int measure_set_samplerate( int corr ) {
    esp_err_t err;
    err = i2s_set_sample_rates( I2S_PORT, ( samplingFrequency * atoi( config_get_MeasureVoltageFrequency() ) / 4 ) + corr );
    if ( err != ESP_OK ) {
        log_e("Failed set samplerate: %d", err);
        while ( true );
    }
    return( ESP_OK );
}

uint16_t * measure_get_buffer( void ) {
    TX_buffer = 0;
    while ( TX_buffer == 0 ) {}
    return( &buffer_probe[0][0] ); 
}

uint16_t * measure_get_fft( void ) {
  double vReal[ numbersOfFFTSamples * 2 ];
  double vImag[ numbersOfFFTSamples * 2 ];
  arduinoFFT FFT = arduinoFFT( vReal, vImag, numbersOfFFTSamples * 2, samplingFrequency * atoi( config_get_MeasureVoltageFrequency() ) / 4 );

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

float measure_get_Irms( int line ) {
    return( Irms[ line ] );
}

float measure_get_Vrms( int line ) {
    return( Vrms[ line ] );
}

float measure_get_power( int line ) {
    return( Irms[ line ] * Vrms[ line ] );
}

float measure_get_Iratio( void ) {
    return( atof( config_get_MeasureCoilTurns() ) / atof( config_get_MeasureBurdenResistor() ) * 3.3 / 4096 );
}

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

uint8_t measure_get_channel_type( uint16_t channel ){
    if ( channel >= VIRTUAL_CHANNELS )
        return( 0 );
        
    return( channelconfig[ channel ].type );
}

void measure_set_channel_type( uint16_t channel, uint8_t value ){
    if ( channel >= VIRTUAL_CHANNELS )
        return;
        
    channelconfig[ channel ].type = value;
    log_d("set channel %d type to %d", channel, channelconfig[ channel ].type );
}


int16_t measure_get_channel_phaseshift( uint16_t channel ) {
    if ( channel >= VIRTUAL_CHANNELS )
        return( 0 );

    return( channelconfig[ channel ].phaseshift );
}

void measure_set_channel_phaseshift( uint16_t channel, int16_t value ) {
    if ( channel >= VIRTUAL_CHANNELS )
        return;

    channelconfig[ channel ].phaseshift = value;
    log_d("set channel %d phaseshift to %d", channel, channelconfig[ channel ].phaseshift );
}

uint8_t * measure_get_channel_opcodeseq( uint16_t channel ) {
    if ( channel >= VIRTUAL_CHANNELS )
        return( NULL );

    return( channelconfig[ channel ].operation );
}

char * measure_get_channel_opcodeseq_str( uint16_t channel, uint16_t len, char *dest ) {
    char microcode_tmp[ 3 ] = "";
    uint8_t *opcode = channelconfig[ channel ].operation;

    if ( channel >= VIRTUAL_CHANNELS )
        return( NULL );

    if ( !dest )
        return( NULL );

    for( int a = 0 ; a < MAX_MICROCODE_OPS ; a++ ) {
        snprintf( microcode_tmp, sizeof( microcode_tmp ), "%02x", *opcode );
        strncat( dest, microcode_tmp, len );
        opcode++;
    }

    return( dest );
}

void measure_set_channel_opcodeseq( uint16_t channel, uint8_t *value ) {
    char microcode[ VIRTUAL_CHANNELS * 3 ] = "";
    char microcode_tmp[ 3 ] = "";
    uint8_t *opcode = value;

    if ( channel >= VIRTUAL_CHANNELS )
        return;

    memcpy( channelconfig[ channel ].operation, value, MAX_MICROCODE_OPS );

    for( int a = 0 ; a < MAX_MICROCODE_OPS ; a++ ) {
        snprintf( microcode_tmp, sizeof( microcode_tmp ), "%02x", *opcode );
        strncat( microcode, microcode_tmp, sizeof( microcode ) );
        opcode++;
    }

    log_d("set channel %d microcode to \"%s\"", channel, microcode );
}

void measure_set_channel_opcodeseq_str( uint16_t channel, const char *value ) {
    char spanset[] = "0123456789ABCDEFabcdef";
    char *ptr = (char *)value;
    uint8_t opcode_binary = 0;
    int opcode_pos = 0;

    if ( channel >= VIRTUAL_CHANNELS )
        return;

    log_d("set channel %d microcode to \"%s\"", channel, ptr );
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
            log_d("opcode = %02x, pos = %d", opcode_binary, opcode_pos );
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
