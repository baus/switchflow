//
// Copyright 2003-2006 Christopher Baus. http://baus.net/
// Read the LICENSE file for more information.

#ifndef SFRP_HOST_MAP
#define SFRP_HOST_MAP

#include <map>
#include <util/config_file.hpp>
#include <http/url.hpp>
#include <string>

void build_host_map(const config_file& config, std::map<std::string, std::pair<httplib::url, bool> >& host_map);

#endif // SFRP_HOST_MAP
