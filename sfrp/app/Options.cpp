// 
// Copyright (C) Christopher Baus.  All rights reserved.
//
#include <iostream>
#include <string>

#include <boost/program_options.hpp>

#include <util/logger.hpp>
#include <util/ScopeGuard.h>

#include "version.hpp"
#include "Options.h"


Options::Options():
  m_configFile("/etc/sfrp.conf")
{
  
}

Options::~Options()
{
  
}

Options::STATUS Options::processCommandLine(int argc, char *argv[])
{
  namespace po = boost::program_options;

  po::options_description desc("Allowed options");
  desc.add_options()
    ("help", "display help message")
    ("version", "display version and exit")
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

    //
    // Don't print the copyright when showing the version
    printShortCopyright();
    
    if(vm.count("help")) {
      std::cout<<desc<<std::endl;
      return INFO;
    }
    
    if(vm.count("copyright")){
      printCopyright();
      return INFO;
    } 
    
    if(vm.count("file")) {
      m_configFile = vm["file"].as<std::string>();
    }
  }
  catch(po::error e){
    std::cout<<e.what()<<std::endl<<std::endl;
    std::cout<<desc<<std::endl;
    return ERROR;
  }  
  return RUN;
}

std::string Options::getConfigFile()
{
  return m_configFile;
}

void Options::printShortCopyright()
{
  std::cout<<"SwitchFlow Reverse Proxy"<<std::endl;
  std::cout<<"Copyright 2003-2006, Christopher Baus. All rights reserved."<<std::endl;
  std::cout<<"For information e-mail christopher@baus.net or visit http://www.baus.net/"<<std::endl<<std::endl;
}

void Options::printCopyright()
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
