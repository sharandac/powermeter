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

#include "measure.h"
#include "config.h"


TaskHandle_t _MEASURE_Task;

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
  int inPinI = atoi( config_get_MeasurePin() );
  for ( int i = 0 ; i < MEASURE_CHANELS ; i++ ) {
    pinMode( inPinI + i ,INPUT);     //Set Analog Inputs
    adcAttachPin( inPinI + i );
    PowerOverAll[ i ] = atof( config_get_MeasureCounter() ) / MEASURE_CHANELS ;
  }
}

/*
 * 
 */
void measure_mes( void ) {
  
  double Irms[ MEASURE_CHANELS ];
  double sumI[ MEASURE_CHANELS ];
  
  int rounds = 0;
  unsigned long NextMillis = millis() + 950;

  static int firstrun = 0;
  static int lastSampleI[ MEASURE_CHANELS ];
  static int sampleI[ MEASURE_CHANELS ];
  static double lastFilteredI[ MEASURE_CHANELS ];
  static double filteredI[ MEASURE_CHANELS ];

  int inPinI = atoi( config_get_MeasurePin() );
  double I_RATIO = atof( config_get_MeasureCoilTurns() ) / atof( config_get_MeasureBurdenResistor() ) * 3.3 / 4096 * ICAL;

  memset( sumI, 0, sizeof( sumI ) );
  /*
   * Sample measure
   */
  while ( millis() < NextMillis ) {
    vTaskSuspendAll();
    cli();
    for (int n=0; n<numberOfSamples; n++) {
      for ( int i = 0 ; i < MEASURE_CHANELS ; i++ ) {
        /*
         * sample, highpass filter and root-mean-square multiply
         */
        lastSampleI[ i ]=sampleI[ i ];
        sampleI[ i ] = analogRead( inPinI + i );
        lastFilteredI[ i ] = filteredI[ i ];
        filteredI[ i ] = 0.9989 * ( lastFilteredI[ i ] + sampleI[ i ]- lastSampleI[ i ] );    
        backbuffer[ i ][ n ] = filteredI[ i ] + 2048;
        /*
         * root-mean-square method measure, square measure values and add to sumI
         */
        sumI[ i ] += filteredI[ i ] * filteredI[ i ];
        
      }
      /*
       * 75μs delay, 9μs for ADC 
       */
      delayMicroseconds(66);
    }
    sei();
    xTaskResumeAll();
    if ( TX_buffer != -1 ) {
      memcpy( buffer, &backbuffer[ TX_buffer ][ 0 ] , sizeof( buffer ) );
      TX_buffer = -1;
    }
    rounds++;
  }
  
//  Serial.printf("%d mesurements\r\n", rounds );

  for ( int i = 0 ; i < MEASURE_CHANELS ; i++ ) {
    //Calculation of the root of the mean of the voltage and measure squared (rms)
    //Calibration coeficients applied. 
    Irms[ i ] = ( I_RATIO * sqrt( sumI[ i ] / ( numberOfSamples * rounds ) ) ) - atof( config_get_MeasureOffset() );
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
//    snprintf( value, sizeof( value ), "%.3f", PowerOverAll[ 0 ] + PowerOverAll[ 1 ] + PowerOverAll[ 2 ]);
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

    vTaskDelay(1);
   
    if ( millis() - NextMillis > DELAY ) {
      NextMillis += DELAY;
      measure_mes();
    }
  }
}
