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

#ifndef DCS_CLIENT_H
#define DCS_CLIENT_H

#include <cstdlib>
#include <deque>
#include <iostream>

#include "boost/bind.hpp"
#include "boost/asio.hpp"

#include "DCS_Message.h"
#include "DCS_State.h"

using boost::asio::ip::tcp;

namespace SMPTE_SYNC
{

    class Client_State;
    typedef std::deque<DCS_Message_Ptr> DCS_Message_Queue;

    /**
     * @brief DCS_Client class implements the DCS client that communicates to the DCS_Server.
     *
     * Implements for the client of a DCS server as defined by
     * SMPTE ST 430-10:2010 D-Cinema Operations — Auxiliary Content Synchronization Protocol
     *
     * Uses the Boost::ASIO libraries to implement an asynchronous TCP/IP connection between the 
     * DCS client and server.
     *
     */

    class DCS_Client : public DCS_State
    {
    public:

        /**
         * Constructor
         *
         * @param io_service \link boost::asio::io_service \endlink is the io service running communication
         * @param endpoint_iterator \link tcp::resolver::iterator \endlink is the endpoint attempting to 
         * establish a connection with
         * @param iMessageHeaderSize is size in bytes of the DCS_Message header
         * @param iClientState \link Client_State \endlink is a pointer to the Client_State object that 
         * manages and validates the states of the client implementation of SMPTE ST 430-14:20YY
         * @param iCallback \link SetRPLLocationCallback \endlink is the callback to provide location of the Aux Data server
         *
         */
        DCS_Client(boost::asio::io_service& io_service,
                   tcp::resolver::iterator endpoint_iterator,
                   int32_t iMessageHeaderSize,
                   Client_State *iClientState,
                   SetRPLLocationCallback iCallback);

        /// Destructor
        virtual ~DCS_Client();

        /// Sends a DCS_Message
        void Send(DCS_Message_Ptr msg);

        /// Closes the TCP/IP connection to the DCS_Server
        void Close();

    private:

        /**
         * Handles connections
         *
         * @param error \link boost::system::error_code \endlink is the error code from the Boost ASIO library
         *
         */
        void HandleConnect(const boost::system::error_code& error);

        /**
         * Reads the header information for a DCS_Message. If the pre-allocated memory for the payload is
         * not large enough to contain the payload, it deallocates and allocates additional memory.
         *
         * @param error \link boost::system::error_code \endlink is the error code from the Boost ASIO library
         *
         */
        void HandleReadHeader(const boost::system::error_code& error);

        /**
         * Reads the payload for a DCS_Message. Once all of the payload has been read, it creates a DCS_Message
         * by calling the MessageFactory::CreateDCSMessage.
         * If a valid DCS_Message is created, the messages seralizes the payload to popuplate the data.
         * The message is then executed and deleted.
         * Once the message is executed a new boost::asio::async_read is called.
         *
         * @param error \link boost::system::error_code \endlink is the error code from the Boost ASIO library
         *
         */
        void HandleReadBody(const boost::system::error_code& error);

        /**
         * 
         * Implements boost::asio::async_write by equeuing a DCS_Message for sending.
         * If the pre-allocated memory for the payload is not large enough to contain the payload,
         * it deallocates and allocates additional memory.
         * Serializes DCS_Message into a buffer for writing.
         *
         * @param error \link boost::system::error_code \endlink is the error code from the Boost ASIO library
         *
         */
        void DoWrite(DCS_Message_Ptr msg);

        /**
         *
         * Implements boost::asio::async_write by equeuing a DCS_Message for sending.
         * If the pre-allocated memory for the payload is not large enough to contain the payload,
         * it deallocates and allocates additional memory.
         * Serializes DCS_Message into a buffer for writing.
         * Closes the TCP/IP connection if an error was passed in.
         *
         * @param error \link boost::system::error_code \endlink is the error code from the Boost ASIO library
         *
         */
        void HandleWrite(const boost::system::error_code& error);

        /**
         *
         * Closes the TCP/IP connection and sets the state to \link eState_Disconnected \endlink
         *
         */
        void DoClose(void);

        /**
         *
         * Executes a DCS_Message.
         * Each message requires a response. The reponses is created and sent with appropriate data for the payload.
         *
         * @param iMsg DCS_Message is the message to execute.
         * @return true/false if execution of message was successful or not.
         *
         */
        bool Execute(DCS_Message *iMsg);

        /// Reference to the boost::asio::io_service used for the DCS_Client
        boost::asio::io_service&    io_service_;
        
        /// Socket used for connecting to the DCS_Server
        tcp::socket                 socket_;

        /**
         *
         * State of the client as specified in      
         * SMPTE ST 430-10:2010 D-Cinema Operations — Auxiliary Content Synchronization Protocol
         *
         */
        Client_State                *clientState_;
        
        /// Size of the DCS_Message header for reading messages
        int32_t                     readHeaderBufferSize_;
        
        /// Buffer to store the DCS_Message header bytes reading messages
        uint8_t                     *readHeaderBuffer_;
        
        /// Size of the largest DCS_Message payload. Grows to accomadate the largest payload.
        int32_t                     readPayloadBufferSize_;

        /// Buffer to store the DCS_Message payload bytes
        uint8_t                     *readPayloadBuffer_;
        
        /// Size of the largest DCS_Message payload for writing messages. Grows to accomadate the largest payload.
        int32_t                     writeBufferSize_;

        /// Buffer to store the DCS_Message payload bytes
        uint8_t                     *writeBuffer_;

        /// Queue for storing messages if there are multiple messages to send without blocking the sender
        DCS_Message_Queue           write_msgs_;
        
        /// Callback installed by the client to provide location of the Aux Data server
        SetRPLLocationCallback      setRPLLocationCallback_;
    };

}  // namespace SMPTE_SYNC

#endif // DCS_CLIENT_H
