#ifndef _COMMON_H
#define _COMMON_H

#include <string.h>
#include <errno.h>
#include <limits.h>

#define BLOCKSIZE 32
#define GRIDSIZEX 2048
#define GRIDSIZEY 2048

static char * CookieCopy( const char * cookiename )
{
	const char * cookie_raw = getenv( "HTTP_COOKIE" );
	if( !cookie_raw ) return 0;
	int matchlen = strlen( cookiename ) + 4;
	char * matchstr = alloca( matchlen );
	sprintf( matchstr, "; %s=", cookiename );
	const char * match = 0;
	if( memcmp( cookie_raw, matchstr+2, matchlen - 3 ) == 0 )
	{
		match = cookie_raw;
		matchlen -= 2;
	}
	else
	{
		match = strstr( cookie_raw, matchstr );
		if( !match ) return 0;
	}
	match += matchlen - 1;
	const char * matchend = strstr( match, ";" );
	if( !matchend )
		matchend = match + strlen( match );
	int valuelen = matchend - match + 1;
	char * ret = malloc( valuelen );
	memcpy( ret, match, valuelen-1 );
	ret[valuelen-1] = 0;
	return ret;
}

static char * ReadPostData()
{
	char * postdata = malloc(1);
	postdata[0] = 0;
	int postdatalen = 0;
	for(;;)
	{
		int c = fgetc( stdin );
		if( c != EOF )
		{
			postdata[postdatalen++] = c;
			postdata = realloc( postdata, postdatalen+1 );
			postdata[postdatalen] = 0;
		}
		else
		{
			break;
		}
	}
	return postdata;
}

// Returns malloc'd memory, also still url-encoded.
static char * GetPostField( const char * request, const char * fieldid )
{
	if( !request ) return 0;
	int matchlen = strlen( fieldid ) + 3;
	char * matchstr = alloca( matchlen );
	sprintf( matchstr, "&%s=", fieldid );
	const char * match = 0;
	if( memcmp( request, matchstr+1, matchlen - 2 ) == 0 )
	{
		match = request;
		matchlen -= 1;
	}
	else
	{
		match = strstr( request, matchstr );
		if( !match ) return 0;
	}
	match += matchlen - 1;
	const char * matchend = strstr( match, "&" );
	if( !matchend )
		matchend = match + strlen( match );
	int valuelen = matchend - match + 1;
	char * ret = malloc( valuelen );
	memcpy( ret, match, valuelen-1 );
	ret[valuelen-1] = 0;
	return ret;
}

long long GetPostFieldInt64( const char * request, const char * fieldid )
{
	if( !request ) return LLONG_MIN;
	int matchlen = strlen( fieldid ) + 3;
	char * matchstr = alloca( matchlen );
	sprintf( matchstr, "&%s=", fieldid );
	const char * match = 0;
	if( memcmp( request, matchstr+1, matchlen - 2 ) == 0 )
	{
		match = request;
		matchlen -= 1;
	}
	else
	{
		match = strstr( request, matchstr );
		if( !match ) return LLONG_MIN;
	}
	match += matchlen - 1;
	const char * matchend = strstr( match, "&" );
	if( !matchend )
		matchend = match + strlen( match );
	int valuelen = matchend - match + 1;
	char * ret = malloc( valuelen );
	memcpy( ret, match, valuelen-1 );
	ret[valuelen-1] = 0;

	char * retend = 0;
	long long retval = strtol( ret, &retend, 10 );
	if( errno == ERANGE )
	{
		return LLONG_MIN;
	}
	return retval;
}


static int isclean( const char * str )
{
	for( char c; c = *str; str++ )
	{
		switch( c )
		{
		case 0 ... 47: return 0;
		case 58 ... 64: return 0;
		case 91 ... 94: return 0;
		case 95: break; // Underscores OK
		case 96: return 0;
		default:
			if( c >= 123 ) return 0;
		}
	}
	return 1;
}



static const char tohex1buff[16] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f' };

static unsigned char hex1byte( const char data )
{
	if( data >= '0' && data <= '9' )
		return (data - '0');
	else if( data >= 'a' && data <= 'f' )
		return (data - 'a'+10);
	else if( data >= 'A' && data <= 'F' )
		return (data - 'A'+10);
	else
		return 16;
}

static unsigned char hex2byte( const char * data )
{
	return (hex1byte(data[0])<<4) | (hex1byte(data[1]));
}

static int CNURLEncode( char * encodeinto, int maxlen, const char * buf, int buflen )
{
	int i = 0;
	for( ; buf && *buf; buf++ )
	{
		char c = *buf;
		if( c == ' ' )
		{
			encodeinto[i++] = '+';
		}
		else if( c < 46 || c > 126 || c == 96 )
		{
			encodeinto[i++] = '%';
			encodeinto[i++] = tohex1buff[c>>4];
			encodeinto[i++] = tohex1buff[c&15];
			break;
		}
		else
		{
			encodeinto[i++] = c;
		}
		if( i >= maxlen - 3 )  break;
	}
	encodeinto[i] = 0;
	return i;
}

static int CNURLDecode( char * decodeinto, int maxlen, const char * buf )
{
	int i = 0;

	for( ; buf && *buf; buf++ )
	{
		char c = *buf;
		if( c == '+' )
		{
			decodeinto[i++] = ' ';
		}
		else if( c == '?' || c == '&' )
		{
			break;
		}
		else if( c == '%' )
		{
			if( *(buf+1) && *(buf+2) )
			{
				decodeinto[i++] = hex2byte( buf+1 );
				buf += 2;
			}
		}
		else
		{
			decodeinto[i++] = c;
		}
		if( i >= maxlen -1 )  break;

	}
	decodeinto[i] = 0;
	return i;
}

#endif

