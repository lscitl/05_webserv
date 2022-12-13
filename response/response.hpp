#ifndef __RESPONSE_HPP__
#define __RESPONSE_HPP__

#include <algorithm>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "response_form.hpp"

struct Response {
	typedef std::pair<std::string, std::string> header;

private:
	std::vector<header> headers_;
	int					version_minor_;
	int					version_major_;
	std::vector<char>	body_;
	bool				keep_alive_;
	unsigned int		status_code_;
	std::string			status_;

	std::string
	make() const {
		std::stringstream stream;
		stream << "HTTP/" << 1 << "." << 1 << " " << status_
			   << "\r\n";
		for (std::vector<Response::header>::const_iterator it = headers_.begin();
			 it != headers_.end(); ++it)
			stream << it->first << ": " << it->second << "\r\n";
		std::string data(body_.begin(), body_.end());
		stream << data << "\r\n";
		return stream.str();
	}

	void
	generateStatus_(enum http_status status) {
		std::stringstream stream;
		stream << status << " " << http_status_sstr(status) << "\r\n";
		status_ = stream.str();
	};

public:
	// constructor
	Response()
		: version_minor_(1)
		, version_major_(1)
		, status_code_(HTTP_STATUS_BAD_REQUEST) {
		generateStatus_(HTTP_STATUS_BAD_REQUEST);
	};

	Response(enum http_status status)
		: version_minor_(1)
		, version_major_(1)
		, status_code_(status) {
		generateStatus_(status);
	};

	// make
	std::string
	make_test() {
		std::stringstream stream;
		stream << "HTTP/" << version_major_ << "." << version_minor_ << " " << status_
			   << "\r\n";
		for (std::vector<Response::header>::const_iterator it = headers_.begin();
			 it != headers_.end(); ++it)
			stream << it->first << ": " << it->second << "\r\n";
		std::string data(body_.begin(), body_.end());
		stream << data << "\r\n";
		return stream.str();
	}
};

/*
int main( void ) {
	std::cout << HTTP_STATUS_BAD_REQUEST << std::endl;
	std::cout << http_status_str( HTTP_STATUS_NOT_ACCEPTABLE ) << std::endl;
}

HTTP/1.1 400 Bad Request
Server: nginx/1.23.3
Date: Tue, 13 Dec 2022 15:45:26 GMT
Content-Type: text/html
Content-Length: 157
Connection: close

<html>
<head><title>400 Bad Request</title></head>
<body>
<center><h1>400 Bad Request</h1></center>
<hr><center>nginx/1.23.3</center>
</body>
</html>


*/

#endif