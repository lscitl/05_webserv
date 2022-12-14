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

int main( void ) {
	std::vector<char> vec;
	std::string       str;
	char              b[20];

	strcpy( b, "sdfzxcvzxcvzxcvwqer" );
	b[4] = '\0';
	b[10] = '\0';
	vec.insert( vec.end(), b, b + 20 );

	str.append( b, 0, 20 );
	for ( std::vector<char>::iterator it = vec.begin(); it != vec.end();
		  ++it ) {
		std::cout << *it;
	}
	std::cout << std::endl;
	for ( std::string::iterator it = str.begin(); it != str.end(); ++it ) {
		std::cout << *it;
	}
	std::cout << std::endl;
}
