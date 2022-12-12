#include <sys/socket.h>
#include <sys/event.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/uio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <iostream>
#include <string>
#include <vector>
#include <queue>
#include <map>

#define SERV_PORT 1234
#define SERV_SOCK_BACKLOG 10
#define SERV_RESPONSE                                                    \
	"HTTP/1.1 200 KO\r\nContent-Type: text/plain\r\nTransfer-Encoding: " \
	"chunked\r\n\r\n4\r\nghan\r\n6\r\njiskim\r\n8\r\nyongjule\r\n0\r\n\r\n"
#define BUFFER_SIZE 16 * 1024

enum {
	SOCK_READ = 1,
	HEAD_END_FLAG = 2,
	SOCK_WRITE = 4,
	GET_RESPONSE = 8,
	HEAD_RESPONSE = 16,
	POST_RESPONSE = 32,
	PUT_RESPONSE = 64,
	DELETE_RESPONSE = 128,
	READ_BODY = 256,
	REQ_LINE_PARSED = 512,
	RESPONSE_BODY = 1024,
	NONE = 0
};

enum {
	REQ_GET = 1,
	REQ_HEAD = 2,
	REQ_POST = 4,
	REQ_PUT = 8,
	REQ_DELETE = 16,
	REQ_UNDEFINED = 32
};

void error_exit( std::string err, int ( *func )( int ), int fd ) {
	std::cerr << strerror( errno ) << std::endl;
	if ( func != NULL ) {
		func( fd );
	}
	exit( EXIT_FAILURE );
}

void add_event_change_list( std::vector<struct kevent> &changelist,
							uintptr_t ident, int64_t filter, uint16_t flags,
							uint32_t fflags, intptr_t data, void *udata ) {
	struct kevent tmp_event;
	EV_SET( &tmp_event, ident, filter, flags, fflags, data, udata );
	changelist.push_back( tmp_event );
}

void disconnect_client( int                                client_fd,
						std::map<uintptr_t, t_client_buf> &clients ) {
	std::cout << "client disconnected: " << client_fd << std::endl;
	close( client_fd );
	clients.erase( client_fd );
}

typedef struct ResponseField {
	std::string response_;
	int         body_flag_;
	std::string file_path_;
	int         sent_pos_;
} t_response_field;

typedef struct ClientBuffer {
	std::map<std::string, std::string> field_;
	std::queue<t_response_field>       response_queue_;
	std::string                        rdsaved_;
	std::string                        response_;
	timespec                           timeout_;
	uintptr_t                          client_fd;
	char                               rbuf_[BUFFER_SIZE];
	int                                rdchecked_;
	int                                wrchecked_;
	int                                req_type_;
	int                                flag_;

	ClientBuffer()
		: field_(),
		  response_queue_(),
		  rdsaved_(),
		  response_(),
		  timeout_(),
		  client_fd(),
		  rbuf_(),
		  rdchecked_( 0 ),
		  wrchecked_( 0 ),
		  req_type_( 0 ),
		  flag_( 0 ) {
	}
	~ClientBuffer() {
	}
} t_client_buf;

bool request_line_check( std::string &req_line, t_client_buf &buf ) {
	return true;
}

bool request_line_parser( uintptr_t fd, t_client_buf &buf ) {
	std::string req_line;
	std::string req_target;
	std::string req_type;
	size_t      crlf_pos;

	while ( buf.flag_ & REQ_LINE_PARSED == false ) {
		// if ( buf.rdsaved_.size() ) {
		crlf_pos = buf.rdsaved_.find( "\r\n" );
		if ( crlf_pos != std::string::npos ) {
			req_line = buf.rdsaved_.substr( buf.rdchecked_, crlf_pos );
			buf.rdchecked_ = crlf_pos + 2;
			if ( request_line_check( req_line, buf ) == false ) {
				// bad request
				return false;
			}
			switch ( buf.req_type_ ) {
				case REQ_GET:
					break;
				case REQ_HEAD:
					break;
				case REQ_POST:
					break;
				case REQ_PUT:
					break;
				case REQ_DELETE:
					break;
				case REQ_UNDEFINED:
					break;
				default:
					if ( header_field_parser( fd, buf ) == false ) {
						// need to read more.
					} else {
					}
					break;
			}
			buf.flag_ |= REQ_LINE_PARSED;
			break;
		}
		buf.rdsaved_ = buf.rdsaved_.erase( 0, buf.rdchecked_ );
		buf.rdchecked_ = 0;
		buf.flag_ |= SOCK_READ;
		return false;
	}
	return true;
}

bool header_valid_check( std::string &key_val, size_t col_pos ) {
	// check logic
	return true;
}

bool header_field_parser( uintptr_t fd, t_client_buf &buf ) {
	std::string header_field_line;
	size_t      crlf_pos;

	while ( buf.flag_ & HEAD_END_FLAG == false ) {
		// if ( buf.rdsaved_.size() ) {
		crlf_pos = buf.rdsaved_.find( "\r\n" );
		if ( crlf_pos != std::string::npos ) {
			header_field_line = buf.rdsaved_.substr( buf.rdchecked_, crlf_pos );
			buf.rdchecked_ = crlf_pos + 2;
			if ( header_field_line.size() == 0 ) {
				buf.flag_ |= HEAD_END_FLAG;
				break;
			}
			crlf_pos = header_field_line.find( ":" );
			if ( crlf_pos != std::string::npos ) {
				size_t pos2 = crlf_pos + 1;
				// to do
				if ( header_valid_check( header_field_line, crlf_pos ) ) {
					while ( header_field_line[pos2] == ' ' ||
							header_field_line[pos2] == '\t' ) {
						++pos2;
					}
					buf.field_[header_field_line.substr( 0, crlf_pos )] =
						header_field_line.substr(
							pos2, header_field_line.size() - pos2 );
				}
			}
			continue;
		}
		buf.rdsaved_ = buf.rdsaved_.erase( 0, buf.rdchecked_ );
		buf.rdchecked_ = 0;
		buf.flag_ |= SOCK_READ;
		return false;
		// }
		// return false;
	}
	buf.flag_ &= ~HEAD_END_FLAG;
	return true;
}

bool read_client_fd( uintptr_t fd, t_client_buf &buf ) {
	char    strbuf[8196];
	ssize_t n;

	if ( buf.flag_ & SOCK_READ ) {
		n = read( fd, strbuf, 1024 );
		if ( n <= 0 ) {
			// client error??
			return false;
		}
		buf.rdsaved_.append( strbuf, buf.rdsaved_.size(), n );
		return true;
	}
	return true;
}

bool write_client_fd( uintptr_t fd, t_client_buf &buf ) {
}

bool create_client_event( uintptr_t serv_sd, struct kevent *cur_event,
						  std::vector<struct kevent>        &changelist,
						  std::map<uintptr_t, t_client_buf> &clients ) {
	uintptr_t client_fd;
	if ( ( client_fd = accept( serv_sd, NULL, NULL ) ) == -1 ) {
		std::cerr << strerror( errno ) << std::endl;
		return false;
	} else {
		std::cout << "accept new client: " << client_fd << std::endl;
		fcntl( client_fd, F_SETFL, O_NONBLOCK );
		t_client_buf *new_buf = new t_client_buf();
		add_event_change_list( changelist, client_fd, EVFILT_READ,
							   EV_ADD | EV_ENABLE, 0, 0, new_buf );
		add_event_change_list( changelist, client_fd, EVFILT_WRITE,
							   EV_ADD | EV_DISABLE, 0, 0, new_buf );
		return true;
	}
}

int main( void ) {
	int                serv_sd;
	struct sockaddr_in serv_addr;

	serv_sd = socket( PF_INET, SOCK_STREAM, IPPROTO_TCP );
	if ( serv_sd == -1 ) {
		error_exit( "socket()", NULL, 0 );
	}

	int opt = 1;
	if ( setsockopt( serv_sd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof( opt ) ) ==
		 -1 ) {
		error_exit( "setsockopt()", close, serv_sd );
	}

	// if ( fcntl( serv_sd, F_SETFL, O_NONBLOCK ) == -1 ) {
	// 	error_exit( "fcntl()", close, serv_sd );
	// }

	memset( &serv_addr, 0, sizeof( serv_addr ) );
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons( SERV_PORT );
	serv_addr.sin_addr.s_addr = htonl( INADDR_ANY );

	if ( bind( serv_sd, (struct sockaddr *)&serv_addr, sizeof( serv_addr ) ) ==
		 -1 ) {
		error_exit( "bind", close, serv_sd );
	}

	if ( listen( serv_sd, SERV_SOCK_BACKLOG ) == -1 ) {
		error_exit( "bind", close, serv_sd );
	}

	std::map<uintptr_t, t_client_buf> clients;
	std::vector<struct kevent>        changelist;
	struct kevent                     eventlist[8];
	int                               kq;

	kq = kqueue();
	if ( kq == -1 ) {
		error_exit( "kqueue()", close, serv_sd );
	}

	add_event_change_list( changelist, serv_sd, EVFILT_READ, EV_ADD | EV_ENABLE,
						   0, 0, NULL );

	int            event_len;
	struct kevent *cur_event;
	int            l = 0;
	while ( true ) {
		event_len = kevent( kq, changelist.begin().base(), changelist.size(),
							eventlist, 8, NULL );
		if ( event_len == -1 ) {
			error_exit( "kevent()", close, serv_sd );
		}
		changelist.clear();

		// std::cout << "current loop: " << l++ << std::endl;

		for ( int i = 0; i < event_len; ++i ) {
			cur_event = &eventlist[i];

			if ( cur_event->flags & EV_ERROR ) {
				if ( cur_event->ident == serv_sd ) {
					error_exit( "server socket error", close, serv_sd );
				} else {
					std::cerr << "client socket error" << std::endl;
					disconnect_client( cur_event->ident, clients );
				}
			} else if ( cur_event->filter == EVFILT_READ ) {
				if ( cur_event->ident == serv_sd ) {
					if ( create_client_event( serv_sd, cur_event, changelist,
											  clients ) == false ) {
						// error ???
					}
				} else {
					t_client_buf *buf =
						static_cast<t_client_buf *>( cur_event->udata );
					ssize_t n_read;
					std::cout << "recieved data from " << cur_event->ident
							  << ":" << std::endl;

					n_read = read( cur_event->ident, buf->rbuf_, BUFFER_SIZE );
					if ( n_read < 0 ) {
						continue;
					}
					buf->rdsaved_.append( buf->rbuf_, buf->rdsaved_.size(),
										  n_read );
					if ( request_line_parser( cur_event->ident, *buf ) ==
						 false ) {
						continue;
					}
					add_event_change_list( changelist, cur_event->ident,
										   EVFILT_WRITE, EV_ENABLE, 0, 0, buf );
					add_event_change_list( changelist, cur_event->ident,
										   EVFILT_READ, EV_DISABLE, 0, 0, buf );
				}
			} else if ( cur_event->filter == EVFILT_WRITE ) {
				// std::cout << "sending response" << std::endl;

				t_client_buf *buf = (t_client_buf *)cur_event->udata;

				if ( buf->flag_ & SOCK_WRITE ) {
					int n;
					n = write( cur_event->ident, buf->response_.c_str(),
							   buf->response_.size() );
					if ( n < 0 ) {
						// client error
					} else if ( n != buf->response_.size() ) {
						// partial write
					} else {
						if ( buf->response_queue_.front().body_flag_ ) {
							uintptr_t rd_fd = open(
								buf->response_queue_.front().file_path_.c_str(),
								O_RDONLY, 0644 );
							if ( rd_fd = -1 ) {
								std::cerr << "file open error" << std::endl;
							} else {
								add_event_change_list(
									changelist, rd_fd, EVFILT_READ,
									EV_ADD | EV_ENABLE, 0, 0, buf );
							}
							add_event_change_list( changelist, cur_event->ident,
												   EVFILT_WRITE, EV_DISABLE, 0,
												   0, buf );
						}
						buf->response_queue_.pop();
					}
				} else if ( buf->flag_ & RESPONSE_BODY ) {
				}
				if ( buf->flag_ & SOCK_WRITE == false ) {
					add_event_change_list( changelist, cur_event->ident,
										   EVFILT_WRITE, EV_DISABLE, 0, 0,
										   buf );
				}
				std::cout << "sending response end" << std::endl;
			}
		}
	}
	return 0;
}
