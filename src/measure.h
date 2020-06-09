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

  #define MEASURE_CHANELS       3
  #define numbersOfSamples      510
  #define numbersOfFFTSamples   32
  #define samplingFrequency     numbersOfSamples*MEASURE_CHANELS
  #define DELAY                 1000
  
  void measure_init( void );
  void measure_mes( void );
  int measure_set_samplerate( int corr );
  float measure_get_power( int line );
  uint16_t * measure_get_buffer( void );
  uint16_t * measure_get_fft( void );
  void measure_Task( void * pvParameters );
  void measure_StartTask( void );
  float measure_get_Irms( int line );
  float measure_get_Iratio( void );
  
#endif // _MEASURE_H
