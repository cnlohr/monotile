#include <stdio.h>
#include <unistd.h>
#include <string.h>

// Testing federation in...
// THIS DOES NOT WORK.

int main()
{
	FILE * ft = fopen( "../data/log.txt", "a" );
	fprintf( ft, "START\n" ); fflush( ft );

	printf( "Transfer-Encoding: chunked\r\n" );
	printf( "X-Accel-Buffering: no\r\n" );
	printf( "cache-control: no-cache\r\n" );
	//printf( "Connection: keep-alive\r\n" );
	//printf( "retry: 1000\r\n" );
	printf( "Content-Type: text/event-stream\r\n\r\n" ); // application/octet-stream
	//printf( "Content-Type: application/octet-stream\r\n\r\n" );
	fflush( stdout );

	//printf( "5\r\nSTART\r\n" );
	while(1)
	{
		char buffer[1024];
		fflush( ft );
		int r = read( 0, buffer, sizeof( buffer ) );
		fflush( ft );
		printf( "10\r\n%10d\r\n", r );
		fprintf( ft, "----\n" );
		fprintf( ft, "%d\n", r );
		if( r <= 0 )
		{
			//fprintf( ft, "Connection closed\n" );
			//fclose( ft );
			break;
		}
		fwrite( buffer, r, 1, ft );
		fprintf( ft, "----\n" );
	}
	printf( "\r\n" );

	return 0;
}
