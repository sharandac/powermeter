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

  #define numbersOfSamples      384
  #define numbersOfFFTSamples   32
  #define samplingFrequency     numbersOfSamples*VIRTUAL_ADC_CHANNELS
  #define DELAY                 1000
  #define I2S_PORT              I2S_NUM_0

  struct channelconfig {
    uint8_t type;
    int16_t phaseshift;
    uint8_t operation[8];
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

  #define OPMASK 0xf0

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
    DIV_4096 = 0xc0
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

  void measure_init( void );
  void measure_mes( void );
  int measure_set_phaseshift( int corr );
  int measure_set_samplerate( int corr );
  uint16_t * measure_get_buffer( void );
  uint16_t * measure_get_fft( void );
  float measure_get_Irms( int line );
  float measure_get_Vrms( int line );
  float measure_get_power( int line );
  float measure_get_Iratio( void );
  void measure_StartTask( void );
  void measure_StartSignalGenTask( void );
  void measure_Task( void * pvParameters );
  void measure_SignalGenTask( void * pvParameters );
  
#endif // _MEASURE_H
