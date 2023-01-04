#pragma once
#ifndef __SPX__SESSION__STORAGE__HPP
#define __SPX__SESSION__STORAGE__HPP

#include "spx_core_type.hpp"
#include "spx_core_util_box.hpp"

/* for hashfuncs*/
#include <bitset>
#include <cstdint>
#include <time.h>

#define SESSIONID "sessionID="

typedef struct Cookie {
	typedef std::map<std::string, std::string> key_val_t;
	key_val_t								   content;

	void
	parse_cookie_header(const std::string& cookie_header) {
		size_t start = 0;
		while (start < cookie_header.size()) {
			size_t end = cookie_header.find(';', start);
			if (end == std::string::npos)
				end = cookie_header.size();
			std::string session	  = cookie_header.substr(start, end - start);
			size_t		equal_pos = session.find('=');
			size_t		key_pos	  = 0;
			if (equal_pos != std::string::npos) {
				std::string key	  = session.substr(key_pos, equal_pos);
				std::string value = session.substr(equal_pos + 1);
				if (!key.empty()) // additional valid check for key needed
					content[key] = value;
			}
			start = end + 1;
			while (cookie_header[start] == ' ')
				++start;
		}
	}
	std::string
	to_string() {
		std::stringstream ss;
		for (key_val_t::iterator it = content.begin(); it != content.end(); ++it) {
			ss << it->first << "=" << it->second << "; ";
		}
		std::string result = ss.str();
		size_t		pos	   = result.find_last_of(";");
		return result.substr(0, pos);
	}

} cookie_t;

typedef struct Session {

	typedef std::pair<std::string, std::string> session_content;

	int count_;
	Session(int i) {
		count_ = i;
	}
	~Session() { }
	void
	addCount() {
		count_++;
	}

} session_t;

class SessionStorage {
	typedef std::string						SessionID;
	typedef std::string						SessionValue;
	typedef std::pair<SessionID, session_t> session_key_val;
	typedef std::map<SessionID, session_t>	storage_t;

	storage_t storage_;
	int		  count;

public:
	SessionStorage();
	~SessionStorage();

	// void
	//  parse_session_header(const std::string& session_header);
	bool		is_key_exsits(const std::string& c_key) const;
	session_t&	find_value_by_key(std::string& c_key);
	std::string find_session_to_string(const std::string& c_key);
	// void		add_new_session(SessionID id, session_t session);
	void		add_new_session(SessionID id);
	std::string make_hash(uintptr_t& seed_in);
	void		addCount();
};
typedef SessionStorage session_storage_t;

#endif
