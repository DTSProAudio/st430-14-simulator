//
// server.cpp
// ~~~~~~~~~~
//
// Copyright (c) 2003-2013 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include "server.hpp"
#include <signal.h>
#include <utility>

namespace http {
namespace server {

server::server(boost::asio::io_service& io_service,
               const std::string& address,
               const std::string& port)
    :   io_service_(io_service),
        signals_(io_service_),
        acceptor_(io_service_),
        connection_manager_(),
        socket_(io_service_)
{
  // Register to handle the signals that indicate when the server should exit.
  // It is safe to register for the same signal multiple times in a program,
  // provided all registration for the specified signal is made through Asio.
  signals_.add(SIGINT);
  signals_.add(SIGTERM);
#if defined(SIGQUIT)
  signals_.add(SIGQUIT);
#endif // defined(SIGQUIT)

  do_await_stop();

  // Open the acceptor with the option to reuse the address (i.e. SO_REUSEADDR).
  boost::asio::ip::tcp::resolver resolver(io_service_);
  boost::asio::ip::tcp::endpoint endpoint = *resolver.resolve({address, port});
  acceptor_.open(endpoint.protocol());
  acceptor_.set_option(boost::asio::ip::tcp::acceptor::reuse_address(true));
  acceptor_.bind(endpoint);
  acceptor_.listen();

  do_accept();
}

    void server::SetPopulateContentCallback(request_handler::Callback iCallback)
    {
        request_handler_.SetPopulateContentCallback(iCallback);
    }
    
    void server::SetMaxEditUnitsPerRequest(int32_t iUnits)
    {
        request_handler_.SetMaxEditUnitsPerRequest(iUnits);
    }
    
    int32_t server::GetMaxEditUnitsPerRequest(void)
    {
        return request_handler_.GetMaxEditUnitsPerRequest();
    }
    
    void server::SetMaxEditUnitsAheadOfCurrentEditUnit(int32_t iUnits)
    {
        request_handler_.SetMaxEditUnitsAheadOfCurrentEditUnit(iUnits);
    }
    
    int32_t server::GetMaxEditUnitsAheadOfCurrentEditUnit(void)
    {
        return request_handler_.GetMaxEditUnitsAheadOfCurrentEditUnit();
    }
    
    void server::SetMillisecondsPerFrame(int32_t iMilliseconds)
    {
        request_handler_.SetMillisecondsPerFrame(iMilliseconds);
    }
    
    int32_t server::GetMillisecondsPerFrame(void)
    {
        return request_handler_.GetMillisecondsPerFrame();
    }
    
    void server::SetCurrentFrameCallback(SMPTE_SYNC::CurrentFrameCallback iCallback)
    {
        request_handler_.SetCurrentFrameCallback(iCallback);
    }

void server::do_accept()
{
  acceptor_.async_accept(socket_,
      [this](boost::system::error_code ec)
      {
        // Check whether the server was stopped by a signal before this
        // completion handler had a chance to run.
        if (!acceptor_.is_open())
        {
          return;
        }

        if (!ec)
        {
            this->SetState(eState_Connected);
            
          connection_manager_.start(std::make_shared<connection>(
              std::move(socket_), connection_manager_, request_handler_));
        }

        do_accept();
      });
}

void server::do_await_stop()
{
  signals_.async_wait(
      [this](boost::system::error_code /*ec*/, int /*signo*/)
      {
        // The server is stopped by cancelling all outstanding asynchronous
        // operations. Once all operations have finished the io_service::run()
        // call will exit.
        acceptor_.close();
        connection_manager_.stop_all();
      });
}

} // namespace server
} // namespace http