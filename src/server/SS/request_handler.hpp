//
// request_handler.hpp
// ~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2013 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef HTTP_REQUEST_HANDLER_HPP
#define HTTP_REQUEST_HANDLER_HPP

#include <string>
#include <vector>
#include "boost/function.hpp"
#include "DataTypes.h"

namespace http {
namespace server {

struct reply;
struct request;

/// The common handler for all incoming requests.
class request_handler
{
public:
    typedef boost::function<bool(const std::string &iDataEssenceCodingUL_,
                                 int32_t iStart,
                                 int32_t iCount,
                                 const std::string &iEncryptionType,
                                 std::vector<char> &iContent)> Callback;
    
    request_handler(const request_handler&) = delete;
    request_handler& operator=(const request_handler&) = delete;

    explicit request_handler(void);

    /// Handle a request and produce a reply.
    void handle_request(const request& req, reply& rep);

    void SetPopulateContentCallback(Callback iCallback);

    void SetMaxEditUnitsPerRequest(int32_t iUnits);
    int32_t GetMaxEditUnitsPerRequest(void);
    
    void SetMaxEditUnitsAheadOfCurrentEditUnit(int32_t iUnits);
    int32_t GetMaxEditUnitsAheadOfCurrentEditUnit(void);

    void SetMillisecondsPerFrame(int32_t iMilliseconds);
    int32_t GetMillisecondsPerFrame(void);

    void SetCurrentFrameCallback(SMPTE_SYNC::CurrentFrameCallback iCallback);

private:
    /// Perform URL-decoding on a string. Returns false if the encoding was
    /// invalid.
    static bool url_decode(const std::string& in, std::string& out);
    
    Callback populateContentCallback_;
    SMPTE_SYNC::CurrentFrameCallback currentFrameCallback_;

    int32_t         maxEditUnitsPerRequest_;
    int32_t         maxEditUnitsAheadOfCurrentEditUnitToRequest_;
    int32_t         millisecondsPerFrame_;
};

} // namespace server
} // namespace http

#endif // HTTP_REQUEST_HANDLER_HPP