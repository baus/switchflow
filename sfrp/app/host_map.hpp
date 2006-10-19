//
// Copyright (c) Christopher Baus.  All rights reserved.
#ifndef SFRP_HOST_MAP
#define SFRP_HOST_MAP

#include <map>
#include <util/config_file.hpp>
#include <http/url.hpp>
#include <string>

void build_host_map(const config_file& config, std::map<std::string, std::pair<httplib::URL, bool> >& host_map);

#endif
