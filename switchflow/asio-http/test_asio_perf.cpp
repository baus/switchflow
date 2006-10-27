//
// Copyright 2003-2006 Christopher Baus. http://baus.net/
// Read the LICENSE file for more information.

#include <cstdio>
#include <iostream>
#include <vector>
#include <valarray>
#include <boost/shared_ptr.hpp>
#include <boost/thread.hpp>
#include <boost/bind.hpp>
#include <asio.hpp>
#include <memory>

const std::size_t message_size = /**/ 1*1024 /*/ 64*1024 /**/;
const int port = 9999;
const std::size_t message_iterations = /** 100 /*/ 100000 /**/;

namespace asio = ::boost::asio;

class async_server;

void handle_receive_from(async_server* p_this,
                         const boost::asio::error& error,
                         size_t /*bytes_transferred*/);


namespace detail_test
{
  struct timed_scope
  {
    boost::xtime t0;
    boost::xtime t1;
    std::size_t n;
    
    std::vector<double> & results;
    
    inline timed_scope(std::vector<double> & r, std::size_t iterations = 1)
      : results(r), n(iterations)
    {
      boost::xtime_get(&t0,boost::TIME_UTC);
    }
    
    inline ~timed_scope()
    {
      boost::xtime_get(&t1,boost::TIME_UTC);
      double t = double(t1.sec)+double(t1.nsec)/double(1000000000);
      t -= double(t0.sec)+double(t0.nsec)/double(1000000000);
      std::cerr << "### TIME"
        << ": total = " << t
        << "; iterations = " << n
        << "; iteration = " << t/double(n)
        << "; iterations/second = " << double(n)/t
        << '\n';
      results.push_back(double(n)/t);
    }
  };
  
  template <typename out>
  out & result_summary(out & o, const std::vector<double> & results)
  {
    std::valarray<double> r(&results[1],results.size()-2);
    o << r.sum()/r.size();
    return o;
  }
  
  void sleep_for_secs(int n)
  {
    boost::xtime t;
    boost::xtime_get(&t,boost::TIME_UTC);
    t.sec += n;
    boost::thread::sleep(t);
  }
}

void run(boost::asio::demuxer* p_demuxer)
{
  p_demuxer->run();
}

struct async_server
{
  char io_buffer[message_size];
  boost::asio::demuxer demuxer;
  boost::asio::datagram_socket socket;
  std::auto_ptr<detail_test::timed_scope> timer;
  std::size_t message_count;
  std::size_t message_recount;
  std::auto_ptr<boost::thread> runner;
  std::vector<double> results;
  
  async_server()
    : socket(this->demuxer,boost::asio::ipv4::udp::endpoint(port))
    , message_count(0)
    , message_recount(0)
  {
    boost::asio::mutable_buffer receive_buffer(io_buffer, message_size);
    std::list<boost::asio::mutable_buffer> buffers;
    buffers.push_back(receive_buffer);
    socket.async_receive(buffers, 0, boost::bind(handle_receive_from,
                                                 this,
                                                 boost::asio::placeholders::error,
                                                 boost::asio::placeholders::bytes_transferred ) );
  }


  void start()
  {
    runner.reset(new boost::thread(boost::bind(run, &demuxer) ));
  }
  
  void stop()
  {
    this->demuxer.interrupt();
    this->runner->join();
    this->clear_timer();
  }
  
  void reset_timer(std::size_t i = 1)
  {
    this->message_recount = i;
    this->timer.reset(new detail_test::timed_scope(this->results,i));
  }
  
  void clear_timer()
  {
    this->timer.reset();
  }
};

struct sync_server;

void sync_server_run(sync_server* );

struct sync_server
{
  char io_buffer[message_size];
  boost::asio::demuxer demuxer;
  boost::asio::datagram_socket socket;
  std::auto_ptr<detail_test::timed_scope> timer;
  std::size_t message_count;
  std::size_t message_recount;
  std::auto_ptr<boost::thread> runner;
  std::vector<double> results;
  volatile bool running;
  
  sync_server()
    : socket(this->demuxer,boost::asio::ipv4::udp::endpoint(port))
    , message_count(0)
    , message_recount(0)
    , running(false)
  {
  }
  
  void handle_receive_from(
    const boost::asio::error& error,
    size_t /*bytes_transferred*/)
  {
    if (!error || error == boost::asio::error::message_size)
    {
      if (++message_count == this->timer->n)
      {
        this->clear_timer();
        message_count = 0;
        this->reset_timer(message_recount);
      }
    }
  }
  
  void start()
  {
    this->runner.reset(new boost::thread(
      boost::bind(sync_server_run,this) ));
  }
  
  void stop()
  {
    this->running = false;
    this->socket.close();
    this->runner->join();
    this->clear_timer();
  }
  
  void reset_timer(std::size_t i = 1)
  {
    this->message_recount = i;
    this->timer.reset(new detail_test::timed_scope(this->results,i));
  }
  
  void clear_timer()
  {
    this->timer.reset();
  }
};

struct sync_client
{
  char io_buffer[message_size];
  boost::asio::demuxer demuxer;
  boost::asio::ipv4::host_resolver host_resolver;
  boost::asio::ipv4::host host;
  boost::asio::ipv4::udp::endpoint receiver_endpoint;
  boost::asio::datagram_socket socket;
  
  sync_client()
    : host_resolver(this->demuxer)
    , receiver_endpoint(port)
    , socket(this->demuxer,boost::asio::ipv4::udp::endpoint(0))
  {
    host_resolver.get_host_by_name(host, "127.0.0.1");
    receiver_endpoint.address(host.address(0));
  }
  
  void send()
  {
    socket.send_to(
      boost::asio::buffer(io_buffer, message_size),
      0, receiver_endpoint);
  }
};

void handle_receive_from(async_server* p_this,
                         const boost::asio::error& error,
                         size_t /*bytes_transferred*/)
{
  if (!error || error == boost::asio::error::message_size)
  {
    if (++p_this->message_count == p_this->timer->n)
    {
      p_this->clear_timer();
      p_this->message_count = 0;
      p_this->reset_timer(p_this->message_recount);
    }
    p_this->socket.async_receive(
        boost::asio::buffer(p_this->io_buffer, message_size),
        0,
        boost::bind(handle_receive_from,
                    p_this, boost::asio::placeholders::error,
                    boost::asio::placeholders::bytes_transferred ) );
  }
}

void sync_server_run(sync_server* p_this)
{
  p_this->running = true;
  boost::asio::mutable_buffer receive_buffer(p_this->io_buffer, message_size);
  std::list<boost::asio::mutable_buffer> buffers;
  buffers.push_back(receive_buffer);
  
  while (p_this->running)
  {
    boost::asio::error error;
    std::size_t bytes_transferred =
      p_this->socket.receive(buffers, 0, boost::asio::assign_error(error));
    p_this->handle_receive_from(error,bytes_transferred);
    if (error && error != boost::asio::error::message_size) break;
  }
  p_this->running = false;
}
  


int main()
{
  sync_client c0;
  {
    async_server s0;
    s0.start();
    detail_test::sleep_for_secs(2);
    std::cerr << "--- ASYNC...\n";
    s0.reset_timer(message_iterations);
    for (std::size_t m = 0; m < message_iterations*(1+4+1); ++m)
    {
      c0.send();
    }
    s0.stop();
    detail_test::result_summary(
      std::cerr << "--  ...ASYNC: average iterations/second = ",
      s0.results) << "\n";
  }
  detail_test::sleep_for_secs(2);
  {
    sync_server s0;
    s0.start();
    detail_test::sleep_for_secs(2);
    std::cerr << "--- SYNC...\n";
    s0.reset_timer(message_iterations);
    for (std::size_t m = 0; m < message_iterations*(1+4+1); ++m)
    {
      c0.send();
    }
    s0.stop();
    detail_test::result_summary(
      std::cerr << "--  ...SYNC: average iterations/second = ",
      s0.results) << "\n";
  }
  return 1;
}
