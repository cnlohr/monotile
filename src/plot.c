#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>
#include <sqlite3.h>
#include "common.h"

int64_t lid = -1;

static int rxnum(void *opaque, int argc, char **argv, char **azColName){
	int i;
	if( argc == 1 )
	{
		*((int64_t*)opaque) = atoi( argv[0] );
	}
	return 0;
}

static int rxstr(void *opaque, int argc, char **argv, char **azColName){
	int i;
	for(i=0; i<argc; i++){
		printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
	}
	printf("\n");
	return 0;
}


int main()
{
	const char * qs_raw = getenv( "QUERY_STRING" );
	if( !qs_raw )
	{
		goto noquerystring;
	}
	char * qs = strdup( qs_raw );

	printf( "Content-Type: text/plain\r\n\r\n" );

	sqlite3 *db;
	int rc = sqlite3_open( "../data/database/monotile.db", &db);
	if (rc != SQLITE_OK)
	{
		printf( "Could not open database: %s\n", sqlite3_errmsg(db));
		sqlite3_close(db);
		return -3;
	}

//	printf( "QUERY_STRING=%s\n", qs );
	char * userkey = CookieCopy( "identitycookie" );
	if( !userkey )
	{
		printf( "Error: Could not get identity cookie\n" );
		return -7;
	}
//	printf( "user=%s\n", userkey );

	char * errmsg = 0;
	char * query = sqlite3_mprintf( "select lid from logins where key='%q';", userkey );
	int recs = 0;
	rc = sqlite3_exec( db, query, rxnum, (void*)&lid, &errmsg );
	if( rc )
	{
		printf( "Error on query: %s / %s %d\n", query, errmsg, rc );
		sqlite3_close(db);
		return -4;
	}
	if( lid <= 0 )
	{
		fprintf( stderr, "Error: Invalid lid\n" );
		return -5;
	}

	printf( "LID: %d\n", lid );



//	char * errmsg = 0;
//	char * query = sqlite3_mprintf( "insert into pixels values ('%q','%q','%q', date('now'));", identitycookie, loginname, userurl );
//	rc = sqlite3_exec( db, query, 0, 0, &errmsg );
//	if( rc )
//	{
//		printf( "Error on query: %s / %s %d\n", query, errmsg, rc );
//		sqlite3_close(db);
//		return 0;
//	}
	return 0;
noquerystring:
	printf( "Content-Type: text/html\r\n\r\n" );
	printf( "Error: Could not get query string\n" );
	return 0;
}

