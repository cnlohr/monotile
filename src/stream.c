#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>
#include <sqlite3.h>
#include "common.h"
#include "os_generic.h"
#include <fcntl.h>
#include <errno.h>
#include <poll.h>
#include <sys/inotify.h>

// For testing with curl --raw -i https://cnvr.io/monotile/stream.cgi

int64_t lastpid = 0;
int64_t firstpid = 0;
int64_t startpid = 0;

static int rxpixel(void *opaque, int argc, char **argv, char **azColName){
	int i;

	if( argc != 5 )
	{
		printf( "Error: Invalid response from sqlite3\n" );
		exit( 0 );
	}

	lastpid = atoi( argv[0] );

	if( lastpid != startpid )
		printf( "P,%s,%s,%s,%s,%s\n", argv[0], argv[1], argv[2], argv[3], argv[4] );

	if( firstpid == 0 )
		firstpid = lastpid;
	return 0;
}


int main()
{
	//char * postdata = ReadPostData();

	printf( "Transfer-Encoding: chunked\r\n" );
	printf( "X-Accel-Buffering: no\r\n" );
	printf( "cache-control: no-cache\r\n" );
	printf( "retry: 1000\r\n" );
	printf( "Connection: keep-alive\r\n" );
	printf( "Content-Type: text/event-stream\r\n\r\n" ); // application/octet-stream

	fflush( stdout );

	sqlite3 *db;
	int rc = sqlite3_open_v2( "../data/database/monotile.db", &db, SQLITE_OPEN_READONLY, 0 );
	if (rc != SQLITE_OK)
	{
		printf( "Error: Could not open database: %s\n", sqlite3_errmsg(db));
		sqlite3_close(db);
		return 0;
	}

	printf( "S,%d,%d,%d\n", GRIDSIZEX, GRIDSIZEY, BLOCKSIZE );
	fflush( stdout );

	startpid = 0; //GetPostFieldInt64( postdata, "startpid" );
	if( startpid <= 0 )
	{
		startpid = 0;
		char * errmsg = 0;
		char * query = sqlite3_mprintf( "select * from (select pid, cx, cy, color, lid from pixels where pid>=%d and removed!=TRUE order by pid desc limit 1) order by pid asc;", startpid );
		int recs = 0;
		firstpid = 0;
		lastpid = 0;
		rc = sqlite3_exec( db, query, rxpixel, 0, &errmsg );
		if( rc )
		{
			printf( "Error: on query: %s / %s %d\n", query, errmsg, rc );
			sqlite3_close(db);
			return 0;
		}
		startpid = lastpid;

		FILE * f = fopen( "../data/grid.dat", "rb" );
		int x, y;
		uint8_t line[GRIDSIZEX+1];
		char lineo[GRIDSIZEX*4+2];
		line[GRIDSIZEX] = '\n';
		for( y = 0; y < GRIDSIZEY; y++ )
		{
			int l = fread( line, GRIDSIZEX, 1, f );
			if( l != 1 )
			{
				printf( "Error: Error reading grid file\n" );
				sqlite3_close(db);
				return 0;
			}

			int e = CNURLEncode( lineo, sizeof( lineo )-1, line, GRIDSIZEX );
			printf( "L,%d,%d,%d,", y, GRIDSIZEX, e );
			lineo[e] = '\n';
			fwrite( lineo, e+1, 1, stdout );
		}
		fclose( f );
	}

	{
		uint8_t buf[1024];
		int rd_from_client = read( 1, buf, sizeof(buf) );
		FILE * fl = fopen( "../data/log.txt", "a" );
		fprintf( fl, "INITIAL RD: %d\n", rd_from_client );
		fclose( fl );
	}

	int inotifyfd = inotify_init1( IN_NONBLOCK );
	int hostname_watch = inotify_add_watch( inotifyfd, "../data/grid.dat", IN_MODIFY | IN_CREATE );
	if( hostname_watch < 0 )
	{
		printf( "Error: inotify cannot watch file\n" );
		sqlite3_close(db);
		return 0;
	}

	for(;;)
	{
		char * errmsg = 0;
		char * query = sqlite3_mprintf( "select * from (select pid, cx, cy, color, lid from pixels where pid>=%d and removed!=TRUE order by pid desc limit 1000) order by pid asc;", startpid );
		int recs = 0;

		firstpid = 0;
		lastpid = 0;
		rc = sqlite3_exec( db, query, rxpixel, 0, &errmsg );
		if( rc )
		{
			printf( "Error: on query: %s / %s %d\n", query, errmsg, rc );
			sqlite3_close(db);
			return 0;
		}
		if( startpid != firstpid && startpid > 0 )
		{
			printf( "Error: Ran off the end of the list (%ld %ld %ld)\n", startpid, lastpid, firstpid );
			return 0;
		}

		startpid = lastpid;

		fflush( stdout );

		struct pollfd fds[2] = {
			{ .fd = inotifyfd, .events = POLLIN,            .revents = 0 },
			{ .fd = 0,         .events = POLLERR | POLLHUP, .revents = 0 },
		};

		// Make poll wait for literally forever.
		int r = poll( fds, 2, 6000 );

		//FILE * fl = fopen( "../data/log.txt", "a" );
		//fprintf( fl, "%d %x %x\n", r, fds[0].revents, fds[1].revents );
		//fclose( fl );

		if( r == 0 )
		{
			printf( "K\n" );
			fflush( stdout );
			continue;
		}

		if( ( fds[1].revents ) & ( POLLHUP | POLLERR ) )
		{
			// Disconnected from other end.
			return 0;
		}

		if ( fds[0].revents & POLLIN )
		{
			struct inotify_event event;
			r = read( inotifyfd, &event, sizeof( event ) );
			printf( "INOTIFY,%d\n", r );
			fflush( stdout );
			if( r < 12 )
			{
				printf( "Error: Confusing inotify message\n" );
				return 0;
			}
		}

	}

	return 0;
}

