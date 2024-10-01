#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>
#include <sqlite3.h>
#include "common.h"
#include "os_generic.h"
#include <fcntl.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

int main()
{
	printf( "Transfer-Encoding: chunked\r\n" );
	printf( "Content-Type: application/octet-stream\r\n\r\n" );

	// Notify any potential listeners.
	int sockfd = socket( AF_INET, SOCK_DGRAM, 0 );
	if( sockfd < 0 )
	{
		printf( "Error: could not notify broad socket\n" );
		return 0;
	}

    struct sockaddr_in server = { 0 };
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = inet_addr( "127.0.0.44" );
	server.sin_port = htons( 7788 );

	int opt = 1;
	if ( setsockopt( sockfd, SOL_SOCKET, SO_REUSEPORT, &opt, sizeof(opt) ) )
	{
		printf( "Error: Could not set socket to reuse %s\n", strerror( errno ) );
		return 0;		
	}

	if( bind( sockfd, (struct sockaddr*)&server, sizeof( server ) ) < 0 )
	{
		printf( "Error: Could not bind to broad socket %s\n", strerror( errno ) );
		return 0;
	}

	// For testing with curl --raw -i https://cnvr.io/monotile/stream.cgi
	printf( "Bound and listening\n" );
	fflush( stdout );

	for(;;)
	{
		uint8_t buf[4000];
		ssize_t r = recv( sockfd, buf, sizeof( buf ), 0 );
		if( r )
		{
		}
			printf( "Received %d\n", r );
			fflush( stdout );
	}

	close( sockfd );

	printf( "Ok" );
	return 0;
}

