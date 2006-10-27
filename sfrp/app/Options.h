//
// Copyright 2003-2006 Christopher Baus. http://baus.net/
// Read the LICENSE file for more information.

//
// Copyright (C) Christopher Baus.  All rights reserved.
//

#ifndef OPTIONS_H
#define OPTIONS_H

#include <string>

class Options
{
public:
  Options();
  virtual ~Options();

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
  STATUS processCommandLine(int argc, char *argv[]);

  //
  // return the location of the configuration file.
  //
  std::string getConfigFile();

  void printCopyright();
  void printShortCopyright();
private:
  //
  // stores the location of the configuration file.
  //
  std::string m_configFile;
};


#endif // OPTIONS_H
