//
// Copyright 2003-2006 Christopher Baus. http://baus.net/
// Read the LICENSE file for more information.

#include <string>

#include <util/config_file.hpp>
#include <http/url.hpp>

#include "host_rules.hpp"

void build_host_rules(const switchflow::util::config_file& config,
                      std::map<std::string, host_rules>& host_rules_map)
{
  for(size_t i = 0; i < config["virtual-host"].get_array_size(); ++i){
    httplib::url url(config["virtual-host"][i]["default-forward-url"].read<std::string>().c_str(), true);
    host_rules rules(url,
                     config["virtual-host"][i]["preserve-host"].read<bool>());
    
    for(size_t cur_rule = 0;
        cur_rule < config["virtual-host"][i]["rule"].get_array_size();
        ++cur_rule){
      switchflow::util::config_file::value cur_rule_value =   
      config["virtual-host"][i]["rule"][cur_rule];
      httplib::url default_url(cur_rule_value["forward-url"].read<std::string>().c_str(), true);
      host_rule rule(cur_rule_value["match-path"].read<std::string>().c_str(), default_url);
      rules.add_rule(rule);
    }
    
    host_rules_map[config["virtual-host"][i]["host-name"].read<std::string>()] = rules;
      
  }
}
