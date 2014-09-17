/**
 *  \file  ev3_link.c
 *  \brief  EV3 remote access.
 *  \author  Vitaly Kravtsov (in4lio@gmail.com)
 *  \copyright  See the LICENSE file.
 */

#include <strings.h>
#include <stdio.h>

#include "lego.h"
#include "ev3_wlan.h"
#include "ev3_link.h"

#define TCP_MESSAGE_LIMIT  1024

static uint8_t __r_msg[ TCP_MESSAGE_LIMIT ];
static COMRPL *__r_head = ( COMRPL* ) __r_msg;
static uint8_t *__r_body = ( uint8_t* )( __r_msg + sizeof( COMRPL ));

static uint8_t __t_msg[ TCP_MESSAGE_LIMIT ];
static COMCMD *__t_head = ( COMCMD* ) __t_msg;
static DIRCMD *__t_dir = ( DIRCMD* )( __t_msg + sizeof( COMCMD ));

static MSGCNT __t_last = 0;

#define DIRECT_LOCALS_OFFSET  10

int ev3_tcp_direct( uint16_t globals, uint16_t locals, uint8_t *code, size_t sz, uint8_t reply )
{
	uint32_t l = __t_dir->Code - __t_msg + sz;

	if ( l > TCP_MESSAGE_LIMIT ) return ( EOF );
	if (( locals > MAX_COMMAND_LOCALS ) || ( globals > MAX_COMMAND_GLOBALS )) return ( EOF );

	__t_head->CmdSize = l - sizeof( CMDSIZE );
	__t_head->MsgCnt = ++__t_last;
	__t_head->Cmd = reply;
	*( UWORD* ) &__t_dir->Globals = ( UWORD ) globals | (( UWORD ) locals >> DIRECT_LOCALS_OFFSET );
	memcpy( __t_dir->Code, code, sz );

	return ( ev3_tcp_write( __t_msg, l ) > 0 ) ? __t_last : ( EOF );
}

int ev3_tcp_direct_reply( int msgn, uint8_t *buf, size_t *sz )
{
	int l;

	if ( msgn == EOF ) return ( EOF );

	l = ev3_tcp_read( __r_msg, sizeof( __r_msg ));
	if ( l == 0 ) return ( 0 );  /* message is absent */

	if ( l != __r_head->CmdSize + sizeof( CMDSIZE )) return ( EOF );  /* corrupt message */

	if ( msgn != __r_head->MsgCnt ) return ( 0 );  /* strange message */

	l -= sizeof( COMRPL );
	if ( *sz < l ) return ( EOF );  /* too short buffer */

	memcpy( buf, __r_body, l );
	*sz = l;

	return ( __r_head->Cmd );
}
