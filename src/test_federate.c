#include <stdio.h>
#include <unistd.h>
#include <string.h>

// Test speed of chunked throughput
// curl --raw -i https://cnvr.io/monotile/test_federate.cgi > /dev/null

char res[16384];

int main()
{
	printf( "Transfer-Encoding: chunked\r\n" );
	printf( "X-Accel-Buffering: no\r\n" );
	printf( "cache-control: no-cache\r\n" );
	printf( "Content-Type: application/octet-stream\r\n\r\n" );
	fflush( stdout );
	memset( res, 'x', sizeof( res ) );
	for( int i = 0; i < 50000; i++ )
	{
		printf( "%d\r\n", 16384 );
		write( 0, res, sizeof( res ) );
		printf( "\r\n" );
		usleep(1);
	}
	printf( "\r\n" );
	return 0;
}
