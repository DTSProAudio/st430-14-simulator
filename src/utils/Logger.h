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

#ifndef __UTILS_LOGGER__
#define __UTILS_LOGGER__

#include <string>
#include <iostream>

#define USE_BOOST_LOGGER

#ifdef USE_BOOST_LOGGER
#include "boost/log/trivial.hpp"

namespace logging = boost::log;
#endif

#define mda_logger_is_enabled true

/**
 *
 * @brief SMPTE_SYNC_LOG is a wrapper to std::cout or the boost logger
 *
 */

#ifdef USE_BOOST_LOGGER
#define SMPTE_SYNC_LOG if (!mda_logger_is_enabled) ; else BOOST_LOG_TRIVIAL(debug)
#define SMPTE_SYNC_LOG_LEVEL(level) if (!mda_logger_is_enabled) ; else BOOST_LOG_TRIVIAL(level)
#else
#define SMPTE_SYNC_LOG if (!mda_logger_is_enabled) ; else std::cout
#define SMPTE_SYNC_LOG_LEVEL(level) if (!mda_logger_is_enabled) ; else std::cout
#endif

namespace SMPTE_SYNC {

    /**
     *
     * @brief Called at the beginning of system initialization to setup the logger.
     * This should be called before any calls to SMPTE_SYNC_LOG
     *
     */
    void Init_Logger();
}

#endif /* defined(__UTILS_LOGGER__) */

