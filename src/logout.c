#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>
#include <sqlite3.h>
#include "common.h"

const char * idcookpre = "identitycookie=";
int main()
{
	char * cookies = strdup( getenv( "HTTP_COOKIE" ) );
	char * ident = strstr( cookies, idcookpre );
	printf( "Set-Cookie: userlogin=\r\n" );
	printf( "Set-Cookie: useravatar=\r\n" );
	printf( "Set-Cookie: identitycookie=\r\n" );
//printf( "Content-Type: text/html\r\n\r\n<html><head><meta charset=\"UTF-8\"></head><body>" );
	printf( "Content-Type: text/html\r\n\r\n<html><head><meta charset=\"UTF-8\"></head><body>" );
	if( !ident )
	{
		printf( "No identity.\n" );
	}
	else
	{
		ident += strlen( idcookpre );
		char * semi = strchr( ident, ';' );
		if( semi ) *semi = 0;

		sqlite3 * db;
		int rc = sqlite3_open( "../data/database/monotile.db", &db);
		if (rc != SQLITE_OK)
		{
			printf( "Could not open database: %s\n", sqlite3_errmsg(db));
			sqlite3_close(db);
			return 0;
		}

		char * errmsg = 0;
		char * query = sqlite3_mprintf( "delete from logins where key='%q';", ident );
		rc = sqlite3_exec( db, query, 0, 0, &errmsg );
		if( rc )
		{
			printf( "Error on query: %s / %s %d\n", query, errmsg, rc );
			sqlite3_close(db);
			return 0;
		}
	}

	printf( "Logged Out.<br>Redirecting: <a href=index.html>Main App</a>" );
	printf( "<script>setTimeout( () => { window.location.href='index.html'; }, 100 );</script>" );
	printf( "</body></html>" );

	return 0;
}
