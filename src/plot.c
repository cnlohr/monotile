#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>
#include <sqlite3.h>
#include "common.h"
#include "os_generic.h"
#include <fcntl.h>
#include <errno.h>

int64_t lid = -1;

static int rxnum(void *opaque, int argc, char **argv, char **azColName){
	int i;
	if( argc == 1 )
	{
		*((int64_t*)opaque) = atoi( argv[0] );
	}
	return 0;
}

int main()
{
	char * postdata = ReadPostData();
	printf( "Content-Type: text/plain\r\n\r\n" );

	sqlite3 *db;
	int rc = sqlite3_open( "../data/database/monotile.db", &db);
	if (rc != SQLITE_OK)
	{
		printf( "Error: Could not open database: %s\n", sqlite3_errmsg(db));
		sqlite3_close(db);
		return 0;
	}

//	printf( "QUERY_STRING=%s\n", qs );
	char * userkey = CookieCopy( "identitycookie" );
	if( !userkey )
	{
		printf( "Error: Could not get identity cookie\n" );
		return 0;
	}
//	printf( "user=%s\n", userkey );
	char * errmsg = 0;
	char * query = sqlite3_mprintf( "select lid from logins where key='%q' and valid=TRUE;", userkey );
	int recs = 0;
	rc = sqlite3_exec( db, query, rxnum, (void*)&lid, &errmsg );
	if( rc )
	{
		printf( "Error: on query: %s / %s %d\n", query, errmsg, rc );
		sqlite3_close(db);
		return 0;
	}
	if( lid <= 0 )
	{
		printf( "Error: Invalid Login ID\n" );
		return 0;
	}

	int64_t cx = GetPostFieldInt64( postdata, "cx" );
	int64_t cy = GetPostFieldInt64( postdata, "cy" );
	int64_t color = GetPostFieldInt64( postdata, "color" );

	if( cx == LLONG_MIN || cy == LLONG_MIN || color == LLONG_MIN )
	{
		printf( "Error parsing arguments; request: %s\n", postdata );
		return 0;
	}

	if( cx < 0 || cx >= GRIDSIZEX || cy < 0 || cy >= GRIDSIZEY )
	{
		printf( "Error: Pixel out of range\n" );
		return 0;
	}

	int64_t bx = cx / BLOCKSIZE;
	int64_t by = cy / BLOCKSIZE;

	// Lock semaphore
	sem_t * s = sem_open( "monotile", O_CREAT, 0777, 1 );
	struct timespec ts;
	if (clock_gettime(CLOCK_REALTIME, &ts) == -1)
	{
		printf( "Error: Local clock error\n" );
		return 0;
	}
	ts.tv_sec += 5; // Wait up to 5 seconds for access.
	int s_status = sem_timedwait( s, &ts );
	if( s_status )
	{
		int en = errno;
		FILE * fl = fopen( "../data/log.txt", "a" );
		fprintf( fl, "Error: Semaphore Lock Failure (%s)\n", strerror( en ) );
		fclose( fl );
	}
	// If we timed out, something went wrong anyway, just plow onward and release when done to shake loose a possible crash.

	query = sqlite3_mprintf( "insert into pixels( lid, cx, cy, bx, by, dtime, color, removed ) values (%lld,%d,%d,%d,%d,%f,%d,FALSE);",
		lid, cx, cy, bx, by, OGGetAbsoluteTime(), color );
	rc = sqlite3_exec( db, query, 0, 0, &errmsg );
	if( rc )
	{
		printf( "Error: on query: %s / %s %d\n", query, errmsg, rc );
		sqlite3_close(db);
		sem_post( s );
		return 0;
	}
	sqlite3_close(db);

	int byteno = cx + cy * GRIDSIZEX;
	FILE * fGrid = fopen("../data/grid.dat", "r+b"); // r+ if you need char mode
	if( !fGrid )
	{
		printf( "Error: permissions setting internally wrong for grid file\n" );
		sem_post( s );
		sem_close( s );
		return 0;
	}

	fseek(fGrid, byteno, SEEK_SET);
	fwrite( &color, 1, 1, fGrid);
	fclose( fGrid );

	sem_post( s );
	sem_close( s );
	printf( "Ok" );
	return 0;

noquerystring:
	printf( "Content-Type: text/html\r\n\r\n" );
	printf( "Error: Could not get query string\n" );
	return 0;
}

