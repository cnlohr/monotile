#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//#include <curl/curl.h>
//#include <sqlite3.h>
#include "common.h"

//const char * idcookpre = "identitycookie=";
int main()
{
	//char * cookies = strdup( getenv( "HTTP_COOKIE" ) );
	//char * ident = strstr( cookies, idcookpre );
	printf( "Set-Cookie: identitycookie=\r\n" );
	printf( "Content-Type: text/html\r\n\r\n<html><head><meta charset=\"UTF-8\"></head><body>" );
	printf( "Test</body></html>" );
	return 0;
}
