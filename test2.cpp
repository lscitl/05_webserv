#include <string>
#include <iostream>
#include <stdio.h>

int main( void ) {
	std::string tmp = "0123456789012345678901234567890123456789012345611";
	std::string a;

	printf( "%d, %d\n", tmp.capacity(), a.capacity() );
	// tmp.c_str();
	a += tmp;
	a.clear();
	printf( "%d, %d\n", tmp.capacity(), a.capacity() );
	printf( "%p, %p\n", tmp.substr( 0, 5 ).c_str(), tmp.c_str() );
}
