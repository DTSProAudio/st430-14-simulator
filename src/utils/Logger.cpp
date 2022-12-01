/*======================================================================*
    Copyright (c) 2015-2022 DTS, Inc. and its affiliates.

    Redistribution and use in source and binary forms, with or without modification,
    are permitted provided that the following conditions are met:

    1. Redistributions of source code must retain the above copyright notice, this
    list of conditions and the following disclaimer.

    2. Redistributions in binary form must reproduce the above copyright notice,
    this list of conditions and the following disclaimer in the documentation
    and/or other materials provided with the distribution.

    3. Neither the name of the copyright holder nor the names of its contributors
    may be used to endorse or promote products derived from this software without
    specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
    ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
    ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
    ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *======================================================================*/

#include "Logger.h"
#include <errno.h>
#include <stdio.h>
#include <cstdarg>
#include <iostream>

#ifdef USE_BOOST_LOGGER

//#define BOOST_LOG_DYN_LINK 1

#include <fstream>
#include <iomanip>

#include "boost/smart_ptr/shared_ptr.hpp"
#include "boost/smart_ptr/make_shared_object.hpp"
#include "boost/log/sinks.hpp"
#include "boost/log/core.hpp"
#include "boost/log/trivial.hpp"
#include "boost/log/expressions.hpp"
#include "boost/log/sinks/sync_frontend.hpp"
#include "boost/log/sinks/text_ostream_backend.hpp"
#include "boost/log/sources/severity_logger.hpp"
#include "boost/log/sources/record_ostream.hpp"
#include "boost/log/utility/setup/common_attributes.hpp"
#include "boost/log/utility/setup/console.hpp"
#include "boost/log/support/date_time.hpp"

namespace src = boost::log::sources;
namespace expr = boost::log::expressions;
namespace sinks = boost::log::sinks;
namespace keywords = boost::log::keywords;

#endif
static bool loggerInited = false;

namespace SMPTE_SYNC
{
    void Init_Logger(void)
    {
#ifdef USE_BOOST_LOGGER
        if (loggerInited)
        {
            return;
        }
        
        loggerInited = true;
        
        typedef sinks::asynchronous_sink< sinks::text_ostream_backend > text_sink;
        boost::shared_ptr< text_sink > sink = boost::make_shared< text_sink >();
        
        //sink->locked_backend()->add_stream(boost::make_shared< std::ofstream >("sample.log"));
        
        logging::add_console_log(std::cout);
        
        sink->set_formatter
        (
         expr::stream
         // line id will be written in hex, 8-digits, zero-filled
         //<< std::hex << std::setw(8) << std::setfill('0') << expr::attr< unsigned int >("LineID")
         //<< ": <" << logging::trivial::severity
         //<< "> " << expr::smessage
         << expr::smessage
         );
        
        logging::core::get()->add_sink(sink);
        
        logging::add_common_attributes();

        boost::log::core::get()->set_filter (
                                             boost::log::trivial::severity >= boost::log::trivial::trace
                                             );
        /*
         using namespace logging::trivial;
         src::severity_logger< severity_level > lg;
         
         BOOST_LOG_SEV(lg, trace) << "A trace severity message";
         BOOST_LOG_SEV(lg, debug) << "A debug severity message";
         BOOST_LOG_SEV(lg, info) << "An informational severity message";
         BOOST_LOG_SEV(lg, warning) << "A warning severity message";
         BOOST_LOG_SEV(lg, error) << "An error severity message";
         BOOST_LOG_SEV(lg, fatal) << "A fatal severity message";
         */
#endif
    }

    
}  // namespace COMMMGR
