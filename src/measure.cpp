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

#include "measure.h"
#include "config.h"

extern "C" {
    #include "soc/syscon_reg.h"
    #include "soc/syscon_struct.h"
}

/*
 * map 8 real channel to virtual channel
 * -1 means not mapped
 * 
 * by example: channel 5 map to virtual channel 0, 6 to 1 and 7 to 2
 */ 
int8_t channelmapping[ MAX_ADC_CHANNELS ] = { -1,-1,-1,-1,-1,0,1,2 };

/*
 * define virtual channel type and their mathematics
 */
struct channelconfig channelconfig[ VIRTUAL_CHANNELS ] =
{ 
  { CURRENT, 0, CHANNEL_ADD_NOP, CHANNEL_ADD_NOP, CHANNEL_ADD_NOP },
  { CURRENT, 0, CHANNEL_ADD_NOP, CHANNEL_ADD_NOP, CHANNEL_ADD_NOP },
  { CURRENT, 0, CHANNEL_ADD_NOP, CHANNEL_ADD_NOP, CHANNEL_ADD_NOP },
  { VIRTUALCURRENT, 0, CHANNEL_ADD_0, CHANNEL_ADD_1, CHANNEL_ADD_2 },
};

TaskHandle_t _MEASURE_Task;
TaskHandle_t _MEASURE_SignalGenTask;

double ICAL = 1.095;

float Power[ VIRTUAL_CHANNELS ];
float Irms[ VIRTUAL_CHANNELS ];

volatile int TX_buffer = -1;
uint16_t buffer[ VIRTUAL_CHANNELS ][ numbersOfSamples ];
uint16_t buffer_fft[ VIRTUAL_CHANNELS ][ numbersOfFFTSamples ];
uint16_t buffer_probe[ VIRTUAL_CHANNELS ][ numbersOfSamples ];

arduinoFFT FFT = arduinoFFT();

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
      .dma_buf_count = MEASURE_CHANNELS * 2,                          
      .dma_buf_len = numbersOfSamples * 2,
    };

  err = i2s_driver_install(I2S_PORT, &i2s_config, 0, NULL);
  if (err != ESP_OK) { 
    Serial.printf("Failed installing driver: %d\r\n", err);
    while (true);
  }
  err = i2s_set_adc_mode( ADC_UNIT_1, (adc1_channel_t)6 );
  if (err != ESP_OK ) {
    Serial.printf("Failed installing driver: %d\r\n", err);
    while (true);
  }

  i2s_stop(I2S_PORT);
  vTaskDelay(5000/portTICK_RATE_MS);
  // 3 Items in pattern table
  SYSCON.saradc_ctrl.sar1_patt_len = MEASURE_CHANNELS - 1;
  // [7:4] Channel
  // [3:2] Bit Width; 3=12bit, 2=11bit, 1=10bit, 0=9bit
  // [1:0] Attenuation; 3=11dB, 2=6dB, 1=2.5dB, 0=0dB
  SYSCON.saradc_sar1_patt_tab[0] = 0x6f7f5f00;
  // make the adc conversation more accurate
  SYSCON.saradc_ctrl.sar_clk_div = 6;
  Serial.printf("Measurement: I2S driver ready\r\n");

  ledcAttachPin( 32 , 0);
  ledcSetup(0, 100, 8);
  ledcWrite(0, 127);
}

/*
 * capture many ADC-buffer in a secound as possible and calculate some stoff
 */
void measure_mes( void ) {
    
  /* ADC-buffer counter for measurement round per run */
  int round=0;
  /* startcounter to prevent trash in first run */
  static int firstrun = 10;
  /* runtime variables for current calculation */
  double sumI[ VIRTUAL_CHANNELS ];
    
  double I_RATIO = atof( config_get_MeasureCoilTurns() ) / atof( config_get_MeasureBurdenResistor() ) * 3.3 / 4096 * ICAL;
  memset( sumI, 0, sizeof( sumI ) );

  /* get the current timer and calculate the exit time for capture ADC-buffer */
  uint64_t NextMillis = millis() + 950;

  /*
   * Sample measure
   */
  while( millis() < NextMillis ){

    /* unsort sampelbuffer from ADC */
    uint16_t tempsamples[ MEASURE_CHANNELS * numbersOfSamples ];
    /* channel sorted samplebuffer */
    uint16_t samples[ MEASURE_CHANNELS ][ numbersOfSamples ];
    /* runtime varibales for lowpass filter stuff */
    static int lastSampleI[ VIRTUAL_CHANNELS ], sampleI[ VIRTUAL_CHANNELS ];
    static double lastFilteredI[ VIRTUAL_CHANNELS ], filteredI[ VIRTUAL_CHANNELS ];

    /* get an ADC buffer */
    size_t num_bytes_read=0;
    esp_err_t err;
    err = i2s_read( I2S_PORT,
                    (char *)tempsamples,
                    sizeof(tempsamples),     // the doc says bytes, but its elements.
                    &num_bytes_read,
                    portMAX_DELAY ); // no timeout

    if (err != ESP_OK) {
      Serial.printf("Error while reading DMA Buffer: %d", err );
      while(1);
    }

    /*
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
    uint16_t *channel[ MEASURE_CHANNELS ];
    for( int i = 0 ; i < MEASURE_CHANNELS ; i++ ) {
      channel[ i ] = &samples[ i ][ 0 ];
    }

    for( int i = 0 ; i < numbersOfSamples * MEASURE_CHANNELS ; i++ ) {
      /* get the right channel number from die sample */
      uint8_t chan = (tempsamples[i]>>12) & 0x7;
      /* assigns the sample to the correct virtual channel */
      if ( channelmapping[ chan ] != -1 && chan < sizeof( channelmapping ) ) {
        *channel[ channelmapping[ chan ] ] = tempsamples[ i ];
        channel[ channelmapping[ chan ] ]++;
      }
    }

    /*
     * calculate current for each virtual channel
     */
    for (int n=0; n<numbersOfSamples; n++) {
      for ( int i = 0 ; i < VIRTUAL_CHANNELS ; i++ ) {
        // sample, highpass filter and root-mean-square multiply
        lastSampleI[ i ]=sampleI[ i ];
        /* get right sample or compute it */
        switch( channelconfig[ i ].type ) {
          case( CURRENT ):          sampleI[ i ] = samples[ i ][ ( n + channelconfig[ i ].phaseshift ) % numbersOfSamples ]&0x0fff;
                                    break;
          case( VIRTUALCURRENT ):   sampleI[ i ] = 0;
                                    if ( channelconfig[i].math[0] != CHANNEL_ADD_NOP ) { sampleI[ i ] += buffer[ channelconfig[i].math[0] ][ n ]; }
                                    if ( channelconfig[i].math[1] != CHANNEL_ADD_NOP ) { sampleI[ i ] += buffer[ channelconfig[i].math[1] ][ n ]; }
                                    if ( channelconfig[i].math[2] != CHANNEL_ADD_NOP ) { sampleI[ i ] += buffer[ channelconfig[i].math[2] ][ n ]; }
                                    break;
        }
        lastFilteredI[ i ] = filteredI[ i ];
        filteredI[ i ] = 0.9989 * ( lastFilteredI[ i ] + sampleI[ i ]- lastSampleI[ i ] );
        buffer[ i ][ n ] = filteredI[ i ] + 2048;
        // root-mean-square method measure, square measure values and add to sumI
        sumI[ i ] += filteredI[ i ] * filteredI[ i ];  
      }
    }
    if ( TX_buffer != -1 ) {
      memcpy( &buffer_probe[0][0], &buffer[0][0] , sizeof( buffer ) );
      TX_buffer = -1;
    }
    round++;
  }

  for ( int i = 0 ; i < atoi(config_get_MeasureChannels()) + 1 ; i++ ) {
    /*
    / Calculation of the root of the mean of the voltage and measure squared (rms)
    / Calibration coeficients applied. 
    */
    Irms[ i ] = ( I_RATIO * sqrt( sumI[ i ] / ( numbersOfSamples * round ) ) ) - atof( config_get_MeasureOffset() );

    /* Set negative measure to zero */
    if (Irms[ i ] < 0) {
      Irms[ i ] = 0;
    }

    /* Calculate Power */
    Power[i] = ( Irms[i] * atof( config_get_MeasureVoltage() ) + Power[ i ] ) / 2;

    if ( firstrun > 0 ) {
      firstrun--;
      Irms[ i ] = 0;
      Power[ i ] = 0;
    }
  }
}

/*
 * 
 */
float measure_get_power( int line ) {
  return( Power[ line ] );
}

/*
 *
 */
int measure_set_samplerate( int corr ) {
  esp_err_t err;
  err = i2s_set_sample_rates( I2S_PORT, ( samplingFrequency * atoi( config_get_MeasureVoltageFrequency() ) / 4 ) + corr );
  if (err != ESP_OK ) {
    Serial.printf("Failed set samplerate: %d\r\n", err);
    while (true);
  }
  return( ESP_OK );
}

/*
 *
 */
uint16_t * measure_get_buffer( void ) {
  TX_buffer = 0;
  while ( TX_buffer == 0 ) {}
  return( &buffer_probe[0][0] ); 
}

/*
 *
 */
uint16_t * measure_get_fft( void ) {
  double vReal[ numbersOfFFTSamples * 2 ];
  double vImag[ numbersOfFFTSamples * 2 ];

  for ( int channel = 0 ; channel < MEASURE_CHANNELS + 1 ; channel++ ) {
    for ( int i = 0 ; i < numbersOfFFTSamples * 2 ; i++ ) {
      vReal[i] = buffer_probe[channel][ (i*8)%numbersOfSamples ];
      vImag[i] = 0;
    }
    FFT.Windowing(vReal, numbersOfFFTSamples, FFT_WIN_TYP_RECTANGLE, FFT_REVERSE);
    FFT.Compute(vReal, vImag, numbersOfFFTSamples * 2, FFT_REVERSE );
    FFT.ComplexToMagnitude(vReal, vImag, numbersOfFFTSamples * 2 );
    
    for( int i = 1 ; i < numbersOfFFTSamples; i++ ){
      buffer_fft[channel][i] = vReal[i];
    }
  }
  return( &buffer_fft[0][0] );
}

/*
 * 
 */
float measure_get_Irms( int line ) {
  return( Irms[ line ] );
}

/*
 *
 */
float measure_get_Iratio( void ) {
  return( atof( config_get_MeasureCoilTurns() ) / atof( config_get_MeasureBurdenResistor() ) * 3.3 / 4096 * ICAL );
}

/*
 * 
 */
void measure_StartTask( void ) {
  xTaskCreatePinnedToCore(
                    measure_Task,   /* Function to implement the task */
                    "measure measurement Task", /* Name of the task */
                    10000,      /* Stack size in words */
                    NULL,       /* Task input parameter */
                    2,          /* Priority of the task */
                    &_MEASURE_Task,       /* Task handle. */
                    _MEASURE_TASKCORE );  /* Core where the task should run */  
}

/*
 * 
 */
void measure_Task( void * pvParameters ) {
  static uint64_t NextMillis = millis();

  Serial.printf("Start Measurement Task on Core: %d\r\n", xPortGetCoreID() );

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

