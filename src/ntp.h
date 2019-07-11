#ifndef _NTP_H

  #define _NTP_H

  #define NTP_RENEW_INTERVAL  3600 * 24

  void ntp_StartTask( void );
  void ntp_Task( void * pvParameters );

#endif // _NTP_H
