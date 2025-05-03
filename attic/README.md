# Failed tests

## test_federate_in

works with Chrome but not Firefox.  See this thread: 
https://bugzilla.mozilla.org/show_bug.cgi?id=1387483
https://developer.mozilla.org/en-US/docs/Web/API/Request/body

The old streaming code was:

```c
#if 0


			{ .fd = 1, .events = POLLERR | POLLRDNORM | POLLHUP, .revents = 0 },
		if( fds[1].revents & POLLRDNORM )
		{
			char buf[1024];
			int r = read( fds[1].fd, buf, 1024 );
			FILE * fl = fopen( "../data/log.txt", "a" );
			fprintf( fl, "BUF: %d\n", r );
			fclose( fl );
		}

		// Not possible, because https://bugzilla.mozilla.org/show_bug.cgi?id=1387483
		if( fds[1].revents & POLLRDNORM )
		{
			uint8_t buffer[2048];
			uint8_t lineo[4096];
			int r = read( 1, buffer, sizeof( buffer ) );
			int e = CNURLEncode( lineo, sizeof( lineo )-1, buffer, r );
			printf( "I,%d,%d,%s,\n", r, e, lineo );
			//uint8_t spare[32768];
			//memset( spare, 'a', 32768 );
			//fwrite( spare, 32768, 1, stdout );
			
			FILE * fl = fopen( "../data/log.txt", "a" );
			fprintf( fl, "%s\n", buffer );
			fclose( fl );
			fflush( stdout );
			if( r == 0 )
			{
				printf( "EXIT1\n"); fflush(stdout); sleep(1);
				printf( "EXIT2\n"); fflush(stdout); sleep(1);
				printf( "EXIT3\n"); fflush(stdout); sleep(1);
				printf( "EXIT4\n"); fflush(stdout); sleep(1);
				printf( "EXIT5\n" );
				return 0;
			}
		}
#endif

```

