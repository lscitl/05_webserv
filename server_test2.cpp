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
#define EVENT_CHANGE_BUF 10
#define BUFFER_SIZE 8 * 1024
#define BUFFER_MAX 80 * 1024

typedef std::vector<char> t_buffer;

enum {
	HEAD_END_FLAG = 2,
	SOCK_WRITE = 4,
	RES_GET = 8,
	RES_HEAD = 16,
	RES_POST = 32,
	RES_PUT = 64,
	RES_DELETE = 128,
	READ_BODY = 256,
	REQ_LINE_PARSED = 512,
	RES_BODY = 1024,
	NONE = 0
};

enum { REQ_LINE_PARSING = 0, REQ_HEADER_PARSING = 1, REQ_BODY = 2 };

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

void add_event_change_list( std::vector<struct kevent> &change_list,
							uintptr_t ident, int64_t filter, uint16_t flags,
							uint32_t fflags, intptr_t data, void *udata ) {
	struct kevent tmp_event;
	EV_SET( &tmp_event, ident, filter, flags, fflags, data, udata );
	change_list.push_back( tmp_event );
}

void disconnect_client( int                                client_fd,
						std::map<uintptr_t, t_client_buf> &clients ) {
	std::cout << "client disconnected: " << client_fd << std::endl;
	close( client_fd );
	clients.erase( client_fd );
}

typedef struct ReqField {
	std::string                        req_target_;
	std::string                        http_ver_;
	std::map<std::string, std::string> field_;
	int                                req_type_;

	ReqField() : req_target_(), http_ver_(), field_(), req_type_( 0 ) {
	}
	~ReqField() {
	}
} t_req_field;

typedef struct ResField {
	std::string res_header_;
	std::string file_path_;
	t_buffer    body_buffer_;
	int         sent_pos_;
	int         body_flag_;
	int         transfer_encoding_;

	ResField()
		: res_header_(),
		  file_path_(),
		  body_buffer_(),
		  sent_pos_( 0 ),
		  body_flag_( 0 ),
		  transfer_encoding_( 0 ) {
	}
	~ResField() {
	}
} t_res_field;

typedef struct ClientBuffer {
	std::queue<std::pair<t_req_field, t_res_field> > req_res_queue_;
	t_buffer                                         rdsaved_;
	timespec                                         timeout_;
	uintptr_t                                        client_fd;
	char                                             rbuf_[BUFFER_SIZE];
	int                                              rdchecked_;
	int                                              flag_;
	int                                              state_;

	ClientBuffer()
		: req_res_queue_(),
		  rdsaved_(),
		  timeout_(),
		  client_fd(),
		  rbuf_(),
		  rdchecked_( 0 ),
		  flag_( 0 ),
		  state_( REQ_LINE_PARSING ) {
	}
	~ClientBuffer() {
	}
} t_client_buf;

bool request_line_check( std::string &req_line, t_client_buf &buf ) {
	return true;
}

int request_line_parser( uintptr_t fd, t_client_buf &buf ) {
	std::string              req_line;
	t_buffer::const_iterator crlf_pos;

	while ( true ) {
		// if ( buf.rdsaved_.size() ) {
		// crlf_pos = strnstr( buf.rdsaved_.data(), "\r\n", buf.rdsaved_.size()
		// );
		crlf_pos = std::find( buf.rdsaved_.begin(), buf.rdsaved_.end(), '\r' );
		if ( crlf_pos != buf.rdsaved_.end() ) {
			if ( *( crlf_pos + 1 ) != '\n' ) {
				// bad request
				return -1;
			}
			req_line.assign( buf.rdsaved_.data(), buf.rdchecked_,
							 crlf_pos - buf.rdsaved_.begin() - buf.rdchecked_ );
			buf.rdchecked_ = crlf_pos - buf.rdsaved_.begin() + 2;
			if ( req_line.size() == 0 ) {
				// request line is empty. get next crlf.
				continue;
			}
			buf.req_res_queue_.push( std::pair<t_req_field, t_res_field>() );
			if ( request_line_check( req_line, buf ) == false ) {
				// bad request
				return -1;
			}
			// buf.flag_ |= REQ_LINE_PARSED;
			break;
		}
		buf.rdsaved_.erase( buf.rdsaved_.begin(),
							buf.rdsaved_.begin() + buf.rdchecked_ );
		buf.rdchecked_ = 0;
		return false;
	}
	return true;
}

bool header_valid_check( std::string &key_val, size_t col_pos ) {
	// check logic
	return true;
}

bool header_field_parser( uintptr_t fd, t_client_buf &buf ) {
	std::string              header_field_line;
	t_buffer::const_iterator crlf_pos;
	int                      idx;

	while ( true ) {
		crlf_pos = std::find( buf.rdsaved_.begin(), buf.rdsaved_.end(), '\r' );
		if ( crlf_pos != buf.rdsaved_.end() ) {
			if ( *( crlf_pos + 1 ) != '\n' ) {
				// bad request
				return false;
			}
			header_field_line.assign(
				buf.rdsaved_.data(), buf.rdchecked_,
				crlf_pos - buf.rdsaved_.begin() - buf.rdchecked_ );
			buf.rdchecked_ = crlf_pos - buf.rdsaved_.begin() + 2;
			if ( header_field_line.size() == 0 ) {
				// request header parsed.
				if ( buf.req_res_queue_.back().first.req_type_ &
					 ( REQ_POST | REQ_PUT ) ) {
					buf.state_ = REQ_BODY;
				}
				break;
				// buf.flag_ |= HEAD_END_FLAG;
			}
			idx = header_field_line.find( ':' );
			if ( idx != std::string::npos ) {
				size_t tmp = idx + 1;
				// to do
				if ( header_valid_check( header_field_line, idx ) ) {
					while ( header_field_line[tmp] == ' ' ||
							header_field_line[tmp] == '\t' ) {
						++tmp;
					}
					buf.req_res_queue_.back()
						.first.field_[header_field_line.substr( 0, idx )] =
						header_field_line.substr(
							tmp, header_field_line.size() - tmp );
				} else {
					// bad request.
					return false;
				}
			}
			continue;
		}
		buf.rdsaved_.erase( buf.rdsaved_.begin(),
							buf.rdsaved_.begin() + buf.rdchecked_ );
		buf.rdchecked_ = 0;
		return false;
	}
	// buf.flag_ &= ~HEAD_END_FLAG;
	return true;
}

// time out case?
void disconnect_client( uintptr_t                   client_fd,
						std::vector<struct kevent> &change_list,
						t_client_buf               *buf ) {
	// client status, tmp file...? check.
	add_event_change_list( change_list, client_fd, EVFILT_READ, EV_DELETE, 0, 0,
						   NULL );
	add_event_change_list( change_list, client_fd, EVFILT_WRITE, EV_DELETE, 0,
						   0, NULL );
	close( client_fd );
	delete buf;
}

bool create_client_event( uintptr_t serv_sd, struct kevent *cur_event,
						  std::vector<struct kevent>        &change_list,
						  std::map<uintptr_t, t_client_buf> &clients ) {
	uintptr_t client_fd;
	if ( ( client_fd = accept( serv_sd, NULL, NULL ) ) == -1 ) {
		std::cerr << strerror( errno ) << std::endl;
		return false;
	} else {
		std::cout << "accept new client: " << client_fd << std::endl;
		fcntl( client_fd, F_SETFL, O_NONBLOCK );
		t_client_buf *new_buf = new t_client_buf();
		add_event_change_list( change_list, client_fd, EVFILT_READ,
							   EV_ADD | EV_ENABLE, 0, 0, new_buf );
		add_event_change_list( change_list, client_fd, EVFILT_WRITE,
							   EV_ADD | EV_DISABLE, 0, 0, new_buf );
		return true;
	}
}

int write_res_header( uintptr_t fd, t_res_field &res,
					  std::vector<struct kevent> &change_list,
					  t_client_buf               *buf ) {
	int n;

	n = write( fd, &res.res_header_.c_str()[res.sent_pos_],
			   res.res_header_.size() - res.sent_pos_ );
	if ( n < 0 ) {
		// client fd error. maybe disconnected.
		// error handle code
		return n;
	}
	if ( n != res.res_header_.size() - res.sent_pos_ ) {
		// partial write
		res.sent_pos_ += n;
	} else {
		// header sent
		if ( res.body_flag_ ) {
			int fd = open( res.file_path_.c_str(), O_RDONLY, 0644 );
			if ( fd < 0 ) {
				// file open error. incorrect direction ??
			}
			add_event_change_list( change_list, buf->client_fd, EVFILT_READ,
								   EV_DISABLE, 0, 0, buf );
			add_event_change_list( change_list, fd, EVFILT_READ,
								   EV_ADD | EV_ENABLE, 0, 0, buf );
			buf->flag_ |= RES_BODY;
			buf->flag_ &= ~SOCK_WRITE;
		} else {
			buf->req_res_queue_.pop();
		}
	}
	return n;
}

int write_res_body( uintptr_t fd, std::vector<struct kevent> &change_list,
					t_client_buf *buf ) {
	int n;
	if ( buf->req_res_queue_.front().second.transfer_encoding_ ) {
		// chunked code.
	} else {
		n = write( fd, buf->rbuf_, buf->)
	}
}

uintptr_t server_init() {
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
}

void ev_read( uintptr_t serv_sd, struct kevent *cur_event,
			  std::vector<struct kevent>        &change_list,
			  std::map<uintptr_t, t_client_buf> &clients ) {
	if ( cur_event->ident == serv_sd ) {
		if ( create_client_event( serv_sd, cur_event, change_list, clients ) ==
			 false ) {
			// error ???
		}
	} else {
		t_client_buf *buf = static_cast<t_client_buf *>( cur_event->udata );
		ssize_t       n_read;
		// std::cout << "recieved data from " << cur_event->ident
		// 		  << ":" << std::endl;

		if ( buf->state_ != REQ_BODY ||
			 buf->req_res_queue_.front().second.body_buffer_.size() <
				 BUFFER_MAX ) {
			n_read = read( cur_event->ident, buf->rbuf_, BUFFER_SIZE );
		}
		if ( n_read < 0 ) {
			return;
		}
		if ( buf->state_ != REQ_BODY ) {
			buf->rdsaved_.insert( buf->rdsaved_.end(), buf->rbuf_,
								  buf->rbuf_ + n_read );
		}
		switch ( buf->state_ ) {
			case REQ_LINE_PARSING:
				if ( request_line_parser( cur_event->ident, *buf ) == false ) {
					return;
				}
			case REQ_HEADER_PARSING:
				if ( header_field_parser( cur_event->ident, *buf ) == false ) {
					// need to read more from the client socket.
				}
			case REQ_BODY:
				t_buffer::const_iterator ite =
					buf->req_res_queue_.front().second.body_buffer_.end();
				buf->req_res_queue_.front().second.body_buffer_.insert(
					ite, buf->rbuf_, buf->rbuf_ + n_read );
		}
		switch ( buf->req_res_queue_.back().first.req_type_ ) {
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
				break;
		}
		add_event_change_list( change_list, cur_event->ident, EVFILT_WRITE,
							   EV_ENABLE, 0, 0, buf );
		// add_event_change_list( change_list, cur_event->ident,
		// 					   EVFILT_READ, EV_DISABLE, 0, 0, buf );
	}
}

int main( void ) {
	uintptr_t                         serv_sd = server_init();
	std::map<uintptr_t, t_client_buf> clients;
	std::vector<struct kevent>        change_list;
	struct kevent                     event_list[8];
	int                               kq;

	kq = kqueue();
	if ( kq == -1 ) {
		error_exit( "kqueue()", close, serv_sd );
	}

	add_event_change_list( change_list, serv_sd, EVFILT_READ,
						   EV_ADD | EV_ENABLE, 0, 0, NULL );

	int            event_len;
	struct kevent *cur_event;
	// int            l = 0;
	while ( true ) {
		event_len = kevent( kq, change_list.begin().base(), change_list.size(),
							event_list, 8, NULL );
		if ( event_len == -1 ) {
			error_exit( "kevent()", close, serv_sd );
		}
		change_list.clear();

		// std::cout << "current loop: " << l++ << std::endl;

		for ( int i = 0; i < event_len; ++i ) {
			cur_event = &event_list[i];

			if ( cur_event->flags & EV_ERROR ) {
				if ( cur_event->ident == serv_sd ) {
					error_exit( "server socket error", close, serv_sd );
				} else {
					std::cerr << "client socket error" << std::endl;
					disconnect_client( cur_event->ident, clients );
				}
			} else if ( cur_event->filter == EVFILT_READ ) {
				// if ( cur_event->ident == serv_sd ) {
				// 	if ( create_client_event( serv_sd, cur_event, change_list,
				// 							  clients ) == false ) {
				// 		// error ???
				// 	}
				// } else {
				// 	t_client_buf *buf =
				// 		static_cast<t_client_buf *>( cur_event->udata );
				// 	ssize_t n_read;
				// 	// std::cout << "recieved data from " << cur_event->ident
				// 	// 		  << ":" << std::endl;

				// 	if ( buf->state_ != REQ_BODY ||
				// 		 buf->req_res_queue_.front()
				// 				 .second.body_buffer_.size() < BUFFER_MAX ) {
				// 		n_read =
				// 			read( cur_event->ident, buf->rbuf_, BUFFER_SIZE );
				// 	}
				// 	if ( n_read < 0 ) {
				// 		continue;
				// 	}
				// 	if ( buf->state_ != REQ_BODY ) {
				// 		buf->rdsaved_.insert( buf->rdsaved_.end(), buf->rbuf_,
				// 							  buf->rbuf_ + n_read );
				// 	}
				// 	switch ( buf->state_ ) {
				// 		case REQ_LINE_PARSING:
				// 			if ( request_line_parser( cur_event->ident,
				// 									  *buf ) == false ) {
				// 				continue;
				// 			}
				// 		case REQ_HEADER_PARSING:
				// 			if ( header_field_parser( cur_event->ident,
				// 									  *buf ) == false ) {
				// 				// need to read more from the client socket.
				// 			}
				// 		case REQ_BODY:
				// 			t_buffer::const_iterator ite =
				// 				buf->req_res_queue_.front()
				// 					.second.body_buffer_.end();
				// 			buf->req_res_queue_.front()
				// 				.second.body_buffer_.insert(
				// 					ite, buf->rbuf_, buf->rbuf_ + n_read );
				// 	}
				// 	switch ( buf->req_res_queue_.back().first.req_type_ ) {
				// 		case REQ_GET:
				// 			break;
				// 		case REQ_HEAD:
				// 			break;
				// 		case REQ_POST:
				// 			break;
				// 		case REQ_PUT:
				// 			break;
				// 		case REQ_DELETE:
				// 			break;
				// 		case REQ_UNDEFINED:
				// 			break;
				// 		default:
				// 			break;
				// 	}
				// 	add_event_change_list( change_list, cur_event->ident,
				// 						   EVFILT_WRITE, EV_ENABLE, 0, 0, buf );
				// 	// add_event_change_list( change_list, cur_event->ident,
				// 	// 					   EVFILT_READ, EV_DISABLE, 0, 0, buf );
				// }
			} else if ( cur_event->filter == EVFILT_WRITE ) {
				// std::cout << "sending response" << std::endl;

				t_client_buf *buf = (t_client_buf *)cur_event->udata;

				if ( buf->flag_ & RES_BODY ) {
					write_res_body( cur_event->ident,
									buf->req_res_queue_.front().second,
									change_list, buf );
				} else {
					if ( write_res_header( cur_event->ident,
										   buf->req_res_queue_.front().second,
										   change_list, buf ) < 0 ) {
						// error
					}
					if ( buf->req_res_queue_.size() == 0 ) {
						add_event_change_list( change_list, cur_event->ident,
											   EVFILT_WRITE, EV_DISABLE, 0, 0,
											   buf );
					}
				}
				if ( buf->flag_ & SOCK_WRITE == false ) {
					add_event_change_list( change_list, cur_event->ident,
										   EVFILT_WRITE, EV_DISABLE, 0, 0,
										   buf );
				}
				std::cout << "sending response end" << std::endl;
			}
		}
	}
	return 0;
}
