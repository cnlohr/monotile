#include <stdio.h>
#include <unistd.h>
#include <string.h>

// Test speed of chunked throughput
// curl --raw -i https://cnvr.io/monotile/test_federate.cgi > /dev/null

char res[16384];

int main()
{
	printf( "Transfer-Encoding: chunked\r\n" );
	printf( "Content-Type: application/octet-stream\r\n\r\n" );
	fflush( stdout );
	memset( res, 'x', sizeof( res ) );
	for( int i = 0; i < 50000; i++ )
	{
		write( 0, res, sizeof( res ) );
		usleep(1);
	}
	write( 0, "done\n", 5); 
	return 0;
}
