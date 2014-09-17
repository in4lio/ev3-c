/**
 *  \file  ev3_wlan.c
 *  \brief  EV3 WLAN interface.
 *  \author  Vitaly Kravtsov (in4lio@gmail.com)
 *  \copyright  See the LICENSE file.
 */

#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

// WIN32 /////////////////////////////////////////
#ifdef __WIN32__
#define socklen_t int32_t
#include <winsock.h>
#include <wininet.h>

#define bzero( ptr, sz ) memset( ptr, 0, sz )

// UNIX //////////////////////////////////////////
#else
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <arpa/inet.h>

//////////////////////////////////////////////////
#endif

//#define DEBUG_TCP_MESSAGES

#define UDP_PORT  3015
#define TCP_PORT  5555

#define TCP_UNLOCK_REQUEST  "GET /target?sn=\r\nProtocol:EV3\r\n\r\n"
#define TCP_UNLOCK_REPLY  "Accept:EV340\r\n\r\n"
#define TCP_UNLOCK_REPLY_SZ  ( sizeof( TCP_UNLOCK_REPLY ) - 1 )

char *ev3_brick_addr = "192.168.0.204";

static int udpfd;
static int sockfd;
static struct sockaddr_in __addr;

int ev3_udp_connect( void )
{
	u_long optval = 1;
	int res;

// WIN32 /////////////////////////////////////////
#ifdef __WIN32__
	WSADATA wsa;

	res = WSAStartup( MAKEWORD( 2, 2 ), &wsa );
	if ( res ) {
		printf( "*** ERROR *** ev3_udp_connect() = %d\n", res );
		perror( "    WSAStartup()" );
		printf( "\n" );
		udpfd = EOF;
		return ( EOF );
	}

//////////////////////////////////////////////////
#endif
	udpfd = socket( PF_INET, SOCK_DGRAM, 0 );
	if ( udpfd < 0 ) {
		printf( "*** ERROR *** ev3_udp_connect() = %d\n", udpfd );
		perror( "    socket()" );
		printf( "\n" );
		udpfd = EOF;

// WIN32 /////////////////////////////////////////
#ifdef __WIN32__
		WSACleanup();

//////////////////////////////////////////////////
#endif
		return ( EOF );
	}
	bzero( &__addr, sizeof( struct sockaddr_in ));
	__addr.sin_family = AF_INET;
	__addr.sin_port = htons( UDP_PORT );
	__addr.sin_addr.s_addr = htonl( INADDR_ANY );

	res = bind( udpfd, ( struct sockaddr* ) &__addr, sizeof( __addr ));
	if ( res < 0 ) {
		printf( "*** ERROR *** ev3_udp_connect() = %d\n", res );
		perror( "    bind()" );
		printf( "\n" );
		close( udpfd );
		udpfd = EOF;

// WIN32 /////////////////////////////////////////
#ifdef __WIN32__
		WSACleanup();

//////////////////////////////////////////////////
#endif
		return ( EOF );
	}

// WIN32 /////////////////////////////////////////
#ifdef __WIN32__
	setsockopt( udpfd, SOL_SOCKET, SO_BROADCAST, ( const char* ) &optval, sizeof( optval ));
	ioctlsocket( udpfd, FIONBIO, &optval );

// UNIX //////////////////////////////////////////
#else
	setsockopt( udpfd, SOL_SOCKET, SO_BROADCAST, &optval, sizeof( optval ));
	fcntl( udpfd, F_SETFL, O_NONBLOCK );

//////////////////////////////////////////////////
#endif
	return ( 0 );
}

int ev3_udp_disconnect( void )
{
	if ( udpfd < 0 ) return ( EOF );
	close( udpfd );

// WIN32 /////////////////////////////////////////
#ifdef __WIN32__
	WSACleanup();

//////////////////////////////////////////////////
#endif
	return ( 0 );
}

int ev3_udp_catch_address( void )
{
	uint8_t buf[ 128 ];
	socklen_t addr_l = sizeof( struct sockaddr_in );
	int res;

	res = recvfrom( udpfd, ( char* ) buf, sizeof( buf ), 0, ( struct sockaddr* ) &__addr, &addr_l );
	if ( res > 0 ) {
		buf[ res ] = '\x00';
		printf( "*** ( EV3 ) Hello! ***\n%s", buf );
		printf( "IP-Address: %s\n", inet_ntoa( __addr.sin_addr ));
		ev3_brick_addr = NULL;  /* the brick IP address into __addr.sin_addr */
		return ( res );
	}
	return ( 0 );
}

int ev3_tcp_connect( void )
{
	u_long optval = 1;
	u_long ip = __addr.sin_addr.s_addr;  /* save address read from UDP message */

// WIN32 /////////////////////////////////////////
#ifdef __WIN32__
	WSADATA wsa;

	int res = WSAStartup( MAKEWORD( 2, 2 ), &wsa );
	if ( res ) {
		printf( "*** ERROR *** ev3_tcp_connect() = %d\n", res );
		perror( "    WSAStartup()" );
		printf( "\n" );
		sockfd = EOF;
		return ( EOF );
	}

//////////////////////////////////////////////////
#endif
	sockfd = socket( PF_INET, SOCK_STREAM, 0 );
	if ( sockfd < 0 ) {
		printf( "*** ERROR *** ev3_tcp_connect() = %d\n", sockfd );
		perror( "    socket()" );
		printf( "\n" );
		sockfd = EOF;

// WIN32 /////////////////////////////////////////
#ifdef __WIN32__
		WSACleanup();

//////////////////////////////////////////////////
#endif
		return ( EOF );
	}
	bzero( &__addr, sizeof( struct sockaddr_in ));
	__addr.sin_family = AF_INET;
	__addr.sin_port = htons( TCP_PORT );
	__addr.sin_addr.s_addr = ( ev3_brick_addr ) ? inet_addr( ev3_brick_addr ) : ip;

	res = connect( sockfd, ( struct sockaddr* ) &__addr, sizeof( __addr ));
	if ( res < 0 ) {
		printf( "*** ERROR *** ev3_tcp_connect() = %d\n", res );
		perror( "    connect()" );
		printf( "\n" );
		close( sockfd );
		sockfd = EOF;

// WIN32 /////////////////////////////////////////
#ifdef __WIN32__
		WSACleanup();

//////////////////////////////////////////////////
#endif
		return ( EOF );
	}

// WIN32 /////////////////////////////////////////
#ifdef __WIN32__
	ioctlsocket( sockfd, FIONBIO, &optval );

// UNIX //////////////////////////////////////////
#else
	fcntl( sockfd, F_SETFL, O_NONBLOCK );

//////////////////////////////////////////////////
#endif
	return ( 1 );
}

int ev3_tcp_disconnect( void )
{
	if ( sockfd < 0 ) return ( EOF );
	close( sockfd );

// WIN32 /////////////////////////////////////////
#ifdef __WIN32__
	WSACleanup();

//////////////////////////////////////////////////
#endif
	printf( "*** ( EV3 ) Bye! ***\n" );
	return ( 0 );
}

int ev3_tcp_write( uint8_t *msg, int sz )
{
	int res;

#ifdef DEBUG_TCP_MESSAGES
	printf( "> " );
	for ( int i = 0; i < sz; i++ ) printf( "%02X", msg[ i ]);
	printf( "\n" );

#endif
	res = send( sockfd, ( const char* ) msg, sz, 0 );
	if ( res < 0 ) {
		printf( "*** ERROR *** ev3_tcp_write() = %d\n", errno );
		perror( "    send()" );
		printf( "\n" );
		return ( EOF );
	}
	return ( sz );
}

int ev3_tcp_read( uint8_t *buf, int sz )
{
	int res;

	res = recv( sockfd, ( char* ) buf, sz, 0 );

#ifdef DEBUG_TCP_MESSAGES
	if ( res > 0 ) {
		printf( "< " );
		for ( int i = 0; i < res; i++ ) printf( "%02X", buf[ i ]);
		printf( "\n" );
	}

#endif
	if ( res > 0 ) return ( res );

	return ( 0 );
}

int ev3_tcp_unlock_request( void )
{
	return ev3_tcp_write(( uint8_t* ) TCP_UNLOCK_REQUEST, sizeof( TCP_UNLOCK_REQUEST ) - 1 );
}

int ev3_tcp_unlock_reply( void )
{
	uint8_t buf[ 128 ];
	int sz = ev3_tcp_read( buf, sizeof( buf ));

	if ( sz == TCP_UNLOCK_REPLY_SZ ) {
		buf[ TCP_UNLOCK_REPLY_SZ ] = '\x00';
		printf( "%s\n", buf );
		return ( TCP_UNLOCK_REPLY_SZ );
	}
	return ( 0 );
}
