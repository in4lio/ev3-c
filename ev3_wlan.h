/**
 *  \file  ev3_wlan.h
 *  \brief  EV3 WLAN interface.
 *  \author  Vitaly Kravtsov (in4lio@gmail.com)
  *  \copyright  See the LICENSE file.
*/

#pragma once

#include <stddef.h>
#include <stdint.h>

extern char *ev3_brick_addr;

extern int ev3_udp_connect( void );
extern int ev3_udp_disconnect( void );
extern int ev3_udp_catch_address( void );

extern int ev3_tcp_connect( void );
extern int ev3_tcp_disconnect( void );
extern int ev3_tcp_unlock_request( void );
extern int ev3_tcp_unlock_reply( void );
extern int ev3_tcp_write( uint8_t *msg, int sz );
extern int ev3_tcp_read( uint8_t *buf, int sz );
