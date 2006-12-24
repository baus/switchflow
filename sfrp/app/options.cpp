//
// Copyright 2003-2006 Christopher Baus. http://baus.net/
// Read the LICENSE file for more information.

#include <iostream>
#include <string>

#include <event.h>
#include <boost/program_options.hpp>

#include <util/logger.hpp>
#include <util/scope_guard.hpp>

#include "version.hpp"
#include "options.hpp"


options::options():
  config_file_("/etc/sfrp.conf")
{
  
}

options::~options()
{
  
}

options::STATUS options::process_command_line(int argc, char *argv[])
{
  namespace po = boost::program_options;

  po::options_description desc("Allowed options");
  desc.add_options()
    ("help", "display help message")
    ("version", "display sfrp version and exit")
    ("fullversion", "display sfrp and libevent version and exit")
    ("copyright", "display copyright and exit")
    ("file,f", po::value<std::string>(), "configuration file");
  
  try{
    
    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, desc), vm);
    po::notify(vm);
    

    if(vm.count("version")){
      std::cout<<SFRP_VERSION<<"_"<<SVNVersion()<<std::endl;
      return INFO;
    }

    if(vm.count("fullversion")){
      std::cout<<"sfrp version: "<<SFRP_VERSION<<"_"<<SVNVersion()<<std::endl;
      std::cout<<"libevent version: "<<event_get_version()<<std::endl;
      return INFO;
    }

    //
    // Don't print the copyright when showing the version
    print_short_copyright();
    
    if(vm.count("help")) {
      std::cout<<desc<<std::endl;
      return INFO;
    }
    
    if(vm.count("copyright")){
      print_copyright();
      return INFO;
    } 
    
    if(vm.count("file")) {
      config_file_ = vm["file"].as<std::string>();
    }
  }
  catch(po::error e){
    std::cout<<e.what()<<std::endl<<std::endl;
    std::cout<<desc<<std::endl;
    return ERROR;
  }  
  return RUN;
}

std::string options::get_config_file()
{
  return config_file_;
}

void options::print_short_copyright()
{
  std::cout<<"SwitchFlow Reverse Proxy"<<std::endl;
  std::cout<<"Copyright 2003-2006, Christopher Baus. All rights reserved."<<std::endl;
  std::cout<<"See the LICENSE file for details, or visit http://www.baus.net/"<<std::endl<<std::endl;
}

void options::print_copyright()
{
      printf(
  "THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS\n"
  "\"AS IS\" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT\n"
  "LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR\n"
  "A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT\n"
  "OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,\n"
  "SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT\n"
  "LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,\n"
  "DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY\n"
  "THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT\n"
  "(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE\n"
  "OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.\n"
  "\n"
  "\n"
  "This product includes software developed by Niels Provos.\n"
  "\n"
  "[ libevent ]\n"
  "\n"
  "Copyright 2000-2003 Niels Provos <provos@citi.umich.edu>\n"
  "All rights reserved.\n"
  "\n"
  "Redistribution and use in source and binary forms, with or without\n"
  "modification, are permitted provided that the following conditions\n"
  "are met:\n"
  "1. Redistributions of source code must retain the above copyright\n"
  "   notice, this list of conditions and the following disclaimer.\n"
  "2. Redistributions in binary form must reproduce the above copyright\n"
  "   notice, this list of conditions and the following disclaimer in the\n"
  "   documentation and/or other materials provided with the distribution.\n"
  "3. All advertising materials mentioning features or use of this software\n"
  "   must display the following acknowledgement:\n"
  "      This product includes software developed by Niels Provos.\n"
  "4. The name of the author may not be used to endorse or promote products\n"
  "   derived from this software without specific prior written permission.\n"
  "\n"
  "THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR\n"
  "IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES\n"
  "OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.\n"
  "IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,\n"
  "INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT\n"
  "NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,\n"
  "DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY\n"
  "THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT\n"
  "(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF\n"
  "THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.\n"
    );
}
