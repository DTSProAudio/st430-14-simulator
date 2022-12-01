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

#include "DCS_Client.h"

#include <string>

#include "MessageFactory.h"
#include "Client_State.h"
#include "Logger.h"

using boost::asio::ip::tcp;

namespace SMPTE_SYNC
{

    DCS_Client::DCS_Client(boost::asio::io_service& io_service,
                           tcp::resolver::iterator endpoint_iterator,
                           int32_t iMessageHeaderSize,
                           Client_State *iClientState,
                           SetRPLLocationCallback iCallback)
                            :   io_service_(io_service)
                                , socket_(io_service)
                                , clientState_(iClientState)
                                , setRPLLocationCallback_(iCallback)
    
    {
        readHeaderBufferSize_ = iMessageHeaderSize;
        readHeaderBuffer_ = new uint8_t[readHeaderBufferSize_];
        memset(readHeaderBuffer_, 0x0, readHeaderBufferSize_);

        // Start off with a payload the same size as the header
        //
        readPayloadBufferSize_ = iMessageHeaderSize;
        readPayloadBuffer_ = new uint8_t[readPayloadBufferSize_];
        memset(readPayloadBuffer_, 0x0, readPayloadBufferSize_);
        
        // Start off with a payload the same size as the header
        //
        writeBufferSize_ = iMessageHeaderSize * 2;
        writeBuffer_ = new uint8_t[writeBufferSize_];

        boost::asio::async_connect(socket_, endpoint_iterator,
                                boost::bind(&DCS_Client::HandleConnect, this,
                                            boost::asio::placeholders::error));
    }

    DCS_Client::~DCS_Client()
    {
        delete [] readHeaderBuffer_;
        readHeaderBuffer_ = nullptr;
        
        delete [] readPayloadBuffer_;
        readPayloadBuffer_ = nullptr;
        
        delete [] writeBuffer_;
        writeBuffer_ = nullptr;
    }

    void DCS_Client::Send(DCS_Message_Ptr msg)
    {
        io_service_.post(boost::bind(&DCS_Client::DoWrite, this, msg));
    }

    void DCS_Client::Close()
    {
        io_service_.post(boost::bind(&DCS_Client::DoClose, this));
    }

    void DCS_Client::HandleConnect(const boost::system::error_code& error)
    {
        if (!error)
        {
            this->SetState(eState_Connected);
            boost::asio::async_read(socket_,
                                    boost::asio::buffer(readHeaderBuffer_, readHeaderBufferSize_),
                                    boost::bind(&DCS_Client::HandleReadHeader, this,
                                                boost::asio::placeholders::error));
        }
        else
        {
            SMPTE_SYNC_LOG << "DCS_Client::HandleConnect error = "
            << error.value() << " "
            << error.message();

            this->DoClose();
        }
    }

    void DCS_Client::HandleReadHeader(const boost::system::error_code& error)
    {
        if (!error)
        {
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
                                    boost::bind(&DCS_Client::HandleReadBody, this,
                                                boost::asio::placeholders::error));
        }
        else
        {
            SMPTE_SYNC_LOG << "DCS_Client::HandleReadHeader error = "
            << error.value() << " "
            << error.message();

            this->DoClose();
        }
    }

    void DCS_Client::HandleReadBody(const boost::system::error_code& error)
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
                                    boost::bind(&DCS_Client::HandleReadHeader, this,
                                                boost::asio::placeholders::error));
        }
        else
        {
            SMPTE_SYNC_LOG << "DCS_Client::HandleReadBody error = "
            << error.value() << " "
            << error.message();

            this->DoClose();
        }
    }

    void DCS_Client::DoWrite(DCS_Message_Ptr msg)
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
                                    boost::bind(&DCS_Client::HandleWrite, this,
                                                boost::asio::placeholders::error));
        }
    }

    void DCS_Client::HandleWrite(const boost::system::error_code& error)
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
                                         boost::bind(&DCS_Client::HandleWrite, this, boost::asio::placeholders::error));
            }
        }
        else
        {
            SMPTE_SYNC_LOG << "DCS_Client::HandleWrite error = "
            << error.value() << " "
            << error.message();

            this->DoClose();
        }
    }

    void DCS_Client::DoClose(void)
    {
        this->SetState(eState_Disconnected);
        socket_.close();
    }

    bool DCS_Client::Execute(DCS_Message *iMsg)
    {
        SMPTE_SYNC_LOG_LEVEL(trace) << "DCS_Client::Execute";
        
        bool success = true;
        
        /*
        if (iMsg->GetKind1() == 0x02 && iMsg->GetKind2() == 0x01)
        {
            SMPTE_SYNC_LOG_LEVEL(trace) << "DCS_Client::Execute - DCS_Message_AnnounceResponse";
        }
        */
        
        if (iMsg->GetKind1() == DCS_Message_AnnounceRequest::kind1_ && iMsg->GetKind2() == DCS_Message_AnnounceRequest::kind2_)
        {
            SMPTE_SYNC_LOG_LEVEL(trace) << "DCS_Client::Execute - DCS_Message_AnnounceRequest";

            DCS_Message_AnnounceRequest *request = dynamic_cast<DCS_Message_AnnounceRequest*>(iMsg);
            
            if (request != nullptr)
            {
                DCS_Message_AnnounceResponse *response = new DCS_Message_AnnounceResponse();
                
                time_t timeSinceEpochInSeconds = time(NULL);
                
                response->SetCurrentTime(timeSinceEpochInSeconds);
                response->SetRequestID(request->GetRequestID());
                response->SetDeviceDescription("DCS_Client");
                response->SetResponseKey(eResponseKey_RRPSuccessful);
                //response->SetResponseText("OK");
                
                this->Send(DCS_Message_Ptr(response));
            }
            else
                SMPTE_SYNC_LOG_LEVEL(trace) << "DCS_Client::Execute - ERROR UNKNOWN COMMAND DCS_Message_AnnounceRequest";
        }

        if (iMsg->GetKind1() == DCS_Message_GetNewLeaseRequest::kind1_ && iMsg->GetKind2() == DCS_Message_GetNewLeaseRequest::kind2_)
        {
            SMPTE_SYNC_LOG_LEVEL(trace) << "DCS_Client::Execute - DCS_Message_GetNewLeaseRequest";
            
            DCS_Message_GetNewLeaseRequest *request = dynamic_cast<DCS_Message_GetNewLeaseRequest*>(iMsg);
            
            if (request != nullptr)
            {
                DCS_Message_GetNewLeaseResponse *response = new DCS_Message_GetNewLeaseResponse();
                
                response->SetRequestID(request->GetRequestID());
                response->SetResponseKey(eResponseKey_RRPSuccessful);
                
                this->Send(DCS_Message_Ptr(response));
            }
            else
                SMPTE_SYNC_LOG_LEVEL(trace) << "DCS_Client::Execute - ERROR UNKNOWN COMMAND DCS_Message_GetNewLeaseRequest";
        }

        if (iMsg->GetKind1() == DCS_Message_SetRPLLocationRequest::kind1_ && iMsg->GetKind2() == DCS_Message_SetRPLLocationRequest::kind2_)
        {
            SMPTE_SYNC_LOG_LEVEL(trace) << "DCS_Client::Execute - DCS_Message_SetRPLLocationRequest";
            
            DCS_Message_SetRPLLocationRequest *request = dynamic_cast<DCS_Message_SetRPLLocationRequest*>(iMsg);
            
            if (request != nullptr)
            {
                std::string resourceURL = request->GetResourceURL();
                SMPTE_SYNC_LOG_LEVEL(trace) << "resourceURL = " << resourceURL;
                
                DCS_Message_SetRPLLocationResponse *response = new DCS_Message_SetRPLLocationResponse();
                
                response->SetRequestID(request->GetRequestID());
                response->SetResponseKey(eResponseKey_RRPSuccessful);
                
                this->Send(DCS_Message_Ptr(response));

                if (setRPLLocationCallback_ != nullptr)
                    setRPLLocationCallback_(resourceURL);
            }
            else
                SMPTE_SYNC_LOG_LEVEL(trace) << "DCS_Client::Execute - ERROR UNKNOWN COMMAND DCS_Message_SetRPLLocationRequest";
        }

        if (iMsg->GetKind1() == DCS_Message_GetStatusRequest::kind1_ && iMsg->GetKind2() == DCS_Message_GetStatusRequest::kind2_)
        {
            SMPTE_SYNC_LOG_LEVEL(trace) << "DCS_Client::Execute - DCS_Message_GetStatusRequest";
            
            DCS_Message_GetStatusRequest *request = dynamic_cast<DCS_Message_GetStatusRequest*>(iMsg);
            
            if (request != nullptr)
            {
                uint32_t requestID = request->GetRequestID();
                SMPTE_SYNC_LOG_LEVEL(trace) << "requestID = " << requestID;
                
                DCS_Message_GetStatusResponse *response = new DCS_Message_GetStatusResponse();
                
                response->SetRequestID(request->GetRequestID());
                
                ResponseKey reponse = eResponseKey_Processing;
                Client_State::EState state = clientState_->GetState();

                if (state == Client_State::eState_Play)
                {
                    reponse = eResponseKey_RRPSuccessful;
                }
                
                response->SetResponseKey(reponse);
                
                this->Send(DCS_Message_Ptr(response));
            }
            else
                SMPTE_SYNC_LOG << "DCS_Client::Execute - ERROR UNKNOWN COMMAND DCS_Message_GetStatusRequest";
        }

        if (iMsg->GetKind1() == DCS_Message_UpdateTimelineRequest::kind1_ && iMsg->GetKind2() == DCS_Message_UpdateTimelineRequest::kind2_)
        {
            SMPTE_SYNC_LOG_LEVEL(trace) << "DCS_Client::Execute - DCS_Message_UpdateTimelineRequest";
            
            DCS_Message_UpdateTimelineRequest *request = dynamic_cast<DCS_Message_UpdateTimelineRequest*>(iMsg);
            
            if (request != nullptr)
            {
                uint32_t requestID = request->GetRequestID();
                SMPTE_SYNC_LOG_LEVEL(trace) << "requestID = " << requestID;
                
                DCS_Message_UpdateTimelineResponse *response = new DCS_Message_UpdateTimelineResponse();
                
                response->SetRequestID(request->GetRequestID());
                response->SetResponseKey(eResponseKey_RRPSuccessful);
                
                this->Send(DCS_Message_Ptr(response));
            }
            else
                SMPTE_SYNC_LOG << "DCS_Client::Execute - ERROR UNKNOWN COMMAND DCS_Message_UpdateTimelineRequest";
        }

        if (iMsg->GetKind1() == DCS_Message_SetOutputModeRequest::kind1_ && iMsg->GetKind2() == DCS_Message_SetOutputModeRequest::kind2_)
        {
            SMPTE_SYNC_LOG_LEVEL(trace) << "DCS_Client::Execute - DCS_Message_SetOutputModeRequest";
            
            DCS_Message_SetOutputModeRequest *request = dynamic_cast<DCS_Message_SetOutputModeRequest*>(iMsg);
            
            if (request != nullptr)
            {
                uint32_t requestID = request->GetRequestID();
                SMPTE_SYNC_LOG_LEVEL(trace) << "requestID = " << requestID;
                
                DCS_Message_SetOutputModeResponse *response = new DCS_Message_SetOutputModeResponse();
                
                response->SetRequestID(request->GetRequestID());
                response->SetResponseKey(eResponseKey_RRPSuccessful);
                
                this->Send(DCS_Message_Ptr(response));
            }
            else
                SMPTE_SYNC_LOG << "DCS_Client::Execute - ERROR UNKNOWN COMMAND DCS_Message_SetOutputModeRequest";
        }

        if (iMsg->GetKind1() == DCS_Message_GetLogEventListRequest::kind1_ && iMsg->GetKind2() == DCS_Message_GetLogEventListRequest::kind2_)
        {
            SMPTE_SYNC_LOG_LEVEL(trace) << "DCS_Client::Execute - DCS_Message_GetLogEventListRequest";
            
            DCS_Message_GetLogEventListRequest *request = dynamic_cast<DCS_Message_GetLogEventListRequest*>(iMsg);
            
            if (request != nullptr)
            {
                uint32_t requestID = request->GetRequestID();
                SMPTE_SYNC_LOG_LEVEL(trace) << "requestID = " << requestID;
                
                DCS_Message_GetLogEventListResponse *response = new DCS_Message_GetLogEventListResponse();
                
                response->SetRequestID(request->GetRequestID());
                response->SetResponseKey(eResponseKey_RRPSuccessful);
                
                this->Send(DCS_Message_Ptr(response));
            }
            else
                SMPTE_SYNC_LOG << "DCS_Client::Execute - ERROR UNKNOWN COMMAND DCS_Message_GetLogEventListRequest";
        }

        if (iMsg->GetKind1() == DCS_Message_GetLogEventRequest::kind1_ && iMsg->GetKind2() == DCS_Message_GetLogEventRequest::kind2_)
        {
            SMPTE_SYNC_LOG_LEVEL(trace) << "DCS_Client::Execute - DCS_Message_GetLogEventRequest";
            
            DCS_Message_GetLogEventRequest *request = dynamic_cast<DCS_Message_GetLogEventRequest*>(iMsg);
            
            if (request != nullptr)
            {
                uint32_t requestID = request->GetRequestID();
                SMPTE_SYNC_LOG_LEVEL(trace) << "requestID = " << requestID;
                
                DCS_Message_GetLogEventResponse *response = new DCS_Message_GetLogEventResponse();
                
                response->SetRequestID(request->GetRequestID());
                response->SetResponseKey(eResponseKey_RRPSuccessful);
                
                this->Send(DCS_Message_Ptr(response));
            }
            else
                SMPTE_SYNC_LOG << "DCS_Client::Execute - ERROR UNKNOWN COMMAND DCS_Message_GetLogEventRequest";
        }
        
        return success;
    }
    
}  // namespace SMPTE_SYNC
