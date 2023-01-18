#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>
#include <sqlite3.h>
#include "common.h"

struct strpair_t { int len; char * str; };

size_t curlback_strpair( char *ptr, size_t size, size_t nmemb, void *userdata )
{
	struct strpair_t * tp = userdata;
	int tocopy = size * nmemb;
	tp->str = realloc( tp->str, tp->len + tocopy + 1 );
	memcpy( tp->str + tp->len, ptr, tocopy );
	tp->len += tocopy;
	tp->str[tp->len] = 0;
	return tocopy;
}

int main( int argc, char ** argv )
{
	char identitycookie[65];
	int i;
	{
		int rd;
		unsigned char rrd[64] = { 0 };
		FILE * dr = fopen( "/dev/random", "rb" );
		rd = fread( rrd, 64, 1, dr );
		if( rd != 1 ) goto norand;
		fclose( dr );
		for( i = 0; i < 64; i++ )
		{
			sprintf( identitycookie + i*2, "%02x", rrd[i] );
		}
		identitycookie[i] = 0;
	}
	char * qs = strdup( getenv( "QUERY_STRING" ) );
	//Extract "Code"
	char * code = strstr( qs, "code=" );
	if( !code ) goto badcode;
	char * end = strchr( code, '&' );
	if( !end ) goto badcode;
	*end = 0;
	if( !isclean( code + 5 ) ) goto badcode;
	if( strlen( code ) < 1 ) goto badcode;

	char authline[1024];
	FILE * fPrivateAppAuth = fopen( "../data/.privateappauth", "r" );
	if( !fPrivateAppAuth ) goto badauthinternal;
	if( 0 == fgets( authline, 1024, fPrivateAppAuth ) ) goto badauthinternal;
	char * authnewline = strchr( authline, '\n' );
	if( authnewline ) *authnewline = 0;
	fclose( fPrivateAppAuth );

	CURLcode res;

	char * access_token = 0;

	struct strpair_t auth_reply = { 0 };
	{
		const char * github_accesstoken_page = "https://github.com/login/oauth/access_token";
		int reqlinelen = strlen( github_accesstoken_page ) + strlen( code ) + strlen( authline ) + 2;
		char * aturl = alloca( reqlinelen + 1 );
		snprintf( aturl, reqlinelen + 1, "%s?%s&%s", github_accesstoken_page, code, authline );

		free( qs ); // No longer needed.

		CURL * curl = curl_easy_init();
		if( !curl ) goto internalcurlerror;
		curl_easy_setopt( curl, CURLOPT_URL, aturl );
		curl_easy_setopt( curl, CURLOPT_WRITEFUNCTION, curlback_strpair );
		curl_easy_setopt( curl, CURLOPT_WRITEDATA, &auth_reply );
		res = curl_easy_perform(curl);
		curl_easy_cleanup(curl);
		if( res ) goto curlcodefail;
		if( auth_reply.len == 0 ) goto internalcurlerror2;
		if( strstr( auth_reply.str, "error=" ) ) goto authreplyfail;
		access_token = strstr( auth_reply.str, "access_token=" );
		if( !access_token ) goto authreplyfail;
		access_token += strlen( "access_token=" );
		char * access_token_end = strchr( access_token, '&' );
		if( access_token_end ) *access_token_end = 0;
		if( !isclean( access_token ) ) goto authreplyfail;
	}

	struct strpair_t user_data = { 0 };
	char * loginname = 0;
	char * avatarurl = 0;
	{
		const char * authpref = "Authorization: Bearer ";
		int reqlinelen = strlen( authpref ) + strlen( access_token ) + 1;
		char * authreq = alloca( reqlinelen + 1 );
		snprintf( authreq, reqlinelen, "%s%s", authpref, access_token );

		CURL * curl = curl_easy_init();
		if( !curl ) goto internalcurlerror;

		struct curl_slist *list = NULL;
		list = curl_slist_append( list, authreq );
		list = curl_slist_append( list, "User-Agent: cURL" );
		curl_easy_setopt( curl, CURLOPT_HTTPHEADER, list );
		curl_easy_setopt( curl, CURLOPT_URL, "https://api.github.com/user" );
		curl_easy_setopt( curl, CURLOPT_WRITEFUNCTION, curlback_strpair );
		curl_easy_setopt( curl, CURLOPT_WRITEDATA, &user_data );
		res = curl_easy_perform(curl);
		curl_easy_cleanup(curl);
		if( res ) goto curlcodefail;
		if( user_data.len == 0 ) goto internalcurlerror2;
		// user_data should be filled out, now.  Extract user login name.
		const char * loginnamebase = strstr( user_data.str, "\"login\":\"" );
		if( !loginnamebase ) goto userloginmissing;
		loginnamebase += strlen( "\"login\":\"" );
		const char * liend = strchr( loginnamebase, '\"' );
		if( !liend ) goto userloginmissing;
		int loginnamelen = liend - loginnamebase;

		loginname = malloc( loginnamelen + 1 );
		memcpy( loginname, loginnamebase, loginnamelen );
		loginname[loginnamelen] = 0;

		const char * avu = "\"avatar_url\":\"";
		int avulen = strlen( avu );
		const char * avatarurlbase = strstr( user_data.str, avu );
		if( avatarurlbase )
		{
			avatarurlbase += avulen;
			const char * aue = strchr( avatarurlbase, '\"' );
			if( aue )
			{
				int al = aue - avatarurlbase;
				avatarurl = malloc( al+1 );
				memcpy( avatarurl, avatarurlbase, al );
				avatarurl[al] = 0;
			}
		}
	}

	{
		int loginnamelen = strlen( loginname );
		const char * loginnamecookieprefix = "Set-Cookie: userlogin=";
		int prefixnamelen = strlen( loginnamecookieprefix );
		int alen = loginnamelen*3 + 4 + prefixnamelen;
		char * setcookiestr = alloca( alen + 1 );
		sprintf( setcookiestr, "%s", loginnamecookieprefix );
		CNURLEncode( setcookiestr + prefixnamelen, loginnamelen*3, loginname, loginnamelen );
		printf( "%s\r\n", setcookiestr );
	}
	if( avatarurl )
	{
		int avatarurllen = strlen(avatarurl);
		const char * loginavatarcookieprefix = "Set-Cookie: useravatar=";
		int prefixavatarlen = strlen( loginavatarcookieprefix );
		int alen = avatarurllen*3 + 4 + prefixavatarlen;
		char * setcookiestr = alloca( alen + 1 );
		sprintf( setcookiestr, "%s", loginavatarcookieprefix );
		CNURLEncode( setcookiestr + prefixavatarlen, avatarurllen*3, avatarurl, avatarurllen );
		printf( "%s\r\n", setcookiestr );
	}

	printf( "Set-Cookie: identitycookie=%s\r\n", identitycookie );
	printf( "Content-Type: text/html\r\n\r\n<html><head><meta charset=\"UTF-8\"></head><body>" );
	printf( "User Login: %s<br>", loginname );
	// Pair loginname with identitycookie

	sqlite3 *db;
	int rc = sqlite3_open( "../data/monotile.db", &db);
	if (rc != SQLITE_OK)
	{
		printf( "Could not open database: %s\n", sqlite3_errmsg(db));
		sqlite3_close(db);
		return 0;
	}

	char * errmsg = 0;
	char * query = sqlite3_mprintf( "insert into logins values ('%q','%q', date('now'));", identitycookie, loginname );
	rc = sqlite3_exec( db, query, 0, 0, &errmsg );
	if( rc )
	{
		printf( "Error on query: %s / %s %d\n", query, errmsg, rc );
		sqlite3_close(db);
		return 0;
	}

	printf( "Redirecting: <a href=index.html>Main App</a>" );
	printf( "<script>setTimeout( () => { window.location.href='index.html'; }, 100 );</script>" );
//	printf( "<xmp>%s</xmp>", user_data.str );
//	printf( "<xmp>%s</xmp>", avatarurl );
	printf( "</body></html>" );
	sqlite3_close(db);

	return 0;
badcode:
	printf( "Error: Auth failed.  No Access Code.\n" );
	return 0;
norand:
	printf( "Content-Type: text/html\r\n\r\nError: Cookie generation code failed.\n" );
	return 0;
badauthinternal:
	printf( "Error: Bad Internal Auth Configuration\n" );
	return 0;
internalcurlerror:
	printf( "Error: Internal CURL error.\n" );
	return 0;
internalcurlerror2:
	printf( "Error: Internal CURL error (in reply).\n" );
	return 0;
curlcodefail:
	printf( "Error: Curl code fail: %d\n", res );
	return 0;
authreplyfail:
	printf( "Error: Remote Auth: %s\n", auth_reply.str );
	return 0;
userloginmissing:
	printf( "Error: Remote Auth: %s\n", auth_reply.str );
	return 0;
}

