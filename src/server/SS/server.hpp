//
// server.hpp
// ~~~~~~~~~~
//
// Copyright (c) 2003-2013 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef HTTP_SERVER_HPP
#define HTTP_SERVER_HPP

#include <string>
#include "boost/asio.hpp"
#include "connection.hpp"
#include "connection_manager.hpp"
#include "request_handler.hpp"
#include "SS_State.h"

namespace http {
namespace server {

/// The top-level class of the HTTP server.
class server : public SMPTE_SYNC::SS_State
{
public:
    server(const server&) = delete;
    server& operator=(const server&) = delete;

    virtual ~server()
    {
    }
    
    /// Construct the server to listen on the specified TCP address and port, and
    /// serve up files from the given directory.
    explicit server(boost::asio::io_service& io_service,
                    const std::string& address,
                    const std::string& port);

    virtual void SetPopulateContentCallback(request_handler::Callback iCallback);

    virtual void SetMaxEditUnitsPerRequest(int32_t iUnits);
    
    virtual int32_t GetMaxEditUnitsPerRequest(void);
    
    virtual void SetMaxEditUnitsAheadOfCurrentEditUnit(int32_t iUnits);
    
    virtual int32_t GetMaxEditUnitsAheadOfCurrentEditUnit(void);
    
    virtual void SetMillisecondsPerFrame(int32_t iMilliseconds);
    
    virtual int32_t GetMillisecondsPerFrame(void);
    
    virtual void SetCurrentFrameCallback(SMPTE_SYNC::CurrentFrameCallback iCallback);

private:
  /// Perform an asynchronous accept operation.
  void do_accept();

  /// Wait for a request to stop the server.
  void do_await_stop();

  /// The io_service used to perform asynchronous operations.
  boost::asio::io_service&    io_service_;

  /// The signal_set is used to register for process termination notifications.
  boost::asio::signal_set signals_;

  /// Acceptor used to listen for incoming connections.
  boost::asio::ip::tcp::acceptor acceptor_;

  /// The connection manager which owns all live connections.
  connection_manager connection_manager_;

  /// The next socket to be accepted.
  boost::asio::ip::tcp::socket socket_;

  /// The handler for all incoming requests.
  request_handler request_handler_;
};

} // namespace server
} // namespace http

#endif // HTTP_SERVER_HPP