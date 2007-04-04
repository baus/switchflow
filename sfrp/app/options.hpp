//
// Copyright 2003-2006 Christopher Baus. http://baus.net/
// Read the LICENSE file for more information.

#ifndef OPTIONS_H
#define OPTIONS_H

#include <string>

class options
{
public:
  options();
  virtual ~options();

  enum STATUS
  {
    RUN,   // the program should continue run
    INFO,  // information was displayed and the program should exit
    ERROR, // an error occurred and the program should exit.
  };

  //
  // Parse commandline and set internal variables
  // which can be retrieved at a latter time.
  //
  // Also display command line errors and help to stdout.
  //
  // @return STATUS, please review STATUS types.
  //
  STATUS process_command_line(int argc, char *argv[]);

  //
  // return the location of the configuration file.
  //
  std::string get_config_file();

  void print_copyright();
  void print_short_copyright();
private:
  //
  // stores the location of the configuration file.
  //
  std::string config_file_;
};


#endif // OPTIONS_H
