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
void test_performance(listT& l, int num_insert_ops)
{
  l.clear();
  int outer_limit = num_insert_ops/10;
  boost::posix_time::ptime start = boost::posix_time::microsec_clock::universal_time();

  
  for(int x=0; x < outer_limit; ++x){
    for(int i = 0; i <10; ++i){
      l.insert(l.end(), i);
    }
    while(!l.empty()){
      l.erase(l.begin());
    }
  }
  
  boost::posix_time::ptime end = boost::posix_time::microsec_clock::universal_time();
  
  std::cout<<to_simple_string(end - start)<<std::endl;
  std::cout<<"total insert/erase ops: "<<outer_limit * 10 * 2<<std::endl;
}

template<typename listT>
void dump_list(const listT& l)
{
  typename listT::const_iterator cur =
    l.begin();

  typename listT::const_iterator end =
    l.end();

  for(;cur != end; ++cur){
    std::cout<<*cur<<std::endl;
  }
}

void test()
{
  switchflow::util::pooled_list<int>::pool list_pool(20);
  switchflow::util::pooled_list<int>::pooled_list list1(list_pool);
  switchflow::util::pooled_list<int>::pooled_list list2(list_pool);
  for(int i = 30; i > 21; --i){
    list2.insert(list2.end(), i);
  }

  dump_list(list2);
  list2.sort();
  dump_list(list2);
  
  dump_list(list1);
  dump_list(list2);
  
  switchflow::util::pooled_list<int>::reverse_iterator rcur =
    list1.rbegin();
  
  switchflow::util::pooled_list<int>::reverse_iterator rend =
    list1.rend();

  for(;rcur != rend; ++rcur){
    std::cout<<*rcur<<std::endl;
  }

  
  list1.remove(5);
  dump_list(list1);
  
  
  std::cout<<"done"<<std::endl;

}

int main()
{
  std::list<int> std_list;
  switchflow::util::pooled_list<int>::pool list_pool(10);
  switchflow::util::pooled_list<int>::pooled_list list1(list_pool);
  std::cout<<"done allocating..."<<std::endl;

  std::cout<<"Testing std::list"<<std::endl;
  test_performance(std_list, 50000000);
  std::cout<<std::endl<<"Testing pooled_list"<<std::endl;
  test_performance(list1, 50000000);

  return 0;

}
