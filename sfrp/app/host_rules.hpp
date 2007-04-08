//
// Copyright 2003-2007 Christopher Baus. http://baus.net/
// Read the LICENSE file for more information.

#ifndef SFRP_HOST_RULES
#define SFRP_HOST_RULES

#include <http/url.hpp>
#include <util/config_file.hpp>
#include <string>

class host_rule
{
public:
  host_rule(const char* match_path, const httplib::url& forward_url):
    match_path_(match_path), forward_url_(forward_url)
    {}

  friend class host_rules;
  
private:
  std::string match_path_;
  httplib::url forward_url_;
};

class host_rules
{
public:
  host_rules(){}
  host_rules(const httplib::url& default_forward_url, bool preserve_host):
    default_forward_url_(default_forward_url), preserve_host_(preserve_host)
    {
    }

  void add_rule(const host_rule& host_rule)
    {
      rules_.push_back(host_rule);
    }

  const httplib::url& find_forward_url(const char* path) const
    {
      std::list<host_rule>::const_iterator cur = rules_.begin();
      std::list<host_rule>::const_iterator end = rules_.end();
      for(;cur != end;++cur){
        if(cur->match_path_ == path){
          return cur->forward_url_;
        }
      }
      return default_forward_url_;
    }

  bool preserve_host() const
    {
      return preserve_host_;
    }
  
private:
  std::list<host_rule> rules_;
  httplib::url default_forward_url_;
  bool preserve_host_;
};

void build_host_rules(const switchflow::util::config_file& config,
                      std::map<std::string, host_rules>& host_rules_map);

#endif // SFRP_HOST_RULES
