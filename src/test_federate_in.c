#include <stdio.h>
#include <unistd.h>
#include <string.h>

// Testing federation in...
// THIS DOES NOT WORK.

int main()
{
	printf( "Transfer-Encoding: chunked\r\n" );
	printf( "X-Accel-Buffering: no\r\n" );
	printf( "cache-control: no-cache\r\n" );
	printf( "Content-Type: application/octet-stream\r\n\r\n" );
	fflush( stdout );

	FILE * ft = fopen( "/home/cnlohr/projects/monotile/data/test.txt", "a" );
	fprintf( ft, "START\n" );
	while(1)
	{
		char buffer[1024];
		int r = read( 0, buffer, sizeof( buffer ) );
		if( r < 0 )
		{
			fprintf( ft, "Connection closed\n" );
			fclose( ft );
			break;
		}
		fwrite( buffer, r, 1, ft );
	}

	return 0;
}
