//
// Copyright 2003-2006 Christopher Baus. http://baus.net/
// Read the LICENSE file for more information.

// configfiletest.cpp : Defines the entry point for the console application.
//
#include <iostream>

#include <util/config_file.hpp>

int main(int argc, char* argv[])
{
  config_file test_config_file;
  test_config_file.parse_file("test_config.txt");
  test_config_file.dump(std::cout);
  return 0;
}

