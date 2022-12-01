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

#ifndef DCS_SERVER_H
#define DCS_SERVER_H

#include <algorithm>
#include <cstdlib>
#include <deque>
#include <iostream>
#include <list>
#include <set>
#include <string>

#include "boost/bind.hpp"
#include "boost/shared_ptr.hpp"
#include "boost/enable_shared_from_this.hpp"
#include "boost/asio.hpp"
#include "boost/thread/thread.hpp"
#include "DCS_Message.h"
#include "DCS_State.h"

using boost::asio::ip::tcp;

namespace SMPTE_SYNC
{

    class DCS_Server;

    /**
     * @brief DCS_Message_Queue is a typedef for a queue to store DCS_Message_Ptr which is a boost::shared_ptr to a DCS_Message.
     * The queue stores messages before they are sent to the client.
     *
     */

    typedef std::deque<DCS_Message_Ptr> DCS_Message_Queue;

    /**
     * @brief DCS_Session is the class that represents and manages an individual connection to a client.
     * In the current implementation of DCS_Session and DCS_Server, only a single connection to a single client is allowed. 
     * Further development will be needed fully support the DCS_Server and DCS_Session to have multiple connections to DCS_Clients.
     *
     */

    class DCS_Session : public boost::enable_shared_from_this<DCS_Session>, public DCS_State
    {
    public:

        /**
         *
         * Constructor
         *
         * @param io_service reference to the shared boost::asio::io_service
         * @param iMessageHeaderSize is the size of the MessageHeader. Used to allocate memory to store the serialized data for the header for writing
         * @param iURL is the path to the SS_Server. This is used in the DCS_Message_SetRPLLocationRequest
         * @param iIsReadyCallback is a callback that is used to set the state of the SS_Server to a ready state when a DCS_Message_GetStatusResponse message with the ResponseKey of eResponseKey_RRPSuccessful is received
         *
         */
        DCS_Session(boost::asio::io_service& io_service,
                    int32_t iMessageHeaderSize,
                    const std::string &iURL,
                    int32_t iPlayoutID,
                    IsReadyCallback iIsReadyCallback,
                    SetPlayoutIDCallback iSetPlayoutIDCallback);

        /// Destructor
        virtual ~DCS_Session();
        
        /// Gets a reference to the socket used for the connection of DCS_Session
        tcp::socket& socket();

        /**
         *
         * Start initiaites the first boost::asio::async_read to read incoming messages from the DCS_Client 
         * and sends a DCS_Message_AnnounceRequest to the DCS_Client
         *
         */
        void Start();

        /**
         *
         * Sends a DCS_Message to the DCS_Client. The DCS_Message may be enqued for sending if an is a pending message being sent
         *
         */
        void Send(DCS_Message_Ptr msg);

        /**
         *
         * Creates a new DCS_Message_AnnounceRequest message, populates it, and sends it.
         *
         */
        void AnnounceRequest(void);

        /**
         *
         * Creates a new DCS_Message_GetStatusRequest message, populates it, and sends it.
         *
         */
        void GetStatusRequest(void);
        
        /**
         *
         * Execute is called when a new DCS_Message (responses) are received. 
         * The message type is indentified via a dynamic_cast based on the kind1_ and kind2_ and the message is handled appropriately
         *
         * @param iMsg is a properly constructed and populated DCS_Message object
         * @return true/false - true if the DCS_Message was handled, false if the DCS_Message was not handled or an error occurred
         *
         */
        bool Execute(DCS_Message *iMsg);
        
        /**
         *
         * Sets the DCS_Session state to eState_Disconnected and closes the socket
         *
         */
        void DoClose(void);

    private:

        /**
         *
         * HandleReadHeader is called when the full amount of the header is received by the socket.
         * Reads the header data to determine the size of the DCS_Message payload and initiates a boost::asio::async_read to read the payload
         *
         */
        void HandleReadHeader(const boost::system::error_code& error);

        /**
         *
         * HandleReadBody is called when the full amount of the payload is received by the socket.
         * Reads the payload data then creates a new DCS_Message object by calling MessageFactory::CreateDCSMessage
         * Once the DCS_Message is created, the message is executed by calling DSC_Session::Execute.
         * After the DCS_Message is executed, the message is deleted, then boost::asio::async_read with DCS_Session::HandleReadHeader to read the next DCS_Message
         *
         * @param error is a error if there was one during reading the body. If there was an error, the error is logged and DCS_Session::DoClose is called to cleanup the socket
         *
         */
        void HandleReadBody(const boost::system::error_code& error);

        /**
         *
         * HandleWrite writes a message that was enqued on the write_msgs_.
         * If the message payload for this message was larger than the preallocated writeBuffer_, the current writeBuffer_ is deleted and a new larger writeBuffer_ is allocated
         *
         * @param error is a error if there was one during the previous call. If there was an error, the error is logged and DCS_Session::DoClose is called to cleanup the socket
         *
         */
        void HandleWrite(const boost::system::error_code& error);
        
        /// Socket for the connection to the DCS_Client
        tcp::socket socket_;

        /// Size of the header
        int32_t             readHeaderBufferSize_;

        /// Preallocated buffer to read the DCS_Message header into before it is constructed
        uint8_t             *readHeaderBuffer_;

        /// Size of the payload for current message being read. Size determined by the length of the payload that is read from the DCS_Message header
        int32_t             readPayloadBufferSize_;

        /// Preallocated buffer to read the DCS_Message payload into before it is constructed
        uint8_t             *readPayloadBuffer_;

        /// Size of the DCS_Message to be written. This represents the full size of the serialized DCS_Message for both the header and payload
        int32_t             writeBufferSize_;

        /// Buffer for the DCS_Message to be written. This represents the full message of the serialized DCS_Message for both the header and payload
        uint8_t             *writeBuffer_;

        /// Pending message queue to be sent. If multiple messages are sent in quick succession, they will be enqueued and sent in the order they were enqueued.
        DCS_Message_Queue   write_msgs_;
        
        /// The URI of the SS_Server
        std::string         dcsResourceURL_;

        /// The payout of the DCS_Server
        uint32_t            playoutID_;

        /// Callback to the SS_Server to set the ready state when the DCS_Message_GetStatusResponse ResponseKey is eResponseKey_RRPSuccessful
        IsReadyCallback     isReady_;
        
        /// Callback to the SS_Server to set the ready state when the DCS_Message_GetStatusResponse ResponseKey is eResponseKey_RRPSuccessful
        SetPlayoutIDCallback     setPlayoutID_;
    };

    /**
     *
     * @brief DCS_Session_Ptr is a typedef for the boost::shared_ptr to a DCS_Session.
     * The DCS_Server creates the DCS_Session and stores it as a boost::shared_ptr to be deleted when the connection is closed
     *
     */

    typedef boost::shared_ptr<DCS_Session> DCS_Session_Ptr;

    /**
     *
     * @brief DCS_Server is class that manages connections to DCS_Client objects
     *
     */

    class DCS_Server : public DCS_State
    {
    public:

        /**
         *
         * Constructor
         *
         * @param io_service is a reference to the shared boost::asio::io_service
         * @param endpoint is the endpoint of which to listen for incoming connections on. The endpoint represents the IP and port of which to listen on.
         * @param iURL is the path to the SS_Server. This is used in the DCS_Message_SetRPLLocationRequest
         * @param iPlayoutID is playout id of the DCS_Server
         * @param iMessageHeaderSize is the size of the MessageHeader. Used to allocate memory to store the serialized data for the header for writing
         * @param iIsReadyCallback is a callback that is used to set the state of the SS_Server to a ready state when a DCS_Message_GetStatusResponse message with the ResponseKey of eResponseKey_RRPSuccessful is received
         *
         */
        DCS_Server(boost::asio::io_service& io_service
                   , const tcp::endpoint& endpoint
                   , const std::string &iURL
                   , uint32_t iPlayoutID
                   , int32_t iMessageHeaderSize
                   , IsReadyCallback iIsReadyCallback
                   , SetPlayoutIDCallback iSetPlayoutIDCallback);

        /// Destructor
        virtual ~DCS_Server();
        
        /// TODO: Is this used?
        /// Sends a new DCS_Message_AnnounceResponse
        void NotifyRPLChange(void);

        /**
         *
         * Returns the state of the DCS_Server (really the state of the DCS_Session). Currently, only a single connection to DCS_Client is supported.
         *
         * @return vallue of EState
         *
         */
        virtual EState GetState(void);

    private:

        /**
         *
         * Called to establish a new connection (DCS_Session) to a DCS_Client
         *
         */
        void StartAccept();
        
        /**
         *
         * Called when a DCS_Client initiates a connection to the DCS_Server
         * DCS_Server::StartAccept is called after the incoming connection is accepted to start listing for another incoming DCS_Client connection
         *
         * @param session is the pointer to the DCS_Session object representing the connection to a DCS_Client
         * @param error is error code if there was one. If there is an error DCS_Session::DoClose is called to clean up the connection
         */
        void HandleAccept(DCS_Session_Ptr session,
                          const boost::system::error_code& error);
        
        /// A reference to the shared boost::asio::io_service
        boost::asio::io_service&    io_service_;

        /// The acceptor listens for incoming connections and accepts them
        tcp::acceptor               acceptor_;

        /// This is the size of the DCS_Message header
        int32_t                     messageHeaderSize_;

        /// This is the list of currently connected DCS_Clients. Note we only support a single client at this time.
        std::set<DCS_Session_Ptr>   clients_;

        /// This is the playout id of the DCS server
        uint32_t                    playoutID_;

        /// This is the URI of the SS_Server. It is returned to the DCS_Client with the DCS_Message_SetRPLLocationRequest message
        std::string                 dcsResourceURL_;

        /// A callback to notify the SS_Server the client is ready for processing.
        IsReadyCallback             isReady_;

        /// Callback to the SS_Server to set the ready state when the DCS_Message_GetStatusResponse ResponseKey is eResponseKey_RRPSuccessful
        SetPlayoutIDCallback     setPlayoutID_;
    };

}  // namespace SMPTE_SYNC

#endif // DCS_SERVER_H
