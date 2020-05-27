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
#include <Arduino.h>
#include "freertos/ringbuf.h"
#include "esp_types.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/periph_ctrl.h"
#include "driver/timer.h"
#include <driver/i2s.h>

#include "measure.h"
#include "config.h"


TaskHandle_t _MEASURE_Task;

const i2s_port_t I2S_PORT = I2S_NUM_0;

double ICAL = 1.095;

float PowerSum[ MEASURE_CHANELS ]; //Power Calculation
float PowerOverAll[ MEASURE_CHANELS ];
float Irms_real[ MEASURE_CHANELS ];

volatile int TX_buffer = -1;
uint16_t buffer[ numberOfSamples ];
uint16_t backbuffer[ MEASURE_CHANELS ][ numberOfSamples ];

/*
 * 
 */
void measure_init( void ) {

  for ( int i = 0 ; i < MEASURE_CHANELS ; i++ ) {
    PowerOverAll[ i ] = atof( config_get_MeasureCounter() ) / MEASURE_CHANELS ;
  }


  Serial.printf("Configuring I2S ...");
  esp_err_t err;

  // config i2s to capture data from internal adc
  const i2s_config_t i2s_config = {
      .mode = i2s_mode_t(I2S_MODE_MASTER | I2S_MODE_RX | I2S_MODE_ADC_BUILT_IN ),
      .sample_rate = samplingFrequency,                 
      .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
      .channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT,     // fill right and left channel with data
      .communication_format = i2s_comm_format_t(I2S_COMM_FORMAT_I2S),
      .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,     
      .dma_buf_count = 4,                          
      .dma_buf_len = numberOfSamples               
    };

  // setup pinconfig to complete internal setup
  i2s_pin_config_t pin_config = {
      .bck_io_num = -1,
      .ws_io_num = -1,
      .data_out_num = -1,
      .data_in_num = -1
  };

  err = i2s_driver_install(I2S_PORT, &i2s_config, 0, NULL);
  if (err != ESP_OK) { 
    Serial.printf("Failed installing driver: %d\r\n", err);
    while (true);
  }
  err = i2s_set_pin(I2S_PORT, &pin_config);
  if (err != ESP_OK) {
    Serial.printf("Failed setting pin: %d\r\n", err);
    while (true);
  }
  err = i2s_set_clk( I2S_PORT, samplingFrequency, I2S_BITS_PER_SAMPLE_16BIT, I2S_CHANNEL_MONO );
  if (err != ESP_OK) {
    Serial.printf("Failed setting I2C clock: %d\r\n", err);
    while (true);
  }
  Serial.println("I2S driver ready\r\n");
}

/*
 * 
 */
void measure_mes( void ) {
  
  double Irms[ MEASURE_CHANELS ];
  double sumI[ MEASURE_CHANELS ];

  static int firstrun = 0;
  static int lastSampleI[ MEASURE_CHANELS ];
  static int sampleI[ MEASURE_CHANELS ];
  static double lastFilteredI[ MEASURE_CHANELS ];
  static double filteredI[ MEASURE_CHANELS ];

  double I_RATIO = atof( config_get_MeasureCoilTurns() ) / atof( config_get_MeasureBurdenResistor() ) * 3.3 / 4096 * ICAL;

  memset( sumI, 0, sizeof( sumI ) );

  /*
   * Sample measure
   */
  for ( int i = 0 ; i < MEASURE_CHANELS ; i++ ) {

    uint16_t samples[numberOfSamples];

    // change adc input channel and disable i2s to prevent garbage
    i2s_stop(I2S_PORT);
    i2s_set_adc_mode(ADC_UNIT_1, (adc1_channel_t)( atoi( config_get_MeasurePin() ) + i ) );
    i2s_start(I2S_PORT);
    // wait 100ms to change the adc input an wait for buffer
    delay(100);
    // disable adc to prevent garbage
    i2s_adc_disable(I2S_PORT);
    int num_bytes_read = i2s_read_bytes(I2S_PORT, 
                                        (char *)samples,
                                        sizeof(samples),     // the doc says bytes, but its elements.
                                        portMAX_DELAY ); // no timeout
    i2s_adc_enable(I2S_PORT);
    // sort the buffer, exchange LEFT and RIGHT channel for the right data order
    for(int i=0;i<numberOfSamples;i=i+2) {
      uint16_t temp = samples[i+1];
      samples[i+1]=samples[i];
      samples[i]=temp;
    }

    for (int n=0; n<numberOfSamples; n++) {
      // sample, highpass filter and root-mean-square multiply
      lastSampleI[ i ]=sampleI[ i ];
      sampleI[ i ] = samples[n];
      lastFilteredI[ i ] = filteredI[ i ];
      filteredI[ i ] = 0.9989 * ( lastFilteredI[ i ] + sampleI[ i ]- lastSampleI[ i ] );    
      backbuffer[ i ][ n ] = filteredI[ i ] + 2048;
      // root-mean-square method measure, square measure values and add to sumI
      sumI[ i ] += filteredI[ i ] * filteredI[ i ];      
    }
  }
  if ( TX_buffer != -1 ) {
    memcpy( buffer, &backbuffer[ TX_buffer ][ 0 ] , sizeof( buffer ) );
    TX_buffer = -1;
  }
  
  for ( int i = 0 ; i < MEASURE_CHANELS ; i++ ) {
    //Calculation of the root of the mean of the voltage and measure squared (rms)
    //Calibration coeficients applied. 
    Irms[ i ] = ( I_RATIO * sqrt( sumI[ i ] / ( numberOfSamples ) ) ) - atof( config_get_MeasureOffset() );
    Irms_real[ i ] = Irms[ i ];

    //Set negative measure to zero
    if (Irms[ i ] < 0) {
      Irms[ i ] = 0;
    }

    //Calculate Power
    PowerSum[i] = ( Irms[i] * atof( config_get_MeasureVoltage() ) + PowerSum[ i ] ) / 2;

    if (firstrun <= 10) {
      firstrun++;
      PowerSum[ i ] = 0;
    }
    else {
      PowerOverAll[ i ] = PowerOverAll[ i ] + ( PowerSum[ i ] / 3600 );
    }
  }

  if ( firstrun > 10 ) {
    char value[16];
    snprintf( value, sizeof( value ), "%.3f", PowerOverAll[ 0 ]);
    config_set_MeasureCounter( value );
  }
}

/*
 * 
 */
float measure_get_power( int line ) {
  return( PowerSum[ line ] );
}

/*
 * 
 */
float measure_get_poweroverall( int line ) {
  return( PowerOverAll[ line ] );
}

/*
 *
 */
uint16_t * measure_get_buffer( int channel ) {
  TX_buffer = channel;
  while ( TX_buffer == channel ) {}
  return( buffer ); 
}

/*
 * 
 */
void measure_set_poweroverall( int line, float value) {
  PowerOverAll[ line ] = value;
  return;
}
/*
 * 
 */
float measure_get_Irms( int line ) {
  return( Irms_real[ line ] );
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
  static unsigned long NextMillis = millis();

  Serial.printf("Start Measurement Task on Core: %d\r\n", xPortGetCoreID() );

  measure_init();

  while( true ) {

    vTaskDelay( 1 );
   
    if ( millis() - NextMillis > DELAY ) {
      NextMillis += DELAY;
      measure_mes();
    }
  }
}
