#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/uio.h>

#define BUF_LEN 10
#define IOVEC_LEN 3

void set_rbuf_len( struct iovec *buf, ssize_t read_len, int buf_len ) {
	int n = read_len / buf_len;
	int r = read_len % buf_len;
	buf[n].iov_len = r;
}

void reset_buf_len( struct iovec *buf, int iovec_cnt, int buf_len ) {
	for ( int i = 0; i < iovec_cnt; i++ ) {
		buf[i].iov_len = buf_len;
	}
}

int main( void ) {
	struct iovec buf[IOVEC_LEN];
	char         rdbuf1[BUF_LEN];
	char         rdbuf2[BUF_LEN];
	char         rdbuf3[BUF_LEN];
	int          fd = open( "aaa", O_RDWR | O_NONBLOCK, 0777 );
	int          fd2 = open( "bbb", O_RDWR | O_NONBLOCK, 0777 );
	char         asdf[21];

	reset_buf_len( buf, IOVEC_LEN, BUF_LEN );

	int n;
	int n2;
	n = readv( fd, buf, 3 );
	set_rbuf_len( buf, n, BUF_LEN );
	printf( "read: %lu, %lu, %lu\n", buf[0].iov_len, buf[1].iov_len,
			buf[2].iov_len );
	reset_buf_len( buf, IOVEC_LEN, BUF_LEN );
	printf( "len: %d\n", n );
	printf( "write: %lu, %lu, %lu\n", buf[0].iov_len, buf[1].iov_len,
			buf[2].iov_len );
	readv( fd2, buf, 3 );
	set_rbuf_len( buf, n, BUF_LEN );
	printf( "read: %lu, %lu, %lu\n", buf[0].iov_len, buf[1].iov_len,
			buf[2].iov_len );
	writev( STDOUT_FILENO, buf, 3 );
	printf( "\n" );
	printf( "write: %lu, %lu, %lu\n", buf[0].iov_len, buf[1].iov_len,
			buf[2].iov_len );

	// read( fd, asdf, 20 );
	// asdf[20] = 0;
	// printf( "%s\n", asdf );

	// write( fd, "42424242424242424242424", 23 );
	// write( fd, "42424242424242424242424", 23 );

	// read( fd, asdf, 20 );
	// asdf[20] = 0;
	// printf( "%s\n", asdf );
	// read( fd, asdf, 20 );
	// asdf[20] = 0;
	// printf( "%s\n", asdf );
}
