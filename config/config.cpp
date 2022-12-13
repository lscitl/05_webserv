#include "config.hpp"
#include "port_info.hpp"
#include <cctype>
#include <cstring>
#include <string>

namespace {

	uint32_t digit_alpha_[] = {
		0x00000000, /*	0000 0000 0000 0000  0000 0000 0000 0000 */
		/*				?>=< ;:98 7654 3210  /.-, +*)( '&%$ #"!  */
		0x03ff0000, /*	0000 0011 1111 1111  0000 0000 0000 0000 */
		/*				_^]\ [ZYX WVUT SRQP  ONML KJIH GFED CBA@ */
		0x07fffffe, /*	0000 0111 1111 1111  1111 1111 1111 1110 */
		/*				 ~}| {zyx wvut srqp  onml kjih gfed cba` */
		0x07fffffe, /*	0000 0111 1111 1111  1111 1111 1111 1110 */
		0x00000000, /*	0000 0000 0000 0000  0000 0000 0000 0000 */
		0x00000000, /*	0000 0000 0000 0000  0000 0000 0000 0000 */
		0x00000000, /*	0000 0000 0000 0000  0000 0000 0000 0000 */
		0x00000000, /*	0000 0000 0000 0000  0000 0000 0000 0000 */
	};

	uint32_t alpha_upper_case_[] = {
		0x00000000, /*	0000 0000 0000 0000  0000 0000 0000 0000 */
		/*				?>=< ;:98 7654 3210  /.-, +*)( '&%$ #"!  */
		0x00000000, /*	0000 0000 0000 0000  0000 0000 0000 0000 */
		/*				_^]\ [ZYX WVUT SRQP  ONML KJIH GFED CBA@ */
		0x07fffffe, /*	0000 0111 0000 0000  0000 0000 0000 1110 */
		/*				 ~}| {zyx wvut srqp  onml kjih gfed cba` */
		0x00000000, /*	0000 0000 0000 0000  0000 0000 0000 0000 */
		0x00000000, /*	0000 0000 0000 0000  0000 0000 0000 0000 */
		0x00000000, /*	0000 0000 0000 0000  0000 0000 0000 0000 */
		0x00000000, /*	0000 0000 0000 0000  0000 0000 0000 0000 */
		0x00000000, /*	0000 0000 0000 0000  0000 0000 0000 0000 */
	};

	uint32_t digit_[] = {
		0x00000000, /*	0000 0000 0000 0000  0000 0000 0000 0000 */
		/*				?>=< ;:98 7654 3210  /.-, +*)( '&%$ #"!  */
		0x03ff0000, /*	0000 0011 1111 1111  0000 0000 0000 0000 */
		/*				_^]\ [ZYX WVUT SRQP  ONML KJIH GFED CBA@ */
		0x00000000, /*	0000 0000 0000 0000  0000 0000 0000 0000 */
		/*				 ~}| {zyx wvut srqp  onml kjih gfed cba` */
		0x00000000, /*	0000 0000 0000 0000  0000 0000 0000 0000 */
		0x00000000, /*	0000 0000 0000 0000  0000 0000 0000 0000 */
		0x00000000, /*	0000 0000 0000 0000  0000 0000 0000 0000 */
		0x00000000, /*	0000 0000 0000 0000  0000 0000 0000 0000 */
		0x00000000, /*	0000 0000 0000 0000  0000 0000 0000 0000 */
	};
	uint32_t name_token_[] = {
		0x00000000, /*	0000 0000 0000 0000  0000 0000 0000 0000 */
		/*				?>=< ;:98 7654 3210  /.-, +*)( '&%$ #"!  */
		0x03ff6cfa, /*	0000 0011 1111 1111  0110 1100 1111 1010 */
		/*				_^]\ [ZYX WVUT SRQP  ONML KJIH GFED CBA@ */
		0xc7fffffe, /*	1100 0111 1111 1111  1111 1111 1111 1110 */
		/*				 ~}| {zyx wvut srqp  onml kjih gfed cba` */
		0x57ffffff, /*	0101 0111 1111 1111  1111 1111 1111 1111 */
		0x00000000, /*	0000 0000 0000 0000  0000 0000 0000 0000 */
		0x00000000, /*	0000 0000 0000 0000  0000 0000 0000 0000 */
		0x00000000, /*	0000 0000 0000 0000  0000 0000 0000 0000 */
		0x00000000, /*	0000 0000 0000 0000  0000 0000 0000 0000 */
	};

	uint32_t vchar_[] = {
		0x00000200, /*	0000 0000 0000 0000  0000 0010 0000 0000 */
		/*				?>=< ;:98 7654 3210  /.-, +*)( '&%$ #"!  */
		0xfffffffe, /*	1111 1111 1111 1111  1111 1111 1111 1110 */
		/*				_^]\ [ZYX WVUT SRQP  ONML KJIH GFED CBA@ */
		0xffffffff, /*	1111 1111 1111 1111  1111 1111 1111 1111 */
		/*				 ~}| {zyx wvut srqp  onml kjih gfed cba` */
		0x7fffffff, /*	0111 1111 1111 1111  1111 1111 1111 1111 */
		0xffffffff, /*	1111 1111 1111 1111  1111 1111 1111 1111 */ /* obs-text */
		0xffffffff, /*	1111 1111 1111 1111  1111 1111 1111 1111 */
		0xffffffff, /*	1111 1111 1111 1111  1111 1111 1111 1111 */
		0xffffffff, /*	1111 1111 1111 1111  1111 1111 1111 1111 */
	};

	uint32_t listen_[] = {
		0x00000000, /*	0000 0000 0000 0000  0000 0000 0000 0000 */
		/*				?>=< ;:98 7654 3210  /.-, +*)( '&%$ #"!  */
		0x07ff4000, /*	0000 0111 1111 1111  0100 0000 0000 0000 */
		/*				_^]\ [ZYX WVUT SRQP  ONML KJIH GFED CBA@ */
		0x07fffffe, /*	0000 0111 1111 1111  1111 1111 1111 1110 */
		/*				 ~}| {zyx wvut srqp  onml kjih gfed cba` */
		0x07fffffe, /*	0000 0111 1111 1111  1111 1111 1111 1110 */
		0x00000000, /*	0000 0000 0000 0000  0000 0000 0000 0000 */
		0x00000000, /*	0000 0000 0000 0000  0000 0000 0000 0000 */
		0x00000000, /*	0000 0000 0000 0000  0000 0000 0000 0000 */
		0x00000000, /*	0000 0000 0000 0000  0000 0000 0000 0000 */
	};

	uint32_t config_elem_syntax_[] = {
		0x00000000, /*	0000 0000 0000 0000  0000 0000 0000 0000 */
		/*				?>=< ;:98 7654 3210  /.-, +*)( '&%$ #"!  */
		0x00000000, /*	0000 0000 0000 0000  0000 0000 0000 0000 */
		/*				_^]\ [ZYX WVUT SRQP  ONML KJIH GFED CBA@ */
		0x80000000, /*	1000 0000 0000 0000  0000 0000 0000 0000 */
		/*				 ~}| {zyx wvut srqp  onml kjih gfed cba` */
		0x077df3fe, /*	0000 0111 0111 1101  1111 0011 1111 1110 */
		0x00000000, /*	0000 0000 0000 0000  0000 0000 0000 0000 */
		0x00000000, /*	0000 0000 0000 0000  0000 0000 0000 0000 */
		0x00000000, /*	0000 0000 0000 0000  0000 0000 0000 0000 */
		0x00000000, /*	0000 0000 0000 0000  0000 0000 0000 0000 */
	};

	uint32_t isspace_[] = {
		0x00003e00, /*	0000 0000 0000 0000  0011 1110 0000 0000 */
		/*				?>=< ;:98 7654 3210  /.-, +*)( '&%$ #"!  */
		0x00000001, /*	0000 0000 0000 0000  0000 0000 0000 0001 */
		/*				_^]\ [ZYX WVUT SRQP  ONML KJIH GFED CBA@ */
		0x00000000, /*	0000 0000 0000 0000  0000 0000 0000 0000 */
		/*				 ~}| {zyx wvut srqp  onml kjih gfed cba` */
		0x00000000, /*	0000 0000 0000 0000  0000 0000 0000 0000 */
		0x00000000, /*	0000 0000 0000 0000  0000 0000 0000 0000 */
		0x00000000, /*	0000 0000 0000 0000  0000 0000 0000 0000 */
		0x00000000, /*	0000 0000 0000 0000  0000 0000 0000 0000 */
		0x00000000, /*	0000 0000 0000 0000  0000 0000 0000 0000 */
	};

	inline status
	error_(const char* msg, const char* msg2) {
// throw(msg);
#ifdef DEBUG
		std::cout << msg << " : " << msg2 << std::endl;
#endif
		return spx_error;
	}

	inline uint8_t
	syntax_(const uint32_t table[8], uint8_t c) {
		return (table[(c >> 5)] & (1U << (c & 0x1f)));
	}

	template <typename T>
	inline void
	memset_(T& target) {
		memset(&target, 0, sizeof(T));
	}
}

status
spx_config_syntax_checker(std::string const&	   buf,
						  total_port_server_map_p& config_map) { // TODO : [add function] check default_server is exist and properly handle

	std::string::const_iterator		  it			= buf.begin();
	total_port_server_map_p::iterator total_conf_it = config_map.begin();
	server_map_p::iterator			  per_port_server_it;
	uri_location_map_p::iterator	  per_uri_location_it;
	std::string						  temp_string;
	uint8_t							  flag_default_part;
	uint8_t							  flag_location_part;
	uint8_t							  size_count;
	server_info_for_copy_stage_t	  temp_basic_server_info;
	uri_location_for_copy_stage_t	  temp_uri_location_info;

	enum {
		conf_zero,
		conf_start, // skip isspace
		conf_endline,

		conf_waiting_default_value,

		conf_server,
		conf_server_CB_open,
		conf_server_CB_close,
		conf_listen,
		conf_listen_default,
		conf_server_name,
		conf_error_page,
		conf_client_max_body_size,

		conf_waiting_location_value,

		conf_location_zero,
		conf_location_uri, // NOTE:: check is_set default value.
		conf_location_CB_open,
		conf_location_CB_close,
		conf_accepted_methods,
		conf_root,
		conf_index,
		conf_autoindex,
		conf_redirect,
		conf_saved_path,
		conf_cgi_pass,
		conf_cgi_path_info,

		conf_almost_done,
		conf_done
	} state,
		prev_state,
		next_state;

	state = conf_zero;

	while (state != conf_done) {
		switch (state) {
		case conf_zero: {
			memset_(flag_default_part);
			memset_(flag_location_part);
			memset_(size_count);
			memset_(temp_basic_server_info);
			memset_(temp_uri_location_info);
			prev_state = state;
			state	   = conf_start;
			next_state = conf_server;
			break;
		}

		case conf_start: {
			if (syntax_(isspace_, static_cast<uint8_t>(*it))) {
				++it;
				break;
			}
			if (*it == ';') {
				prev_state = next_state;
				state	   = conf_endline;
			} else {
				state = next_state;
			}
			break;
		}

		case conf_endline: {
			if (*it == ';') {
				++it;
				if (prev_state == conf_waiting_default_value) {
					next_state = conf_waiting_default_value;
				} else {
					next_state = conf_waiting_location_value;
				}
				prev_state = state;
				state	   = conf_start;
				break;
			}
		}

		case conf_waiting_default_value: {
			if (syntax_(config_elem_syntax_, static_cast<uint8_t>(*it))) {
				temp_string.push_back(*it);
				++size_count;
				++it;
				prev_state = state;
				break;
			}
			if (*it == ';') {
				if (prev_state == conf_start) {
					prev_state = state;
					state	   = conf_endline;
					next_state = conf_start;
					break;
				}
				return error_("conf_waiting_default_value", "syntax error");
			}
			if (syntax_(isspace_, static_cast<uint8_t>(*it))) {
				prev_state = state;
				state	   = conf_start;
				switch (size_count) {
				case 6: {
					if (temp_string.compare("listen") == KSame) {
						if (flag_default_part & flag_listen) {
							return error_("conf_waiting_value", "listen is already set");
						}
						next_state = conf_listen;
						break;
					}
					return error_("conf_waiting_value", "syntax error");
				}
				case 8: {
					if (temp_string.compare("location") == KSame) {
						if (!(flag_default_part & flag_listen)
							|| !(flag_default_part & flag_server_name)
							|| !(flag_default_part & flag_error_page)
							|| !(flag_default_part & flag_client_max_body_size)) {
							return error_("conf_waiting_value",
										  "need sed default value before location - syntax error");
						}
						next_state = conf_location_uri;
						break;
					}
					return error_("conf_waiting_value", "syntax error");
				}
				case 10: {
					if (temp_string.compare("error_page") == KSame) {
						if (flag_default_part & flag_error_page) {
							return error_("conf_waiting_value", "error_page is already set");
						}
						next_state = conf_error_page;
						break;
					}
					return error_("conf_waiting_value", "syntax error");
				}
				case 11: {
					if (temp_string.compare("server_name") == KSame) {
						if (flag_default_part & flag_server_name) {
							return error_("conf_waiting_value", "server_name is already set");
						}
						next_state = conf_server_name;
						break;
					}
					return error_("conf_waiting_value", "syntax error");
				}
				case 20: {
					if (temp_string.compare("client_max_body_size") == KSame) {
						if (flag_default_part & flag_client_max_body_size) {
							return error_("conf_waiting_value", "client_max_body_size is already set");
						}
						next_state = conf_client_max_body_size;
						break;
					}
					return error_("conf_waiting_value", "syntax error");
				}
				default:
					return error_("conf_waiting_value", "syntax error");
				}
				temp_string.clear();
				size_count = 0;
				break;
			}
			return error_("conf_waiting_value", "syntax error");
		}

		case conf_server: {
			if (buf.compare(it - buf.begin(), 6, "server") == KSame) {
				prev_state = state;
				state	   = conf_start;
				next_state = conf_server_CB_open;
				break;
			}
			return error_("conf_server", "syntax error");
		}

		case conf_server_CB_open: {
			if (*it == '{') {
				++it;
				prev_state = state;
				state	   = conf_start;
				next_state = conf_waiting_default_value;
				break;
			}
			return error_("conf_server_CB_open", "syntax error");
		}

		case conf_server_CB_close: {

			break;
		}

		case conf_listen: {
			if (syntax_(listen_, static_cast<uint8_t>(*it))) {
				temp_string.push_back(*it);
				++size_count;
				++it;
				break;
			}
			if (syntax_(isspace_, static_cast<uint8_t>(*it)) || *it == ';') {
				std::string::const_iterator temp_it	   = temp_string.begin();
				uint8_t						temp_cycle = size_count;

				if (size_count && size_count <= 5) { // min 0, max 65535 // only port case
					while (temp_cycle--) {
						if (std::isdigit(*temp_it)) {
							temp_it++;
						} else {
							return error_("conf_listen", "syntax error");
						}
					}
					temp_basic_server_info.port = std::atoi(temp_string.c_str());
				} else if (10 < size_count && size_count <= 15) { // IP:PORT case / 127.0.0.1:80808 - 15 / localhost:80808 - 15
					if (temp_string.compare("localhost:") == KSame
						|| temp_string.compare("127.0.0.1:") == KSame) {
						temp_it += 10;
						temp_cycle -= 10;
						while (temp_cycle) {
							if (std::isdigit(*(temp_it + temp_cycle))) {
								--temp_cycle;
							} else {
								return error_("conf_listen invalid PORT", "syntax error");
							}
						}
						temp_string.erase(0, 10);
						temp_basic_server_info.port = std::atoi(temp_string.c_str());
					} else {
						return error_("conf_listen invalid IP:", "syntax error");
					}
				} else {
					return error_("conf_listen invalid IP:PORT", "syntax error");
				}
			} else {
				return error_("conf_listen invalid IP:PORT", "syntax error");
			}
			prev_state = state;
			state	   = conf_start;
			if (*it == ';') {
				++it;
				next_state = conf_waiting_default_value;
			} else {
				next_state = conf_listen_default;
			}
			temp_basic_server_info.ip = "127.0.0.1";
			flag_default_part |= flag_listen;
			temp_string.clear();
			size_count = 0;
			break;
		}

		case conf_listen_default: { // NOTE: check other default server
			if (buf.compare(it - buf.begin(), 14, "default_server") == KSame) {
				temp_basic_server_info.default_server_flag = default_server;
				it += 14;
			}
			if (syntax_(isspace_, static_cast<uint8_t>(*it)) || *it == ';') {
				prev_state = state;
				state	   = conf_start;
				next_state = conf_waiting_default_value;
				break;
			}
			return error_("conf_listen_default", "syntax error");
		}

		case conf_server_name: {
			if (syntax_(digit_alpha_, static_cast<uint8_t>(*it))) {
				temp_string.push_back(*it);
				++it;
				break;
			}
			if (syntax_(isspace_, static_cast<uint8_t>(*it)) || *it == ';') {
				temp_basic_server_info.server_name = temp_string;
				prev_state						   = state;
				state							   = conf_start;
				next_state						   = conf_waiting_default_value;
				flag_default_part |= flag_server_name;
				temp_string.clear();
				break;
			}
			return error_("conf_server_name", "syntax error");
		}

		case conf_error_page: { // NOTE : is it need to check valid path?
			if (syntax_(vchar_, static_cast<uint8_t>(*it))) {
				temp_string.push_back(*it);
				++it;
				break;
			}
			if (syntax_(isspace_, static_cast<uint8_t>(*it)) || *it == ';') {
				temp_basic_server_info.error_page = temp_string;
				prev_state						  = state;
				state							  = conf_start;
				next_state						  = conf_waiting_default_value;
				flag_default_part |= flag_error_page;
				temp_string.clear();
				break;
			}
			return error_("conf_error_page", "syntax error");
		}

		case conf_client_max_body_size: {
			if (syntax_(digit_, static_cast<uint8_t>(*it))) {
				temp_string.push_back(*it);
				++size_count;
				++it;
				break;
			}
			if ((*it == 'M' || *it == 'K') && (size_count && size_count <= 4)) {
				uint64_t check_body_size = std::atoi(temp_string.c_str());
				if (check_body_size > 0) {
					if (*it == 'M') {
						check_body_size *= (1024 * 1024);
					} else if (*it == 'K') {
						check_body_size *= 1024;
					}
					temp_basic_server_info.client_max_body_size = check_body_size;
					flag_default_part |= flag_client_max_body_size;
					prev_state = state;
					state	   = conf_start;
					next_state = conf_waiting_default_value;
				} else {
					return error_("conf_client_max_body_size - minus size", "syntax error");
				}
			} else {
				return error_("conf_client_max_body_size", "syntax error");
			}
			break;
		}

		case conf_location_zero: {
			memset_(temp_uri_location_info);
			memset_(flag_location_part);
			prev_state = state;
			state	   = conf_start;
			next_state = conf_waiting_default_value;
			break;
		}

		case conf_waiting_location_value: {
			if (syntax_(config_elem_syntax_, static_cast<uint8_t>(*it))) {
				temp_string.push_back(*it);
				++size_count;
				++it;
				prev_state = state;
				break;
			}
			if (*it == ';') {
				if (prev_state == conf_start) {
					prev_state = state;
					state	   = conf_endline;
					next_state = conf_start;
					break;
				}
				return error_("conf_waiting_default_value", "syntax error");
			}
			if (size_count == 0 && *it == '}' && flag_location_part != 0) {
				prev_state = state;
				state	   = conf_location_CB_close;
				next_state = conf_start;
				break;
			} else if (size_count == 0 && *it == '}' && flag_location_part == 0) {
				return error_("conf_waiting_default_value", "empty location - syntax error");
			}
			if (syntax_(isspace_, static_cast<uint8_t>(*it)) || *it == ';') {
				prev_state = state;
				state	   = conf_start;
				switch (size_count) {
				case 4: {
					if (temp_string.compare("root") == KSame) {
						if (flag_location_part & flag_root) {
							return error_("conf_waiting_locaiton_value", "root is already set");
						}
						next_state = conf_root;
						break;
					}
					return error_("conf_waiting_default_value", "syntax error");
				}
				case 5: {
					if (temp_string.compare("index") == KSame) {
						if (flag_location_part & flag_index) {
							return error_("conf_waiting_locaiton_value", "index is already set");
						}
						next_state = conf_index;
						break;
					}
					return error_("conf_waiting_default_value", "syntax error");
				}
				case 8: {
					if (temp_string.compare("cgi_pass") == KSame) {
						if (flag_location_part & flag_cgi_pass) {
							return error_("conf_waiting_locaiton_value", "cgi_pass is already set");
						}
						next_state = conf_cgi_pass;
						break;
					}
					if (temp_string.compare("redirect") == KSame) {
						if (flag_location_part & flag_redirect) {
							return error_("conf_waiting_locaiton_value", "redirect is already set");
						}
						next_state = conf_redirect;
						break;
					}
					return error_("conf_waiting_default_value", "syntax error");
				}
				case 9: {
					if (temp_string.compare("autoindex") == KSame) {
						if (flag_location_part & flag_autoindex) {
							return error_("conf_waiting_locaiton_value", "autoindex is already set");
						}
						next_state = conf_autoindex;
						break;
					}
					return error_("conf_waiting_default_value", "syntax error");
				}
				case 10: {
					if (temp_string.compare("saved_path") == KSame) {
						if (flag_location_part & flag_saved_path) {
							return error_("conf_waiting_locaiton_value", "saved_path is already set");
						}
						next_state = conf_saved_path;
						break;
					}
					return error_("conf_waiting_default_value", "syntax error");
				}
				case 13: {
					if (temp_string.compare("cgi_path_info") == KSame) {
						if (flag_location_part & flag_cgi_path_info) {
							return error_("conf_waiting_locaiton_value", "cgi_path_info is already set");
						}
						next_state = conf_cgi_path_info;
						break;
					}
					return error_("conf_waiting_default_value", "syntax error");
				}
				case 16: {
					if (temp_string.compare("accepted_methods") == KSame) {
						if (flag_location_part & flag_accepted_methods) {
							return error_("conf_waiting_locaiton_value", "accepted_methods is already set");
						}
						next_state = conf_accepted_methods;
						break;
					}
					return error_("conf_waiting_default_value", "syntax error");
				}
				default: {
					return error_("conf_waiting_default_value", "syntax error");
				}
				}
				temp_string.clear();
				size_count = 0;
				break;
			}
			return error_("conf_waiting_default_value", "syntax error");
		}

		case conf_location_uri: {
			if (syntax_(vchar_, static_cast<uint8_t>(*it))) {
				temp_string.push_back(*it);
				++it;
				break;
			}
			if (syntax_(isspace_, static_cast<uint8_t>(*it)) || *it == '{') {
				temp_uri_location_info.uri = temp_string;
				prev_state				   = state;
				state					   = conf_start;
				next_state				   = conf_location_CB_open;
				temp_string.clear();
				break;
			}
		}

		case conf_location_CB_open: {
			if (*it == '{') {
				++it;
				prev_state = state;
				state	   = conf_start;
				next_state = conf_waiting_location_value;
				break;
			}
			return error_("conf_location_CB_open", "syntax error");
		}

		case conf_location_CB_close: {

			break;
		}

		case conf_accepted_methods: {
			if (syntax_(alpha_upper_case_, static_cast<uint8_t>(*it))) {
				temp_string.push_back(*it);
				++it;
				++size_count;
				break;
			}
			if (syntax_(isspace_, static_cast<uint8_t>(*it)) || *it == ';') {
				switch (size_count) {
				case 3: {
					if (temp_string.compare("GET") == KSame) {
						if (temp_uri_location_info.accepted_methods_flag & KGet) {
							return error_("conf_accepted_methods", "GET is already set");
						}
						temp_uri_location_info.accepted_methods_flag |= KGet;
						break;
					}
					if (temp_string.compare("PUT") == KSame) {
						if (temp_uri_location_info.accepted_methods_flag & KPut) {
							return error_("conf_accepted_methods", "PUT is already set");
						}
						temp_uri_location_info.accepted_methods_flag |= KPut;
						break;
					}
					return error_("conf_accepted_methods", "syntax error");
				}
				case 4: {
					if (temp_string.compare("POST") == KSame) {
						if (temp_uri_location_info.accepted_methods_flag & KPost) {
							return error_("conf_accepted_methods", "POST is already set");
						}
						temp_uri_location_info.accepted_methods_flag |= KPost;
						break;
					}
					if (temp_string.compare("HEAD") == KSame) {
						if (temp_uri_location_info.accepted_methods_flag & KHead) {
							return error_("conf_accepted_methods", "HEAD is already set");
						}
						temp_uri_location_info.accepted_methods_flag |= KHead;
						break;
					}
					return error_("conf_accepted_methods", "syntax error");
				}
				case 6: {
					if (temp_string.compare("DELETE") == KSame) {
						if (temp_uri_location_info.accepted_methods_flag & KDelete) {
							return error_("conf_accepted_methods", "DELETE is already set");
						}
						temp_uri_location_info.accepted_methods_flag |= KDelete;
						break;
					}
					return error_("conf_accepted_methods", "syntax error");
				}
				default:
					return error_("conf_accepted_methods", "syntax error");
				}
				prev_state = state;
				state	   = conf_start;
				next_state = conf_accepted_methods;
				temp_string.clear();
				size_count = 0;
				break;
			}
			return error_("conf_accepted_methods", "syntax error");
		}

		case conf_root: {
			if (syntax_(vchar_, static_cast<uint8_t>(*it))) {
				temp_string.push_back(*it);
				++it;
				break;
			}
			if (syntax_(isspace_, static_cast<uint8_t>(*it)) || *it == ';') {
				temp_uri_location_info.root = temp_string;
				prev_state					= state;
				state						= conf_start;
				next_state					= conf_waiting_location_value;
				flag_location_part |= flag_root;
				temp_string.clear();
				break;
			}
			return error_("conf_root", "syntax error");
		}

		case conf_index: {
			if (syntax_(vchar_, static_cast<uint8_t>(*it))) {
				temp_string.push_back(*it);
				++it;
				break;
			}
			if (syntax_(isspace_, static_cast<uint8_t>(*it)) || *it == ';') {
				temp_uri_location_info.index = temp_string;
				prev_state					 = state;
				state						 = conf_start;
				next_state					 = conf_waiting_location_value;
				flag_location_part |= flag_index;
				temp_string.clear();
				break;
			}
			return error_("conf_index", "syntax error");
		}

		case conf_autoindex: {
			if (buf.compare(it - buf.begin(), 2, "on") == KSame) {
				temp_uri_location_info.autoindex_flag = Kautoindex_on;
				it += 2;
			} else if (buf.compare(it - buf.begin(), 3, "off") == KSame) {
				temp_uri_location_info.autoindex_flag = Kautoindex_off;
				it += 3;
			} else {
				return error_("conf_autoindex", "syntax error");
			}
			if (syntax_(isspace_, static_cast<uint8_t>(*it)) || *it == ';') {
				prev_state = state;
				state	   = conf_start;
				next_state = conf_waiting_location_value;
				flag_location_part |= flag_autoindex;
				break;
			}
			return error_("conf_autoindex", "syntax error");
		}

		case conf_redirect: {
			if (syntax_(vchar_, static_cast<uint8_t>(*it))) {
				temp_string.push_back(*it);
				++it;
				break;
			}
			if (syntax_(isspace_, static_cast<uint8_t>(*it)) || *it == ';') {
				temp_uri_location_info.redirect = temp_string;
				prev_state						= state;
				state							= conf_start;
				next_state						= conf_waiting_location_value;
				flag_location_part |= flag_redirect;
				temp_string.clear();
				break;
			}
			return error_("conf_redirect", "syntax error");
		}

		case conf_saved_path: {
			if (syntax_(vchar_, static_cast<uint8_t>(*it))) {
				temp_string.push_back(*it);
				++it;
				break;
			}
			if (syntax_(isspace_, static_cast<uint8_t>(*it)) || *it == ';') {
				temp_uri_location_info.saved_path = temp_string;
				prev_state						  = state;
				state							  = conf_start;
				next_state						  = conf_waiting_location_value;
				flag_location_part |= flag_saved_path;
				temp_string.clear();
				break;
			}
			return error_("conf_saved_path", "syntax error");
		}

		case conf_cgi_pass: {
			if (syntax_(vchar_, static_cast<uint8_t>(*it))) {
				temp_string.push_back(*it);
				++it;
				break;
			}
			if (syntax_(isspace_, static_cast<uint8_t>(*it)) || *it == ';') {
				temp_uri_location_info.cgi_pass = temp_string;
				prev_state						= state;
				state							= conf_start;
				next_state						= conf_waiting_location_value;
				flag_location_part |= flag_cgi_pass;
				temp_string.clear();
				break;
			}
			return error_("conf_cgi_pass", "syntax error");
		}

		case conf_cgi_path_info: {
			if (syntax_(vchar_, static_cast<uint8_t>(*it))) {
				temp_string.push_back(*it);
				++it;
				break;
			}
			if (syntax_(isspace_, static_cast<uint8_t>(*it)) || *it == ';') {
				temp_uri_location_info.cgi_path_info = temp_string;
				prev_state							 = state;
				state								 = conf_start;
				next_state							 = conf_waiting_location_value;
				flag_location_part |= flag_cgi_path_info;
				temp_string.clear();
				break;
			}
			return error_("conf_cgi_path_info", "syntax error");
		}

		case conf_almost_done: {

			break;
		}

		default:
			return error_("syntax error", "config file");
		}
	}

	return spx_ok;
}
