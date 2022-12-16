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
#define WRITE_BUFFER_MAX 40 * 1024
// #define BUFFER_MAX 80 * 1024

typedef std::vector<char> t_buffer;

enum e_client_buffer_flag {
	REQ_GET = 1,
	REQ_HEAD = 2,
	REQ_POST = 4,
	REQ_PUT = 8,
	REQ_DELETE = 16,
	REQ_UNDEFINED = 32,
	SOCK_WRITE = 128,
	READ_BODY = 256,
	RES_BODY = 1024,
	READ_READY = 1 << 30,
	E_BAD_REQ = 1 << 31
};

enum e_read_status {
	REQ_LINE_PARSING = 0,
	REQ_HEADER_PARSING,
	REQ_BODY,
	RES_BODY,
};

// gzip & deflate are not implemented.
enum { TE_CHUNKED = 0, TE_GZIP, TE_DEFLATE };

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

typedef struct ReqField {
	std::map<std::string, std::string> field_;
	t_buffer                           req_body_;
	std::string                        req_target_;
	std::string                        http_ver_;
	std::string                        file_path_;
	size_t                             body_recieved_;
	size_t                             body_limit_;
	size_t                             content_length_;
	int                                body_flag_;
	int                                req_type_;

	ReqField()
		: field_(),
		  req_body_(),
		  req_target_(),
		  http_ver_(),
		  file_path_(),
		  body_recieved_( 0 ),
		  body_limit_( -1 ),
		  content_length_( 0 ),
		  body_flag_( 0 ),
		  req_type_( 0 ) {
	}
	~ReqField() {
	}
} t_req_field;

typedef struct ResField {
	std::string res_header_;
	std::string file_path_;
	t_buffer    body_buffer_;
	size_t      content_length_;
	int         sent_pos_;
	int         body_flag_;
	int         transfer_encoding_;

	ResField()
		: res_header_(),
		  file_path_(),
		  body_buffer_(),
		  content_length_( 0 ),
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
	char                                             rdbuf_[BUFFER_SIZE];
	int                                              rdchecked_;
	int                                              flag_;
	int                                              state_;

	ClientBuffer()
		: req_res_queue_(),
		  rdsaved_(),
		  timeout_(),
		  client_fd(),
		  rdbuf_(),
		  rdchecked_( 0 ),
		  flag_( 0 ),
		  state_( REQ_LINE_PARSING ) {
	}
	~ClientBuffer() {
	}

	bool request_line_check( std::string &req_line ) {
		return true;
	}

	bool request_line_parser() {
		std::string        req_line;
		t_buffer::iterator crlf_pos = this->rdsaved_.begin();

		while ( true ) {
			crlf_pos = std::find( crlf_pos, this->rdsaved_.end(), '\n' );
			if ( crlf_pos != this->rdsaved_.end() ) {
				if ( *( --crlf_pos ) != '\r' ) {
					this->flag_ |= E_BAD_REQ;
					return false;
				}
				req_line.assign( this->rdsaved_.begin() + this->rdchecked_,
								 crlf_pos );
				this->rdchecked_ = crlf_pos - this->rdsaved_.begin() + 2;
				if ( req_line.size() == 0 ) {
					crlf_pos += 2;
					// request line is empty. get next crlf.
					continue;
				}
				this->req_res_queue_.push(
					std::pair<t_req_field, t_res_field>() );
				if ( this->request_line_check( req_line ) == false ) {
					this->flag_ |= E_BAD_REQ;
					return false;
				}
				break;
			}
			this->rdsaved_.erase( this->rdsaved_.begin(),
								  this->rdsaved_.begin() + this->rdchecked_ );
			this->rdchecked_ = 0;
			return false;
		}
		return true;
	}

	bool header_valid_check( std::string &key_val, size_t col_pos ) {
		// check logic
		return true;
	}

	bool header_field_parser() {
		std::string        header_field_line;
		t_buffer::iterator crlf_pos = this->rdsaved_.begin() + this->rdchecked_;
		int                idx;

		while ( true ) {
			crlf_pos = std::find( crlf_pos, this->rdsaved_.end(), '\n' );
			if ( crlf_pos != this->rdsaved_.end() ) {
				if ( *( --crlf_pos ) != '\r' ) {
					this->flag_ |= E_BAD_REQ;
					return false;
				}
				header_field_line.assign(
					this->rdsaved_.begin() + this->rdchecked_, crlf_pos );
				this->rdchecked_ = crlf_pos - this->rdsaved_.begin() + 2;
				if ( header_field_line.size() == 0 ) {
					// request header parsed.
					break;
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
						this->req_res_queue_.back()
							.first.field_[header_field_line.substr( 0, idx )] =
							header_field_line.substr(
								tmp, header_field_line.size() - tmp );
					} else {
						this->flag_ |= E_BAD_REQ;
						return false;
					}
				}
				continue;
			}
			this->rdsaved_.erase( this->rdsaved_.begin(),
								  this->rdsaved_.begin() + this->rdchecked_ );
			this->rdchecked_ = 0;
			return false;
		}
		const std::map<std::string, std::string> *field =
			&this->req_res_queue_.back().first.field_;
		std::map<std::string, std::string>::const_iterator it;
		it = field->find( "content-length" );
		if ( it != field->end() ) {
			this->req_res_queue_.back().first.content_length_ =
				strtoul( ( it->second ).c_str(), NULL, 10 );
			this->state_ = REQ_BODY;
		}
		it = field->find( "transfer-encoding" );
		if ( it != field->end() &&
			 it->second.find( "chunked" ) != std::string::npos ) {
			this->req_res_queue_.back().first.body_flag_ |= TE_CHUNKED;
			this->state_ = REQ_BODY;
		}
		return true;
	}

} t_client_buf;

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
						  std::vector<struct kevent> &change_list ) {
	uintptr_t client_fd;
	if ( ( client_fd = accept( serv_sd, NULL, NULL ) ) == -1 ) {
		std::cerr << strerror( errno ) << std::endl;
		return false;
	} else {
		std::cout << "accept new client: " << client_fd << std::endl;
		fcntl( client_fd, F_SETFL, O_NONBLOCK );
		t_client_buf *new_buf = new t_client_buf();
		new_buf->client_fd = client_fd;
		// port info will be added.
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
	if ( buf->req_res_queue_.front().second.transfer_encoding_ == TE_CHUNKED ) {
		// chunked code.
	} else {
		// n = write( fd, buf->rdbuf_, buf->)
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

void read_event_handler( uintptr_t serv_sd, struct kevent *cur_event,
						 std::vector<struct kevent> &change_list ) {
	// server port will be updated.
	if ( cur_event->ident == serv_sd ) {
		if ( create_client_event( serv_sd, cur_event, change_list ) == false ) {
			// error ???
		}
	} else {
		t_client_buf *buf = static_cast<t_client_buf *>( cur_event->udata );
		ssize_t       n_read;

		// REQ_BODY, RES_BODY check...?
		// if ( buf->state_ < REQ_BODY ||
		// 	 ( buf->state_ == RES_BODY &&
		// 	   buf->req_res_queue_.front().second.body_buffer_.size() <
		// 		   BUFFER_MAX ) ) {
		// }
		n_read = read( cur_event->ident, buf->rdbuf_, BUFFER_SIZE );
		if ( n_read < 0 ) {
			return;
		}
		buf->flag_ &= ~READ_READY;
		if ( buf->state_ != RES_BODY ) {
			buf->rdsaved_.insert( buf->rdsaved_.end(), buf->rdbuf_,
								  buf->rdbuf_ + n_read );
		}
		while ( buf->flag_ & READ_READY == false ) {
			switch ( buf->state_ ) {
				case REQ_LINE_PARSING:
					if ( buf->request_line_parser() == false ) {
						buf->flag_ &= READ_READY;
						break;
					}
				case REQ_HEADER_PARSING:
					if ( buf->header_field_parser() == false ) {
						// need to read more from the client socket.
						buf->flag_ &= READ_READY;
						break;
					}
					switch ( buf->req_res_queue_.back().first.req_type_ ) {
						case REQ_GET:
							break;
						case REQ_HEAD:
							break;
						case REQ_POST:
							if ( buf->req_res_queue_.back().first.body_flag_ ) {
								uintptr_t fd = open(
									buf->req_res_queue_.back()
										.first.file_path_.c_str(),
									O_RDONLY | O_CREAT | O_NONBLOCK | O_APPEND,
									0644 );
								if ( fd < 0 ) {
									// open error
								}
								add_event_change_list(
									change_list, cur_event->ident, EVFILT_READ,
									EV_DISABLE, 0, 0, buf );
								add_event_change_list(
									change_list, fd, EVFILT_READ,
									EV_ADD | EV_ENABLE, 0, 0, buf );
							} else {
								// maybe error?
							}
							break;
						case REQ_PUT:
							if ( buf->req_res_queue_.back().first.body_flag_ ) {
								uintptr_t fd = open(
									buf->req_res_queue_.back()
										.first.file_path_.c_str(),
									O_RDONLY | O_CREAT | O_NONBLOCK | O_TRUNC,
									0644 );
								if ( fd < 0 ) {
									// open error
								}
								add_event_change_list(
									change_list, cur_event->ident, EVFILT_READ,
									EV_DISABLE, 0, 0, buf );
								add_event_change_list(
									change_list, fd, EVFILT_READ,
									EV_ADD | EV_ENABLE, 0, 0, buf );
							} else {
								// maybe error?
							}
							break;
						case REQ_DELETE:
							break;
						case REQ_UNDEFINED:
							break;
						default:
							break;
					}
				case REQ_BODY:
					// client socket

					break;
				case RES_BODY:
					// server file descriptor.
					t_buffer::const_iterator ite =
						buf->req_res_queue_.front().second.body_buffer_.end();
					buf->req_res_queue_.front().second.body_buffer_.insert(
						ite, buf->rdbuf_, buf->rdbuf_ + n_read );

					// file end check.
					if ( lseek( cur_event->ident, 0, SEEK_CUR ) ==
						 buf->req_res_queue_.front().second.content_length_ -
							 1 ) {
						add_event_change_list( change_list, buf->client_fd,
											   EVFILT_READ, EV_ENABLE, 0, 0,
											   buf );
						close( cur_event->ident );
						add_event_change_list( change_list, cur_event->ident,
											   EVFILT_READ, EV_DELETE, 0, 0,
											   buf );
						// need to check _rdsaved buffer before read.
					}
					break;
			}
		}
		if ( buf->req_res_queue_.size() > 0 ) {
			add_event_change_list( change_list, cur_event->ident, EVFILT_WRITE,
								   EV_ENABLE, 0, 0, buf );
		}
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
					t_client_buf *buf =
						static_cast<t_client_buf *>( cur_event->udata );
					std::cerr << "client socket error" << std::endl;
					disconnect_client( cur_event->ident, change_list, buf );
				}
			} else if ( cur_event->filter == EVFILT_READ ) {
				read_event_handler( serv_sd, cur_event, change_list );
			} else if ( cur_event->filter == EVFILT_WRITE ) {
				// std::cout << "sending response" << std::endl;

				t_client_buf *buf = (t_client_buf *)cur_event->udata;

				if ( buf->flag_ & RES_BODY ) {
					write_res_body( cur_event->ident, change_list, buf );
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
