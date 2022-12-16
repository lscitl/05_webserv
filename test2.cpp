#include <string>
#include <iostream>
#include <stdio.h>
#include <unistd.h>
#include <vector>
#include <algorithm>

// void write_inf( int fd ) {
// 	char *str = "asdfzxcvqwer12345";
// 	int   i = 0;

// 	while ( i < 10000 ) {
// 		write( fd, str, 17 );
// 		i++;
// 	}
// 	close( fd );
// 	exit( 0 );
// }

// int main( void ) {
// 	int    p[2];
// 	int    pid;
// 	char   buf[1024];
// 	int    n;
// 	size_t tmp = 0;

// 	pipe( p );

// 	pid = fork();
// 	if ( pid == 0 ) {
// 		// child
// 		close( p[0] );
// 		write_inf( p[1] );
// 	} else {
// 		// parent read
// 		close( p[1] );
// 		sleep( 5 );
// 		while ( 1 ) {
// 			n = read( p[0], buf, 1024 );
// 			if ( tmp > 120000 ) {
// 				break;
// 			}
// 			if ( n != -1 ) {
// 				tmp += n;
// 			}
// 		}
// 		waitpid( pid, NULL, 0 );
// 	}
// }

#include <string>
#include <iostream>
#include <stdio.h>
#include <unistd.h>
#include <vector>
#include <fcntl.h>

int main( void ) {
	char buf[5];
	int  fd = open( "asdf", O_RDONLY | O_CREAT | O_NONBLOCK, 0644 );

	printf( "%d\n", fd );
	read( fd, buf, 5 );
	write( STDOUT_FILENO, buf, 5 );

	printf( "%lu\n", lseek( fd, 0, SEEK_CUR ) );

	read( fd, buf, 5 );
	write( STDOUT_FILENO, buf, 5 );
	printf( "%lu\n", lseek( fd, 0, SEEK_CUR ) );
}
