/****************************************************************************
 *            ota.cpp
 *
 *  Sa April 27 12:17:32 2019
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
#include "time.h"

#include "ntp.h"
#include "config.h"

TaskHandle_t _NTP_Task;
/*
 * 
 */
const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = 3600;
const int   daylightOffset_sec = 3600;

/*
 * 
 */
void ntp_StartTask( void ) {
  
  xTaskCreatePinnedToCore(
                    ntp_Task,    /* Function to implement the task */
                    "ntp Task",  /* Name of the task */
                    10000,              /* Stack size in words */
                    NULL,               /* Task input parameter */
                    1,                  /* Priority of the task */
                    &_NTP_Task,               /* Task handle. */
                    _NTP_TASKCORE );    /* Core where the task should run */     
}

/*
 * 
 */
void ntp_Task( void * pvParameters ) {
  /*
   * 
   */
  struct tm timeinfo;
  static uint64_t NextMillis = millis();
  
  Serial.printf("Start NTP Task on Core: %d\r\n", xPortGetCoreID() );

  while ( true ) {
    /*   
     * 
     */
    vTaskDelay( 10 );
    if ( NextMillis < millis() ) {
      NextMillis += NTP_RENEW_INTERVAL * 1000l;
      Serial.printf( "NTP-client: renew time\r\n" );

      configTime( gmtOffset_sec, daylightOffset_sec, ntpServer );

      if( !getLocalTime(&timeinfo ) ) {
        Serial.println( "Failed to obtain time" );
      }
      Serial.println( &timeinfo, "NTP-client: %A, %B %d %Y %H:%M:%S" );
    }
  }
}
