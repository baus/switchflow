//
// Copyright (C) Christopher Baus.  All rights reserved.
#include <string>
#include <http/url.hpp>

#include "host_map.hpp"

void build_host_map(const config_file& config,
                    std::map<std::string, std::pair<httplib::URL, bool> >& host_map)
{
  for(int i = 0; i < config["virtual-host"].get_array_size(); ++i)
  {
    host_map[config["virtual-host"][(size_t)i]["host-name"].read<std::string>()] =
      std::make_pair<httplib::URL, bool>(
        httplib::URL(config["virtual-host"][(size_t)i]["forward-url"].read<std::string>().c_str(), true),
        config["virtual-host"][(size_t)i]["preserve-host"].read<bool>());
  }
}
