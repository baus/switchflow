//
// Copyright 2003-2006 Christopher Baus. http://baus.net/
// Read the LICENSE file for more information.

#include <iostream>
#include <list>
#include <string>
#include <boost/date_time/gregorian/gregorian.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

class i_strategy
{
public:
  virtual void execute() = 0;
};

class context
{
public:
  context(i_strategy& strategy):strategy_(strategy)
  {
  
  } 

  void execute()
  {
    strategy_.execute();
  }

  i_strategy& strategy_;
  
};


class strategy_string_test :public i_strategy
{
public:
  void execute()
  {
    std::string("Creating a string allocates and deletes memory from the heap");
  }
};

template <typename policy>
class policy_example
{
public:
  void execute()
  {
    my_policy_.execute();
  }
  
  policy my_policy_;
};



class strategy_string_allocate :public i_strategy
{
public:
  void execute()
  {
    std::string("Creating a string allocates and deletes memory from the heap");
  }
};


class policy_string_allocate
{
public:
  void execute()
  {
    std::string foo("Creating a string allocates and deletes memory from the heap");
  }
};

int test_int = 0;

class strategy_int_increment: public i_strategy
{
  public:
  void execute()
  {
    ++test_int;
  }
};

class policy_int_increment
{
  public:
  void execute()
  {
    ++test_int;
  }
};


using namespace boost::posix_time;
                     
void test_strategy_performance(i_strategy& strategy)
{
  ptime start = microsec_clock::universal_time();
  
  strategy_string_test my_strategy;

  context my_context(strategy);

  for(int i = 0; i < 1000000; ++i){
    my_context.execute();
  }
  ptime end = microsec_clock::universal_time();
  std::cout<<to_simple_string(end - start)<<std::endl;
}

template<class policy_class>
void test_policy_performance()
{
  ptime start = microsec_clock::universal_time();
  
  policy_example<policy_class> policy_object;

  for(int i = 0; i < 1000000; ++i){
    policy_object.execute();
  }

  ptime end = microsec_clock::universal_time();
  
  std::cout<<to_simple_string(end - start)<<std::endl;
}


int main()
{
  std::cout<<"Testing Policy Performance"<<std::endl;
  test_policy_performance<policy_string_allocate>();
  test_policy_performance<policy_int_increment>();
  

  std::cout<<"Testing Strategy Performance"<<std::endl;
  
  strategy_string_allocate s1;
  test_strategy_performance(s1);

  strategy_int_increment s2;
  test_strategy_performance(s2);


  return 0;
}
