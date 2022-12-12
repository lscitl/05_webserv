#include <string>
#include <iostream>
#include <stdio.h>
#include <unistd.h>

void write_inf( int fd ) {
	char *str = "asdfzxcvqwer12345";
	int   i = 0;

	while ( i < 10000 ) {
		write( fd, str, 17 );
		i++;
	}
	close( fd );
	exit( 0 );
}

int main( void ) {
	int    p[2];
	int    pid;
	char   buf[1024];
	int    n;
	size_t tmp = 0;

	pipe( p );

	pid = fork();
	if ( pid == 0 ) {
		// child
		close( p[0] );
		write_inf( p[1] );
	} else {
		// parent read
		close( p[1] );
		// sleep( 5 );
		while ( 1 ) {
			n = read( p[0], buf, 1024 );
			if ( tmp > 100000 ) {
				break;
			}
			if ( n != -1 ) {
				tmp += n;
			}
		}
		waitpid( pid, NULL, 0 );
	}

	// std::string tmp = "0123456789012345678901";
	// std::string a;

	// printf( "%d, %d\n", tmp.capacity(), a.capacity() );
	// // tmp.c_str();
	// a += tmp;
	// a.clear();
	// printf( "%d, %d\n", tmp.capacity(), a.capacity() );
	// printf( "%p, %p\n", tmp.c_str(), tmp.c_str() );
	// printf( "%d, %d\n", tmp.capacity(), a.capacity() );

	// const char *str = tmp.c_str();
	// printf( "%d, %d\n", tmp.capacity(), a.capacity() );
	// printf( "%p, %p\n", str, tmp.c_str() );

	// printf( "%d\n", str[22] );
}
