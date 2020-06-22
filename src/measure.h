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
  #define VIRTUAL_ADC_CHANNELS  3
  #define VIRTUAL_CHANNELS      4

  #define numbersOfSamples      510
  #define numbersOfFFTSamples   32
  #define samplingFrequency     numbersOfSamples*VIRTUAL_ADC_CHANNELS
  #define DELAY                 1000
  #define I2S_PORT              I2S_NUM_0

  struct channelconfig {
    int8_t type;
    int16_t phaseshift;
    int8_t operation[5];
  };

  enum {
    NONE = -1,
    CURRENT,
    VIRTUALCURRENT,
    VOLTAGE,
    VIRTUALVOLTAGE
  };

  #define OPMASK 0xf0

  enum {
    NOP = 0,
    ADD = 0x10,
    MUL = 0x20,
    STORE = 0x30,
    ZERO = 0x40,
    SUB = 0x50,
    FILTER = 0x60,
    NOFILTER = 0x70
  };

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
  };

  void measure_init( void );
  void measure_mes( void );
  int measure_set_samplerate( int corr );
  float measure_get_power( int line );
  uint16_t * measure_get_buffer( void );
  uint16_t * measure_get_fft( void );
  float measure_get_Irms( int line );
  float measure_get_Iratio( void );
  void measure_StartTask( void );
  void measure_StartSignalGenTask( void );
  void measure_Task( void * pvParameters );
  void measure_SignalGenTask( void * pvParameters );
  
#endif // _MEASURE_H
