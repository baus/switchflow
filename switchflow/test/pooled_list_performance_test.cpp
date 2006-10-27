//
// Copyright 2003-2006 Christopher Baus. http://baus.net/
// Read the LICENSE file for more information.

// Copyright (c) 2006
// Christopher Baus http://baus.net/ (christopher at baus dot net)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <iostream>
#include <list>
#include <string>
#include <boost/date_time/gregorian/gregorian.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

// Assumes that the switchflow directory is in the include path
// and pooled_list class is in a subdirectory called util.
#include <util/pooled_list.hpp>
 
                     
template<typename listT>
void test_performance(listT& l, int num_ops)
{
  using namespace boost::posix_time;
  l.clear();

  ptime start = microsec_clock::universal_time();

  
  for(int x=0; x < num_ops; ++x){
    for(int i = 0; i <10; ++i){
      l.insert(l.begin(), i);
    }
    while(!l.empty()){
      l.erase(l.begin());
    }
  }
  
  ptime end = microsec_clock::universal_time();
  
  std::cout<<to_simple_string(end - start)<<std::endl;
  std::cout<<"total insert/erase ops: "<<num_ops * 10 * 2<<std::endl;
}


int main()
{
  std::list<int> std_list;
  switchflow::util::pooled_list<int>::pool list_pool(10);
  switchflow::util::pooled_list<int>::pooled_list list1(list_pool);
  std::cout<<"done allocating..."<<std::endl;

  std::cout<<"Testing std::list"<<std::endl;
  test_performance(std_list, 5000000);

  std::cout<<std::endl<<"Testing pooled_list"<<std::endl;
  test_performance(list1, 5000000);

  std::cout<<std::endl<<"Testing vector"<<std::endl;
  std::vector<int> vect;
  test_performance(vect, 5000000);
  return 0;

}
