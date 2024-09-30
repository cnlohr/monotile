#include <stdio.h>
#include <unistd.h>
#include <string.h>
char res[1024];
int main()
{
	printf( "Transfer-Encoding: chunked\r\n" );
	printf( "Content-Type: application/octet-stream\r\n\r\n" );
	fflush( stdout );
	memset( res, 'x', sizeof( res ) );
	for( int i = 0; i < 500000; i++ )
	{
		write( 0, res, sizeof( res ) );
		usleep(1);
	}
	return 0;
}
