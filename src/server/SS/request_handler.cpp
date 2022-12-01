//
// request_handler.cpp
// ~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2013 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include "request_handler.hpp"

#include <fstream>
#include <sstream>
#include <string>

#include "mime_types.hpp"
#include "reply.hpp"
#include "request.hpp"
#include "Logger.h"

// Code to sleep a thread
//
//#include "boost/date_time/posix_time/posix_time.hpp"
//#include "boost/thread/thread.hpp"
//boost::this_thread::sleep(boost::posix_time::milliseconds(100));

namespace http {
namespace server {

    request_handler::request_handler() :
          maxEditUnitsPerRequest_(240) // approximately 10 seconds
        , maxEditUnitsAheadOfCurrentEditUnitToRequest_(240) // approximately 10 seconds
        , millisecondsPerFrame_(42) // 1000 / 24
    {
    }

    void request_handler::handle_request(const request& req, reply& rep)
    {
        SMPTE_SYNC_LOG << "request_handler::handle_request start";
        
        // Decode url to path.
        std::string request_path;
        if (!url_decode(req.uri, request_path))
        {
            rep = reply::stock_reply(reply::bad_request);
            return;
        }

        static const std::string key_base_path_v1 = "/v1/auxdata/editunits";
        static const std::string key_dataEssenceCodingUL_ = "coding_UL";
        static const std::string key_start = "start";
        static const std::string key_count = "count";
        static const std::string key_accept = "accept";
        
        // Find the base path
        //
        if (request_path.find(key_base_path_v1) == std::string::npos)
        {
            rep = reply::stock_reply(reply::bad_request);
            return;
        }

        // Check to make sure all of our request parameters are present
        //
        if (   request_path.find(key_dataEssenceCodingUL_) == std::string::npos
            && request_path.find(key_start) == std::string::npos
            && request_path.find(key_count) == std::string::npos
            && request_path.find(key_accept) == std::string::npos)
        {
            rep = reply::stock_reply(reply::bad_request);
            return;
        }

        std::string val_dataEssenceCodingUL_;
        std::string val_start;
        std::string val_count;
        std::string val_accept;
        
        std::string::size_type strStart;
        std::string::size_type strEnd;
        
        strStart = request_path.find(key_dataEssenceCodingUL_ + "=") + (key_dataEssenceCodingUL_ + "=").length();
        strEnd = request_path.find("&", strStart);
        val_dataEssenceCodingUL_ = request_path.substr(strStart, strEnd - strStart);

        strStart = request_path.find(key_start + "=") + (key_start + "=").length();
        strEnd = request_path.find("&", strStart);
        val_start = request_path.substr(strStart, strEnd - strStart);
        int32_t start = atoi(val_start.c_str());
        
        strStart = request_path.find(key_count + "=") + (key_count + "=").length();
        strEnd = request_path.find("&", strStart);
        val_count = request_path.substr(strStart, strEnd - strStart);
        int32_t count = atoi(val_count.c_str());

        strStart = request_path.find(key_accept + "=") + (key_accept + "=").length();
        strEnd = request_path.find("&", strStart);
        val_accept = request_path.substr(strStart, strEnd - strStart);

        SMPTE_SYNC_LOG << "request_handler::handle_request "
        << " val_dataEssenceCodingUL_ = " << val_dataEssenceCodingUL_
        << " val_start = " << val_start
        << " val_count = " << val_count
        << " val_accept = " << val_accept;
        
        if (populateContentCallback_)
        {
            if (count > maxEditUnitsPerRequest_)
            {
                count = maxEditUnitsPerRequest_;
                /*
                SMPTE_SYNC_LOG << "request_handler::handle_request count = "
                << count
                << " maxEditUnitsPerRequest_ = "
                << maxEditUnitsPerRequest_
                << " using "
                << maxEditUnitsPerRequest_;
                 */
            }
            
            int32_t currentFrame = 0;
            if (currentFrameCallback_)
                currentFrame = currentFrameCallback_();
            
            int32_t framesAhead = 0;
            if (currentFrame < start)
                framesAhead = start - currentFrame;
            
            if (framesAhead < maxEditUnitsAheadOfCurrentEditUnitToRequest_)
            {
                /*
                SMPTE_SYNC_LOG << "request_handler::handle_request framesAhead = "
                << framesAhead
                << " maxEditUnitsAheadOfCurrentEditUnitToRequest_ = "
                << maxEditUnitsAheadOfCurrentEditUnitToRequest_;
                 */
            }
            
            if ((val_accept == SMPTE_SYNC::sPlainText) || (val_accept == SMPTE_SYNC::sEncrypted))
            {
                //SMPTE_SYNC_LOG << "About to call populateContentCallback_ currentFrame = " << currentFrame;

                std::vector<char> content;
                populateContentCallback_(val_dataEssenceCodingUL_,
                                         start,
                                         count,
                                         val_accept,
                                         content);
                // Fill out the reply to be sent to the client.
                rep.status = reply::ok;
                rep.headers.resize(2);
                rep.headers[0].name = "Content-Length";
                rep.headers[0].value = std::to_string(content.size());
                
                // Assign the content
                //
                rep.content = std::string(content.begin(), content.end());
                
                rep.headers[1].name = "Content-Type";
                rep.headers[1].value = "application/smpte336m";
                
                return;
            }
            else
            {
                rep = reply::stock_reply(reply::not_found);
                return;
            }
        }

        rep = reply::stock_reply(reply::not_found);
    }

    bool request_handler::url_decode(const std::string& in, std::string& out)
    {
      out.clear();
      out.reserve(in.size());
      for (std::size_t i = 0; i < in.size(); ++i)
      {
        if (in[i] == '%')
        {
          if (i + 3 <= in.size())
          {
            int value = 0;
            std::istringstream is(in.substr(i + 1, 2));
            if (is >> std::hex >> value)
            {
              out += static_cast<char>(value);
              i += 2;
            }
            else
            {
              return false;
            }
          }
          else
          {
            return false;
          }
        }
        else if (in[i] == '+')
        {
          out += ' ';
        }
        else
        {
          out += in[i];
        }
      }
      return true;
    }

    void request_handler::SetPopulateContentCallback(Callback iCallback)
    {
        populateContentCallback_ = iCallback;
    }

    void request_handler::SetMaxEditUnitsPerRequest(int32_t iUnits)
    {
        maxEditUnitsPerRequest_ = iUnits;
    }
    
    int32_t request_handler::GetMaxEditUnitsPerRequest(void)
    {
        return maxEditUnitsPerRequest_;
    }
    
    void request_handler::SetMaxEditUnitsAheadOfCurrentEditUnit(int32_t iUnits)
    {
        maxEditUnitsAheadOfCurrentEditUnitToRequest_ = iUnits;
    }
    
    int32_t request_handler::GetMaxEditUnitsAheadOfCurrentEditUnit(void)
    {
        return maxEditUnitsAheadOfCurrentEditUnitToRequest_;
    }

    void request_handler::SetMillisecondsPerFrame(int32_t iMilliseconds)
    {
        millisecondsPerFrame_ = iMilliseconds;
    }
    
    int32_t request_handler::GetMillisecondsPerFrame(void)
    {
        return millisecondsPerFrame_;
    }

    void request_handler::SetCurrentFrameCallback(SMPTE_SYNC::CurrentFrameCallback iCallback)
    {
        currentFrameCallback_ = iCallback;
    }

} // namespace server
} // namespace http