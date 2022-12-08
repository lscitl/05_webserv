#include "syntax.hpp"

namespace {

    uint32_t  usual[] = {
        0x00000000, /* 0000 0000 0000 0000  0000 0000 0000 0000 */

                    /* ?>=< ;:98 7654 3210  /.-, +*)( '&%$ #"!  */
        0x7fffb7d6, /* 0111 1111 1111 1111  1011 0111 1101 0110 */

                    /* _^]\ [ZYX WVUT SRQP  ONML KJIH GFED CBA@ */
        0xffffffff, /* 1111 1111 1111 1111  1111 1111 1111 1111 */

                    /*  ~}| {zyx wvut srqp  onml kjih gfed cba` */
        0x7fffffff, /* 0111 1111 1111 1111  1111 1111 1111 1111 */

        0x00000000, /* 0000 0000 0000 0000  0000 0000 0000 0000 */
        0x00000000, /* 0000 0000 0000 0000  0000 0000 0000 0000 */
        0x00000000, /* 0000 0000 0000 0000  0000 0000 0000 0000 */
        0x00000000, /* 0000 0000 0000 0000  0000 0000 0000 0000 */
    };

    uint32_t  start_line[] = {
        0x00000000, /* 0000 0000 0000 0000  0000 0000 0000 0000 */

                    /* ?>=< ;:98 7654 3210  /.-, +*)( '&%$ #"!  */
        0x00002000, /* 0000 0000 0000 0000  0010 0000 0000 0000 */

                    /* _^]\ [ZYX WVUT SRQP  ONML KJIH GFED CBA@ */
        0x87fffffe, /* 1000 0111 1111 1111  1111 1111 1111 1110 */

                    /*  ~}| {zyx wvut srqp  onml kjih gfed cba` */
        0x00000000, /* 0000 0000 0000 0000  0000 0000 0000 0000 */

        0x00000000, /* 0000 0000 0000 0000  0000 0000 0000 0000 */
        0x00000000, /* 0000 0000 0000 0000  0000 0000 0000 0000 */
        0x00000000, /* 0000 0000 0000 0000  0000 0000 0000 0000 */
        0x00000000, /* 0000 0000 0000 0000  0000 0000 0000 0000 */
    };

	u_char  lowcase[] = {
		"\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
		"\0\0\0\0\0\0\0\0\0\0\0\0\0-\0\0" "0123456789\0\0\0\0\0\0"
		"\0abcdefghijklmnopqrstuvwxyz\0\0\0\0\0"
		"\0abcdefghijklmnopqrstuvwxyz\0\0\0\0\0"
		"\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
		"\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
		"\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
		"\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
	};



    inline status
    error_(const char *msg)
    {
        // throw(msg);
       cerr << msg << endl;
        return spx_error;
    }

    inline int
    syntax_(const uint32_t table[8], int c)
    {
        return (table[((unsigned char)c >> 5)] & (1U << (((unsigned char)c) & 0x1f)));
    }

}

status
spx_http_syntax_start_line_request(string const & line)
{
    string::const_iterator  it = line.cbegin();
    size_t                  temp_len;
    enum    {
        spx_start = 0,
        spx_method,
        spx_sp_before_uri,
        spx_uri_start,
        spx_uri,
        spx_proto,
        spx_almost_done,
        spx_done
    } state;

    state = spx_start;

    while (state != spx_done)   {
        switch (state) {

            case spx_start: {
                if (syntax_(start_line, *it)) {
                    state = spx_method;
                    // add request start point to struct
                    break;
                }
                return error_("invalid line start : request line");
            }


            case spx_method: {// check method
                temp_len = line.find(' ', 0);
                switch (temp_len) {
                    case 3:
                        if (line.find("GET", 0, 3) != string::npos) {
                            // add method to struct or class_member
                            state = spx_sp_before_uri;
                            break;
                        }
                        if (line.find("PUT", 0, 3) != string::npos) {
                            // add method to struct or class_member
                            state = spx_sp_before_uri;
                        }
                        break;

                    case 4:
                        if (line.find("POST", 0, 4) != string::npos) {
                            // add method to struct or class_member
                            state = spx_sp_before_uri;
                            break;
                        }
                        if (line.find("HEAD", 0, 4) != string::npos) {
                            // add method to struct or class_member
                            state = spx_sp_before_uri;
                        }
                        break;

                    case 6:
                        if (line.find("DELETE", 0, 5) != string::npos) {
                            // add method to struct or class_member
                            state = spx_sp_before_uri;
                        }
                        break;

                    case 7:
                        if (line.find("OPTIONS", 0, 5) != string::npos) {
                            // add method to struct or class_member
                            state = spx_sp_before_uri;
                        }
                        break;

                    }
                }
                if (state != spx_sp_before_uri) {
                    return error_("invalid method : request line");
                }
                it += temp_len;
                break;


            case spx_sp_before_uri: {// check uri
                if (*it == ' ') {
                    ++it;
                    state = spx_uri_start;
                    break;
                }
                return error_("invalid uri or method : request line");
            }


            case spx_uri_start: {
                if (*it == '/') {
                    ++it;
                    state = spx_uri;
                    break;
                }
                return error_("invalid uri start : request line");
            }


            case spx_uri:   {// start uri check
                if (syntax_(usual, *it)) {
                    ++it;
                    break;
                }
                if (*it == ' ') {
                    ++it;
                    state = spx_proto;
                    break;
                }
                return error_("invalid uri : request line");
            }


            case spx_proto: {
                temp_len = line.find('\r', it - line.cbegin(), 8);

                // http_len = find_end_pos - &buf[pos];
                switch (http_len)   {
                    case 8:
                        if (strncmp(&buf[pos], "HTTP/1.1", 8) == 0) {
                            // add proto to struct or class_member
                            state = spx_almost_done;
                            break;
                        }
                        if (strncmp(&buf[pos], "HTTP/1.0", 8) == 0) {
                            // add proto to struct or class_member
                            state = spx_almost_done;
                        }
                        break;

                    default:
                        break;
                }
                if (state != spx_almost_done) {
                    return error_("invalid http version or end line : request line");
                }
                pos += http_len;
                break;
            }


            case spx_almost_done:   {
                if (buf[pos] == '\r' && buf[pos + 1] == '\n')   {
                    state = spx_done;
                    break;
                }
                return error_("invalid request end line : request line");
            }


            default:
                return error_("invalid request : request line");

        }
    }
    return spx_ok;
}

status
spx_http_syntax_header_line(char *buf)
{
    size_t  pos, key_len, value_len, temp;
	u_char c;
	char   *find_end_pos;
    enum    {
        spx_start = 0,
        spx_key,
        spx_sp_before_value,
        spx_value,
		spx_sp_after_value,
        spx_almost_done,
        spx_done
    } state;

    pos = 0;
    state = spx_start;

    while (state != spx_done)   {
        switch (state) {

            case spx_start: {
				switch (buf[pos]) {
					case '\r':
						state = spx_almost_done;
						++pos;
						break;

					default:
						state = spx_key;
				}
				break;
            }


            case spx_key:   {
				c = lowcase[buf[pos]];
				if (c)	{
					// add key to struct or class_member
					++pos;
					break;
				}
				switch (buf[pos]) {
					case ':':
						state = spx_sp_before_value;
						++pos;
						break;

					case '\r':
						if (buf[pos + 1] == '\n') {
							return spx_ok;
						}
						++pos;
						state = spx_almost_done;
                        break ;

					case '\n':
						return error_("invalid value NL found : header");

					default:
						break;
				}
				if (state != spx_sp_before_value) {
					return error_("invalid key : header");
				}
            }


            case spx_sp_before_value:   {
				switch (buf[pos]) {
					case ' ':
						++pos;
						break;

					default:
						state = spx_value;
				}
				break;
            }


            case spx_value: {
				switch (buf[pos]) {
					case ' ':
						state = spx_sp_after_value;
						++pos;
						break;

					case '\r':
						if (buf[pos + 1] == '\n') {
							return spx_ok;
						}
						++pos;
						state = spx_almost_done;
                        break ;

					case '\n':
						return error_("invalid value NL found : header");

					case '\0':
						return error_("invalid value NULL : header");

					default:
						++pos;
				}
				break;
            }


            case spx_sp_after_value:    {
				switch (buf[pos]) {
					case ' ':
						++pos;
						break;

					default:
						state = spx_value;
				}
				break;
            }


            case spx_almost_done:   {
				if (buf[pos] == '\n')   {
					state = spx_done;
					break;
				}
				return error_("invalid header end line : header");
            }


            default:
                return error_("invalid key or value : header");
        }

    }
    return spx_ok;
}
