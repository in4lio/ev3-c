/**
 *  \file  ev3_link.h
 *  \brief  EV3 remote access.
 *  \author  Vitaly Kravtsov (in4lio@gmail.com)
 *  \copyright  See the LICENSE file.
 */

#pragma once

#include <stddef.h>
#include <stdint.h>

extern int ev3_tcp_direct( uint16_t globals, uint16_t locals, uint8_t *code, size_t sz, uint8_t reply );
extern int ev3_tcp_direct_reply( int msgn, uint8_t *buf, size_t *sz );
