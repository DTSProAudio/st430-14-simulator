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


#ifndef SS_CLIENT_H
#define SS_CLIENT_H

#include <iostream>
#include <istream>
#include <ostream>
#include <string>

#include "boost/asio.hpp"
#include "boost/bind.hpp"
#include "boost/thread/thread.hpp"
#include "boost/atomic.hpp"

#include "DataTypes.h"

#include "SS_State.h"

using boost::asio::ip::tcp;

namespace SMPTE_SYNC
{
    class AuxDataMgr;
    
    /**
     *
     * @brief SS_Client class implements the SS or sync sample client.
     * This class uses boost::asio to implement the HTTP client for reading the aux data items from the server.
     *
     * Implements for the client of aux server as defined by
     * SMPTE ST 430-10:2010 D-Cinema Operations — Auxiliary Content Synchronization Protocol
     *
     */

    class SS_Client : public SS_State
    {
    public:

        /**
         *
         * Constructor
         *
         * @param io_service \link boost::asio::io_service \endlink is the io service running communication
         * @param iAuxDataMgr is AuxDataMgr that is used to queue received aux data items in the form of a AuxDataBlock pointer
         * @param iEditUnitsPerRequest is the number of edit units that can been requested in each GET request
         * @param iEditUnitsAheadOfCurrentEditUnitToRequest is the number of edit units ahead of the current edit unit that can been requested in each GET request
         * @param iEditUnitsAheadOfCurrentEditUnitToInitiateRequest is the number of edit units or time ahead of the current edit unit received from the SE_Server before a new request can be initiated
         * @param iMillisecondsPerFrame is the number of millisconds in a frame
         * @param iCodingUL is the requested coding UL
         * @param iEncryptionType is the requested encryption type
         * @param iCallback is the callback to get the current frame of the SE_Client
         *
         */
        SS_Client(boost::asio::io_service& io_service
                  , AuxDataMgr *iAuxDataMgr
                  , int32_t iEditUnitsPerRequest
                  , int32_t iEditUnitsAheadOfCurrentEditUnitToRequest
                  , int32_t iEditUnitsAheadOfCurrentEditUnitToInitiateRequest
                  , int32_t iMillisecondsPerFrame
                  , const std::string& iCodingUL
                  , const std::string& iEncryptionType
                  , CurrentFrameCallback iCallback);

        /// Destructor
        virtual ~SS_Client();

        /**
         *
         * Sets the current server and port of the SE_Server.
         * Expected to be in the form of http://127.0.0.1:21234/
         * Should only be called once the DCS_Client determines the proper address of the SE_Server.
         * Starts the fetchAuxDataItemThread_ for fetching data from the SS_Server
         *
         * @param iURL path containing a valid address to a server with a port
         *
         */
        void SetServerAndPort(const std::string& iURL);

        /// Sets the path to the DCS_Server
        void SetServer(const std::string& iServer);

        /// Gets the current path to the DCS_Server
        const std::string& GetServer(void);

        /// Sets the port of the DCS_Server
        void SetPort(const std::string& iPort);

        /// Gets the port of the DCS_Server
        const std::string& GetPort(void);

        /// Sets the number of edit units per request. Calls BuildPath to update data to be next requested.
        void SetEditUnitsPerRequest(int32_t iUnits);

        /// Gets the number of edit units per request
        int32_t GetEditUnitsPerRequest(void);
        
        /// Sets the number of edit units ahead of the current edit unit to request. Calls BuildPath to update data to be next requested.
        void SetEditUnitsAheadOfCurrentEditUnitToRequest(int32_t iUnits);

        /// Gets the number of edit units ahead of the current edit unit to request
        int32_t GetEditUnitsAheadOfCurrentEditUnitToRequest(void);

        /// Sets the coding UL to use for the next requests. Calls BuildPath to update data to be next requested.
        void SetCodingUL(const std::string &iCodingUL);

        /// Sets the coding UL to use for the next requests
        const std::string& GetCodingUL(void);

        /// Sets the encryption type to use for the next requests. Calls BuildPath to update data to be next requested.
        void SetEncryptionType(const std::string &iEncryptionType);

        /// Sets the encryption type to use for the next requests
        const std::string& GetEncryptionType(void);

        /// Sets the number of edit units ahead of the current edit unit to initiate to request.
        void SetEditUnitsAheadOfCurrentEditUnitToInitiateRequest(int32_t iUnits);

        /// Gets the number of edit units ahead of the current edit unit to initiate to request.
        int32_t GetEditUnitsAheadOfCurrentEditUnitToInitiateRequest(void);

        /// Sets number of milliseconds per frame
        void SetMillisecondsPerFrame(int32_t iMilliseconds);

        /**
         *
         * Sets number of milliseconds per frame by providing a numberator and denominator for the frame rate
         *
         * @param iNumerator is the numerator of the frame rate
         * @param iDenominator is the denominator of the frame rate
         *
         */
        void SetMilliscondsPerFrameWithFrameRate(int32_t iNumerator, int32_t iDenominator);

        /// Gets number of milliseconds per frame
        int32_t GetMillisecondsPerFrame(void);

    private:

        /**
         *
         * Sets up the HTTP GET request for an asynchronous call
         * Sets the state of the SE_Client to eState_Buffering
         *
         * @param iPath is URL being requested
         *
         */
        void GET(const std::string& iPath);
        
        /**
         *
         * Called once the endpoint has been resolved by the resolver_
         * Initiates the boost::asio::async_connect
         *
         * @param err is from the async_resolve if there is any, the error is logged and HandleError is called
         * @param endpoint_iterator is the resolved endpoint
         *
         */
        void handle_resolve(const boost::system::error_code& err,
                            tcp::resolver::iterator endpoint_iterator);

        /**
         *
         * Called once the boost::asio::async_connect completes
         * Initiates the boost::asio::async_connect
         *
         * @param err is from the async_connect if there is any, the error is logged and HandleError is called
         *
         */
        void handle_connect(const boost::system::error_code& err);
        
        /**
         *
         * Called once the boost::asio::handle_connect completes
         * Initiates the boost::asio::async_read_until
         *
         * @param err is from the async_write if there is any, the error is logged and HandleError is called
         *
         */
        void handle_write_request(const boost::system::error_code& err);
        
        /**
         *
         * Called once the boost::asio::handle_write_request completes
         * Initiates the boost::asio::async_read_until
         * Reads the response status line.
         *
         * @param err is from the async_write if there is any, the error is logged and HandleError is called
         *
         */
        void handle_read_status_line(const boost::system::error_code& err);

        /**
         *
         * Called once the boost::asio::handle_write_request completes
         * Initiates the boost::asio::async_read_until
         * Reads the response headers
         *
         * @param err is from the async_read_until if there is any, the error is logged and HandleError is called
         *
         */
        void handle_read_headers(const boost::system::error_code& err);
        
        /**
         *
         * Called once the handle_read_headers completes
         * Initiates the boost::asio::async_read
         * Reads the content of the response
         *
         * @param err is from the async_read_until if there is any, the error is logged and HandleError is called
         *
         */
        void handle_read_content(const boost::system::error_code& err);

        /**
         *
         * Called whenever there is an error from one of the boost::asio calls.
         * Sets the SE_Client state to eState_Disconnected
         * Decrements the startEditUnit_ by the last number of requested edit units (editUnitsPerRequest_)
         *
         */
        void HandleError(void);

        /**
         *
         * Runs on the fetchAuxDataItemThread_ to request aux data items from the SS_Server
         * This thread runs at some point in time in the future as computed by the number of frames
         * last requested and the number of frames editUnitsAheadOfCurrentEditUnitToInitiateRequest_
         * Sets the state of the SS_Client to eState_Buffered when enough frames have been received
         *
         */
        void RequestAuxDataItem(void);

        /**
         *
         * Creates the path to be requested by the next GET request.
         * The path contains teh base URL and any additional parameters such as 
         * the codingUL, startEditUnit_, editUnitsPerRequest_, and encryptionType_
         *
         */
        std::string BuildPath(void);

        /// Boost endpoint resolver
        tcp::resolver resolver_;
        
        /// Boost TCP/IP socket used for the GET request
        tcp::socket socket_;

        /// Boost buffer for storing the request. That is the header and data for the GET request
        boost::asio::streambuf request_;
        
        /// Boost buffer for storing the GET request response data
        boost::asio::streambuf response_;
        
        /// This is used to store the whole response accumulated from mutliple read requests
        std::string responsePayload_;
        
        /// The server address
        std::string server_;
        
        /// The port the server is communicating on
        std::string port_;
        
        /// The mutex to protect a set of data used to build the path when SS_Client::BuildPath is called
        boost::mutex    buildPathMutex_;

        /// Coding UL used for each request. Protected as part of the group of items used in SS_Client::BuildPath. Guarded by buildPathMutex_.
        std::string     codingUL_;

        /// The start edit used for each request. Protected as part of the group of items used in SS_Client::BuildPath. Guarded by buildPathMutex_.
        int32_t         startEditUnit_;

        /// Flag to track if a get request has been made but not completed yet. Used to not initiate new get requests until the current one has completed.
        bool            getInProgress_;
        
        /// The edit units per request used for each request. Protected as part of the group of items used in SS_Client::BuildPath. Guarded by buildPathMutex_.
        int32_t         editUnitsPerRequest_;

        /// The encryption type request used for each request. Protected as part of the group of items used in SS_Client::BuildPath. Guarded by buildPathMutex_.
        std::string     encryptionType_;

        /**
         *
         * This is part of two values that determine when the next GET request should be made.
         * This value is the number of edit unit ahead of the current edit unit to request.
         * That is, how many edit units ahead of the current edit unit being received by the SE_Client
         *
         */
        int32_t         editUnitsAheadOfCurrentEditUnitToRequest_;

        /**
         *
         * This is part of two values that determine when the next GET request should be made.
         * This value is the number of edit unite ahead of the current edit unit when the next GET request should be initiated.
         * That is, how many edit units ahead of the current edit unit being received by the SE_Client
         *
         */
        int32_t         editUnitsAheadOfCurrentEditUnitToInitiateRequest_;

        /**
         *
         * The AuxDataMgr stores the received aux data items in a queue.
         * This is shared between the SS_Client (the producer) and the Client_Validator (the consumer).
         *
         */
        AuxDataMgr      *auxDataMgr_;

        /// The thread for calling RequestAuxDataItem
        boost::thread   fetchAuxDataItemThread_;
        
        /**
         *
         * Flag to keep calling RequestAuxDataItem as long as the thread is running. Set to false in the SS_Client destructure so the thread can complete
         * (calling join on the thread) before exiting the destructor
         *
         */
        boost::atomic<bool>            keepRequestingAuxDataItem_;

        /// Pauses calls to RequestAuxDataItem when there is enough data already buffered
        bool            pauseRequestAuxDataItem_;
        
        /// Mutex used to trigger next update for RequestAuxDataItem
        boost::mutex    runAuxDataItemMutex_;

        /// Conditional variable used to trigger next update for RequestAuxDataItem. Used in combination with the runAuxDataItemMutex_
        boost::condition_variable runRequestAuxDataItem_;

        /// Callback for requesting the current frame from the SE_Client
        CurrentFrameCallback currentFrameCallback_;
        
        /// Number of milliseconds per frame based on the current frame rate. Used in determing how long to sleep the fetchAuxDataItemThread_ thread
        int32_t         millisecondsPerFrame_;
    };

}  // namespace SMPTE_SYNC

#endif // SS_CLIENT_H
