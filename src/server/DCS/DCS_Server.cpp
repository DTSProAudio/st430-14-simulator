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
#include "boost/random.hpp"
#include "boost/generator_iterator.hpp"
#include "boost/date_time/posix_time/posix_time.hpp"
#include "boost/date_time/posix_time/posix_time_types.hpp"

#include "DCS_Server.h"
#include "DCS_Message.h"
#include "MessageFactory.h"
#include "Logger.h"

using boost::asio::ip::tcp;

namespace SMPTE_SYNC
{

    DCS_Session::DCS_Session(boost::asio::io_service& io_service,
                             int32_t iMessageHeaderSize,
                             const std::string &iURL,
                             int32_t iPlayoutID,
                             IsReadyCallback iIsReadyCallback,
                             SetPlayoutIDCallback iSetPlayoutIDCallback)
                : socket_(io_service)
                , dcsResourceURL_(iURL)
                , playoutID_(iPlayoutID)
                , isReady_(iIsReadyCallback)
                , setPlayoutID_(iSetPlayoutIDCallback)
    {
        readHeaderBufferSize_ = iMessageHeaderSize;
        readHeaderBuffer_ = new uint8_t[readHeaderBufferSize_];
        
        // Start off with a payload the same size as the header
        //
        readPayloadBufferSize_ = iMessageHeaderSize;
        readPayloadBuffer_ = new uint8_t[readPayloadBufferSize_];

        // Start off with a payload the same size as the header
        //
        writeBufferSize_ = iMessageHeaderSize * 2;
        writeBuffer_ = new uint8_t[writeBufferSize_];
    }

    DCS_Session::~DCS_Session()
    {
        delete [] readHeaderBuffer_;
        readHeaderBuffer_ = nullptr;

        delete [] readPayloadBuffer_;
        readPayloadBuffer_ = nullptr;

        delete [] writeBuffer_;
        writeBuffer_ = nullptr;
    }

    tcp::socket& DCS_Session::socket()
    {
        return socket_;
    }

    void DCS_Session::Start()
    {
        memset(readHeaderBuffer_, 0x0, readHeaderBufferSize_);

        boost::asio::async_read(socket_,
                                boost::asio::buffer(readHeaderBuffer_, readHeaderBufferSize_),
                                boost::bind(&DCS_Session::HandleReadHeader, shared_from_this(),
                                            boost::asio::placeholders::error));

        this->AnnounceRequest();
    }

    void DCS_Session::Send(DCS_Message_Ptr msg)
    {
        bool write_in_progress = !write_msgs_.empty();
        
        write_msgs_.push_back(msg);
        
        if (!write_in_progress)
        {
            int32_t msgSize = write_msgs_.front()->GetMessageSize();
            
            if (msgSize > writeBufferSize_)
            {
                writeBufferSize_ = msgSize;
                delete [] writeBuffer_;
                
                writeBuffer_ = new uint8_t[writeBufferSize_];
            }
            
            memset(writeBuffer_, 0x0, writeBufferSize_);

            uint8_t *tmp = writeBuffer_;
            write_msgs_.front()->WriteHeader(&tmp);
            write_msgs_.front()->WritePayload(&tmp);

            boost::asio::async_write(socket_,
                                     boost::asio::buffer(writeBuffer_, msgSize),
                                     boost::bind(&DCS_Session::HandleWrite, shared_from_this(),
                                                 boost::asio::placeholders::error));
        }
    }

    void DCS_Session::HandleReadHeader(const boost::system::error_code& error)
    {
        if (!error)
        {
            EState currentState = this->GetState();
            if (currentState == eState_Disconnected)
            {
                this->SetState(eState_Connected);
            }
            
            uint8_t *tmp = readHeaderBuffer_;
            MessageHeader msgHeader;
            msgHeader.Read(&tmp);

            if (readPayloadBufferSize_ < msgHeader.length_)
            {
                delete [] readPayloadBuffer_;
                
                readPayloadBufferSize_ = msgHeader.length_;
                readPayloadBuffer_ = new uint8_t[readPayloadBufferSize_];
            }
            
            memset(readPayloadBuffer_, 0x0, readPayloadBufferSize_);
            
            boost::asio::async_read(socket_,
                                    boost::asio::buffer(readPayloadBuffer_, msgHeader.length_),
                                    boost::bind(&DCS_Session::HandleReadBody, shared_from_this(),
                                                boost::asio::placeholders::error));
        }
        else
        {
            this->DoClose();
        }
    }

    void DCS_Session::HandleReadBody(const boost::system::error_code& error)
    {
        if (!error)
        {
            // Read the header (again)
            //
            uint8_t *tmp = readHeaderBuffer_;
            MessageHeader msgHeader;
            msgHeader.Read(&tmp);

            // Determine what command it is and create one
            //
            DCS_Message* msg = MessageFactory::CreateDCSMessage(msgHeader);
            
            if (msg)
            {
                // Serialize the command
                //
                tmp = readPayloadBuffer_;
                msg->ReadPayload(&tmp);

                // Call the handler
                //
                this->Execute(msg);
                
                // Delete the message now that we are all done
                //
                delete msg;
            }

            memset(readHeaderBuffer_, 0x0, readHeaderBufferSize_);

            boost::asio::async_read(socket_,
                                    boost::asio::buffer(readHeaderBuffer_, readHeaderBufferSize_),
                                    boost::bind(&DCS_Session::HandleReadHeader, shared_from_this(),
                                                boost::asio::placeholders::error));
        }
        else
        {
            this->DoClose();
        }
    }

    void DCS_Session::HandleWrite(const boost::system::error_code& error)
    {
        if (!error)
        {
            write_msgs_.pop_front();
            
            if (!write_msgs_.empty())
            {
                int32_t msgSize = write_msgs_.front()->GetMessageSize();
                
                if (msgSize > writeBufferSize_)
                {
                    writeBufferSize_ = msgSize;
                    delete [] writeBuffer_;
                    
                    writeBuffer_ = new uint8_t[writeBufferSize_];
                }

                memset(writeBuffer_, 0x0, writeBufferSize_);

                uint8_t *tmp = writeBuffer_;
                write_msgs_.front()->WriteHeader(&tmp);
                write_msgs_.front()->WritePayload(&tmp);

                boost::asio::async_write(socket_,
                                         boost::asio::buffer(writeBuffer_, msgSize),
                                         boost::bind(&DCS_Session::HandleWrite, shared_from_this(),
                                                    boost::asio::placeholders::error));
            }
        }
        else
        {
            this->DoClose();
        }
    }

    void DCS_Session::AnnounceRequest(void)
    {
        std::time_t now = std::time(0);
        boost::random::mt19937 gen{static_cast<std::uint32_t>(now)};
        int32_t requestID = gen();
        
        time_t timeSinceEpochInSeconds = time(NULL);
        
        SMPTE_SYNC_LOG_LEVEL(trace)  << "DCS_Server::AnnounceRequest: "
        << "currentTime = " << static_cast<int64_t>(timeSinceEpochInSeconds) << " "
        << "requestID = " << requestID;
        
        DCS_Message_AnnounceRequest *request = new DCS_Message_AnnounceRequest();
        request->SetRequestID(requestID);
        request->SetCurrentTime(timeSinceEpochInSeconds);
        request->SetDeviceDescription("DCS_Server");
        
        this->Send(DCS_Message_Ptr(request));
    }

    void DCS_Session::GetStatusRequest(void)
    {
        std::time_t now = std::time(0);
        boost::random::mt19937 gen{static_cast<std::uint32_t>(now)};
        int32_t requestID = gen();
        
        time_t timeSinceEpochInSeconds = time(NULL);
        
        SMPTE_SYNC_LOG_LEVEL(trace)  << "DCS_Server::GetStatusRequest: "
        << "currentTime = " << static_cast<int64_t>(timeSinceEpochInSeconds) << " "
        << "requestID = " << requestID;
        
        DCS_Message_GetStatusRequest *request = new DCS_Message_GetStatusRequest();
        request->SetRequestID(requestID);
        
        this->Send(DCS_Message_Ptr(request));
    }

    bool DCS_Session::Execute(DCS_Message *iMsg)
    {
        SMPTE_SYNC_LOG_LEVEL(trace) << "DCS_Server::Execute";
        
        bool success = true;
        
        if (iMsg->GetKind1() == DCS_Message_AnnounceResponse::kind1_ && iMsg->GetKind2() == DCS_Message_AnnounceResponse::kind2_)
        {
            SMPTE_SYNC_LOG_LEVEL(trace) << "DCS_Server::Execute - DCS_Message_AnnounceResponse";
            
            DCS_Message_AnnounceResponse *response = dynamic_cast<DCS_Message_AnnounceResponse*>(iMsg);
            
            if (response != nullptr)
            {
                DCS_Message_GetNewLeaseRequest *request = new DCS_Message_GetNewLeaseRequest();
                
                std::time_t now = std::time(0);
                boost::random::mt19937 gen{static_cast<std::uint32_t>(now)};
                int32_t requestID = gen();
                
                uint32_t leaseDuration = 10000000;
                
                request->SetLeaseDuration(leaseDuration);
                request->SetRequestID(requestID);
                
                this->Send(DCS_Message_Ptr(request));
            }
            else
                SMPTE_SYNC_LOG << "DCS_Client::Execute - ERROR UNKNOWN COMMAND DCS_Message_AnnounceRequest";
        }
        
        if (iMsg->GetKind1() == DCS_Message_GetNewLeaseResponse::kind1_ && iMsg->GetKind2() == DCS_Message_GetNewLeaseResponse::kind2_)
        {
            SMPTE_SYNC_LOG_LEVEL(trace) << "DCS_Client::Execute - DCS_Message_GetNewLeaseResponse";
            
            DCS_Message_GetNewLeaseResponse *response = dynamic_cast<DCS_Message_GetNewLeaseResponse*>(iMsg);
            
            if (response != nullptr)
            {
                DCS_Message_SetRPLLocationRequest *request = new DCS_Message_SetRPLLocationRequest();
                
                std::time_t now = std::time(0);
                boost::random::mt19937 gen{static_cast<std::uint32_t>(now)};
                int32_t requestID = gen();
                playoutID_ = gen();

                if (setPlayoutID_)
                    setPlayoutID_(playoutID_);
                
                request->SetRequestID(requestID);
                request->SetPlayoutID(playoutID_);
                request->SetResourceURL(dcsResourceURL_);

                this->Send(DCS_Message_Ptr(request));
            }
            else
                SMPTE_SYNC_LOG << "DCS_Client::Execute - ERROR UNKNOWN COMMAND DCS_Message_GetNewLeaseResponse";
        }

        if (iMsg->GetKind1() == DCS_Message_SetRPLLocationResponse::kind1_ && iMsg->GetKind2() == DCS_Message_SetRPLLocationResponse::kind2_)
        {
            SMPTE_SYNC_LOG_LEVEL(trace) << "DCS_Client::Execute - DCS_Message_SetRPLLocationResponse";
            
            DCS_Message_SetRPLLocationResponse *response = dynamic_cast<DCS_Message_SetRPLLocationResponse*>(iMsg);
            
            if (response != nullptr)
            {
                ResponseKey responseKey = response->GetResponseKey();
                uint32_t requestID = response->GetRequestID();
                SMPTE_SYNC_LOG_LEVEL(trace) << "responseKey = " << responseKey;
                SMPTE_SYNC_LOG_LEVEL(trace) << "requestID = " << requestID;
            }
            else
                SMPTE_SYNC_LOG << "DCS_Client::Execute - ERROR UNKNOWN COMMAND DCS_Message_SetRPLLocationResponse";
        }

        if (iMsg->GetKind1() == DCS_Message_GetStatusResponse::kind1_ && iMsg->GetKind2() == DCS_Message_GetStatusResponse::kind2_)
        {
            SMPTE_SYNC_LOG_LEVEL(trace) << "DCS_Client::Execute - DCS_Message_GetStatusResponse";
            
            DCS_Message_GetStatusResponse *response = dynamic_cast<DCS_Message_GetStatusResponse*>(iMsg);
            
            if (response != nullptr)
            {
                bool ready = false;
                if (response->GetResponseKey() == eResponseKey_RRPSuccessful)
                    ready = true;
                
                if (isReady_)
                    isReady_(ready);
            }
            else
                SMPTE_SYNC_LOG << "DCS_Client::Execute - ERROR UNKNOWN COMMAND DCS_Message_GetStatusResponse";
        }

        if (iMsg->GetKind1() == DCS_Message_UpdateTimelineResponse::kind1_ && iMsg->GetKind2() == DCS_Message_UpdateTimelineResponse::kind2_)
        {
            SMPTE_SYNC_LOG_LEVEL(trace) << "DCS_Client::Execute - DCS_Message_UpdateTimelineResponse";
            
            DCS_Message_UpdateTimelineResponse *response = dynamic_cast<DCS_Message_UpdateTimelineResponse*>(iMsg);
            
            if (response != nullptr)
            {
            }
            else
                SMPTE_SYNC_LOG << "DCS_Client::Execute - ERROR UNKNOWN COMMAND DCS_Message_UpdateTimelineResponse";
        }

        if (iMsg->GetKind1() == DCS_Message_SetOutputModeResponse::kind1_ && iMsg->GetKind2() == DCS_Message_SetOutputModeResponse::kind2_)
        {
            SMPTE_SYNC_LOG_LEVEL(trace) << "DCS_Client::Execute - DCS_Message_SetOutputModeResponse";
            
            DCS_Message_SetOutputModeResponse *response = dynamic_cast<DCS_Message_SetOutputModeResponse*>(iMsg);
            
            if (response != nullptr)
            {
            }
            else
                SMPTE_SYNC_LOG << "DCS_Client::Execute - ERROR UNKNOWN COMMAND DCS_Message_SetOutputModeResponse";
        }

        if (iMsg->GetKind1() == DCS_Message_GetLogEventListRequest::kind1_ && iMsg->GetKind2() == DCS_Message_GetLogEventListRequest::kind2_)
        {
            SMPTE_SYNC_LOG_LEVEL(trace) << "DCS_Client::Execute - DCS_Message_GetLogEventListRequest";
            
            DCS_Message_GetLogEventListRequest *response = dynamic_cast<DCS_Message_GetLogEventListRequest*>(iMsg);
            
            if (response != nullptr)
            {
            }
            else
                SMPTE_SYNC_LOG << "DCS_Client::Execute - ERROR UNKNOWN COMMAND DCS_Message_GetLogEventListRequest";
        }

        if (iMsg->GetKind1() == DCS_Message_GetLogEventListResponse::kind1_ && iMsg->GetKind2() == DCS_Message_GetLogEventListResponse::kind2_)
        {
            SMPTE_SYNC_LOG_LEVEL(trace) << "DCS_Client::Execute - DCS_Message_GetLogEventListResponse";
            
            DCS_Message_GetLogEventListResponse *response = dynamic_cast<DCS_Message_GetLogEventListResponse*>(iMsg);
            
            if (response != nullptr)
            {
            }
            else
                SMPTE_SYNC_LOG << "DCS_Client::Execute - ERROR UNKNOWN COMMAND DCS_Message_GetLogEventListResponse";
        }

        if (iMsg->GetKind1() == DCS_Message_GetLogEventRequest::kind1_ && iMsg->GetKind2() == DCS_Message_GetLogEventRequest::kind2_)
        {
            SMPTE_SYNC_LOG_LEVEL(trace) << "DCS_Client::Execute - DCS_Message_GetLogEventRequest";
            
            DCS_Message_GetLogEventRequest *response = dynamic_cast<DCS_Message_GetLogEventRequest*>(iMsg);
            
            if (response != nullptr)
            {
            }
            else
                SMPTE_SYNC_LOG << "DCS_Client::Execute - ERROR UNKNOWN COMMAND DCS_Message_GetLogEventRequest";
        }
        
        if (iMsg->GetKind1() == DCS_Message_GetLogEventResponse::kind1_ && iMsg->GetKind2() == DCS_Message_GetLogEventResponse::kind2_)
        {
            SMPTE_SYNC_LOG_LEVEL(trace) << "DCS_Client::Execute - DCS_Message_GetLogEventResponse";
            
            DCS_Message_GetLogEventResponse *response = dynamic_cast<DCS_Message_GetLogEventResponse*>(iMsg);
            
            if (response != nullptr)
            {
            }
            else
                SMPTE_SYNC_LOG << "DCS_Client::Execute - ERROR UNKNOWN COMMAND DCS_Message_GetLogEventResponse";
        }
        
        /*
         if (iMsg->GetKind1() == 0x02 && iMsg->GetKind2() == 0x00)
         {
         SMPTE_SYNC_LOG << "DCS_Server::Execute - DCS_Message_AnnounceRequest";
         }
         */
        
        return success;
    }

    void DCS_Session::DoClose(void)
    {
        this->SetState(eState_Disconnected);
        socket_.close();
    }

    //----------------------------------------------------------------------

    DCS_Server::DCS_Server(boost::asio::io_service& io_service
                           , const tcp::endpoint& endpoint
                           , const std::string &iURL
                           , uint32_t iPlayoutID
                           , int32_t iMessageHeaderSize
                           , IsReadyCallback iIsReadyCallback
                           , SetPlayoutIDCallback iSetPlayoutIDCallback
                           )
                            :   io_service_(io_service)
                                , acceptor_(io_service, endpoint)
                                , dcsResourceURL_(iURL)
                                , messageHeaderSize_(iMessageHeaderSize)
                                , isReady_(iIsReadyCallback)
                                , setPlayoutID_(iSetPlayoutIDCallback)
                                , playoutID_(iPlayoutID)

    {
        this->StartAccept();
    }

    DCS_Server::~DCS_Server()
    {
        
    }

    void DCS_Server::StartAccept()
    {
        DCS_Session_Ptr new_session(new DCS_Session(io_service_
                                                    , messageHeaderSize_
                                                    , dcsResourceURL_
                                                    , playoutID_
                                                    , isReady_
                                                    , setPlayoutID_
                                                    ));
      
        acceptor_.async_accept(new_session->socket(),
                               boost::bind(&DCS_Server::HandleAccept,
                                        this,
                                        new_session,
                                        boost::asio::placeholders::error));
    }

    void DCS_Server::HandleAccept(DCS_Session_Ptr session,
                                    const boost::system::error_code& error)
    {
        if (!error)
        {
            clients_.insert(session);
            session->Start();
        }
        else
        {
            session->DoClose();
        }

        this->StartAccept();
    }

    void DCS_Server::NotifyRPLChange(void)
    {
        DCS_Message_AnnounceResponse *announ = new DCS_Message_AnnounceResponse();
        announ->SetResponseKey(eResponseKey_RRPSuccessful);

        DCS_Message_Ptr msg(announ);
        
        for (std::set<DCS_Session_Ptr>::iterator iter = clients_.begin(); iter != clients_.end(); iter++)
        {
            (*iter)->Send(msg);
        }
        /*
        DCS_Message msg;
        std::for_each(clients_.begin(), clients_.end(),
                      boost::bind(&DCS_Session::Send, msg, _1));
         */
    }

    DCS_State::EState DCS_Server::GetState(void)
    {
        //SMPTE_SYNC_LOG << "DCS_Server::GetState";

        EState state = eState_Disconnected;
        
        /// TODO: Only supporting a single client at the moment
        for (std::set<DCS_Session_Ptr>::iterator iter = clients_.begin(); iter != clients_.end(); iter++)
        {
            state = (*iter)->GetState();
            break;
        }
        
        return state;
    }

}  // namespace SMPTE_SYNC
