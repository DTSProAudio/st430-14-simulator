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

#ifndef SS_SERVER_H
#define SS_SERVER_H

#include <string>

#include "server.hpp"
#include "request_handler.hpp"

namespace SMPTE_SYNC
{
    /**
     *
     * @brief SS_Server class implements the HTTP server to serve aux data items to the SE_Client
     * The SS_Server is based on a boost HTTP server found here...
     * http://www.boost.org/doc/libs/1_55_0/doc/html/boost_asio/examples/cpp11_examples.html
     *
     * Implements for the client of a sync emitter server as defined by
     * SMPTE ST 430-10:2010 D-Cinema Operations — Auxiliary Content Synchronization Protocol
     *
     */
    class SS_Server : public http::server::server
    {
    public:
        
        /**
         *
         * Constructor
         *
         * @param io_service \link boost::asio::io_service \endlink is the io service running communication
         * @param address is address of the HTTP server
         * @param port is port of the HTTP server
         *
         */
        explicit SS_Server(boost::asio::io_service& io_service,
                        const std::string& address,
                        const std::string& port);

        /// Destructor
        virtual ~SS_Server(void);
        
    private:
    };
}  // namespace SMPTE_SYNC

#endif // SS_SERVER_H
