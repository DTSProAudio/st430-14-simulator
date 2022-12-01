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

#include "SS_Client.h"
#include <string>

#include "Logger.h"
#include "AuxDataMgr.h"

namespace SMPTE_SYNC
{

    SS_Client::SS_Client(boost::asio::io_service& io_service
                         , AuxDataMgr *iAuxDataMgr
                         , int32_t iEditUnitsPerRequest
                         , int32_t iEditUnitsAheadOfCurrentEditUnitToRequest
                         , int32_t iEditUnitsAheadOfCurrentEditUnitToInitiateRequest
                         , int32_t iMillisecondsPerFrame
                         , const std::string& iCodingUL
                         , const std::string& iEncryptionType
                         , CurrentFrameCallback iCallback)
    :   resolver_(io_service)
        , socket_(io_service)
        , auxDataMgr_(iAuxDataMgr)
        , currentFrameCallback_(iCallback)
        , keepRequestingAuxDataItem_(true)
        , pauseRequestAuxDataItem_(true)
        , getInProgress_(false)
        , port_("")
        , server_("")
        , startEditUnit_(0)
        , editUnitsPerRequest_(iEditUnitsPerRequest) // approximately 10 seconds
        , editUnitsAheadOfCurrentEditUnitToRequest_(iEditUnitsAheadOfCurrentEditUnitToRequest) // approximately 10 seconds
        , editUnitsAheadOfCurrentEditUnitToInitiateRequest_(iEditUnitsAheadOfCurrentEditUnitToInitiateRequest) // approximately 5 seconds
        , millisecondsPerFrame_(iMillisecondsPerFrame) // 1000 / 24
        , codingUL_(iCodingUL)
        , encryptionType_(iEncryptionType)
    {
    }
    
    SS_Client::~SS_Client()
    {
        keepRequestingAuxDataItem_ = false;
        runRequestAuxDataItem_.notify_one();
        fetchAuxDataItemThread_.join();
    }

    void SS_Client::SetServer(const std::string& iServer)
    {
        server_ = iServer;
    }
    
    const std::string& SS_Client::GetServer(void)
    {
        return server_;
    }
    
    void SS_Client::SetPort(const std::string& iPort)
    {
        port_ = iPort;
    }
    
    const std::string& SS_Client::GetPort(void)
    {
        return port_;
    }

    void SS_Client::SetServerAndPort(const std::string &iURL)
    {
        /// TODO: use a real URI parser

        // http://127.0.0.1:21234/
        //
        size_t beginning = iURL.find("://") + 3;
        size_t end = iURL.find(":", beginning);
        server_ = iURL.substr(beginning, end - beginning);
        
        beginning = end + 1;
        end = iURL.find("/", beginning);
        port_ = iURL.substr(beginning, end - beginning);

        if (server_ != "" && port_ != "")
        {
            SMPTE_SYNC_LOG << "SS_Client::SetServerAndPort - Starting GET thread...";
            SMPTE_SYNC_LOG << "port_ = " << port_;
            SMPTE_SYNC_LOG << "server_ = " << server_;
            
            // Kick off our first fetch of data
            //
            std::string path = this->BuildPath();
            this->GET(path);

            // Now that we have a valid IP and port of the server
            // spin up the thread
            //
            fetchAuxDataItemThread_ = boost::thread(&SS_Client::RequestAuxDataItem, this);
        }
        else
        {
            SMPTE_SYNC_LOG << "SS_Client::SetServerAndPort - Error setting up IP and port";
            SMPTE_SYNC_LOG << "port_ = " << port_;
            SMPTE_SYNC_LOG << "server_ = " << server_;
        }
    }
    
    void SS_Client::GET(const std::string& iPath)
    {
        SMPTE_SYNC_LOG << "SS_Client::GET: " << iPath;
        
        getInProgress_ = true;

        this->SetState(eState_Buffering);

        // Form the request. We specify the "Connection: close" header so that the
        // server will close the socket after transmitting the response. This will
        // allow us to treat all data up until the EOF as the content.
        std::ostream request_stream(&request_);
        request_stream << "GET " << iPath << " HTTP/1.0\r\n";
        request_stream << "Host: " << server_ << "\r\n";
        request_stream << "Accept: */*\r\n";
        request_stream << "Connection: close\r\n\r\n";
        
        // Start an asynchronous resolve to translate the server and service names
        // into a list of endpoints.
        //tcp::resolver::query query(server, "http");
        tcp::resolver::query query(server_, port_);
        resolver_.async_resolve(query,
                                boost::bind(&SS_Client::handle_resolve, this,
                                            boost::asio::placeholders::error,
                                            boost::asio::placeholders::iterator));
    }

    void SS_Client::handle_resolve(const boost::system::error_code& err,
                        tcp::resolver::iterator endpoint_iterator)
    {
        if (!err)
        {
            // Attempt a connection to each endpoint in the list until we
            // successfully establish a connection.
            boost::asio::async_connect(socket_, endpoint_iterator,
                                       boost::bind(&SS_Client::handle_connect, this,
                                                   boost::asio::placeholders::error));
        }
        else
        {
            SMPTE_SYNC_LOG << "Error: " << err.message();
            this->HandleError();
        }
    }
    
    void SS_Client::handle_connect(const boost::system::error_code& err)
    {
        if (!err)
        {
            this->SetState(eState_Connected);

            // The connection was successful. Send the request.
            boost::asio::async_write(socket_, request_,
                                     boost::bind(&SS_Client::handle_write_request, this,
                                                 boost::asio::placeholders::error));
        }
        else
        {
            SMPTE_SYNC_LOG << "Error: " << err.message();
            this->HandleError();
        }
    }
    
    void SS_Client::handle_write_request(const boost::system::error_code& err)
    {
        if (!err)
        {
            // Read the response status line. The response_ streambuf will
            // automatically grow to accommodate the entire line. The growth may be
            // limited by passing a maximum size to the streambuf constructor.
            boost::asio::async_read_until(socket_, response_, "\r\n",
                                          boost::bind(&SS_Client::handle_read_status_line, this,
                                                      boost::asio::placeholders::error));
        }
        else
        {
            SMPTE_SYNC_LOG << "Error: " << err.message();
            this->HandleError();
        }
    }
    
    void SS_Client::handle_read_status_line(const boost::system::error_code& err)
    {
        if (!err)
        {
            // Check that response is OK.
            std::istream response_stream(&response_);
            std::string http_version;
            response_stream >> http_version;
            unsigned int status_code;
            response_stream >> status_code;
            
            std::string status_message;
            std::getline(response_stream, status_message);
            if (!response_stream || http_version.substr(0, 5) != "HTTP/")
            {
                SMPTE_SYNC_LOG << "Invalid response\n";
                return;
            }
            if (status_code != 200)
            {
                SMPTE_SYNC_LOG << "Response returned with status code ";
                SMPTE_SYNC_LOG << status_code;
                return;
            }
            
            // Read the response headers, which are terminated by a blank line.
            boost::asio::async_read_until(socket_, response_, "\r\n\r\n",
                                          boost::bind(&SS_Client::handle_read_headers, this,
                                                      boost::asio::placeholders::error));
        }
        else
        {
            SMPTE_SYNC_LOG << "Error: " << err;
            this->HandleError();
        }
    }
    
    void SS_Client::handle_read_headers(const boost::system::error_code& err)
    {
        if (!err)
        {
            // Process the response headers.
            std::istream response_stream(&response_);
            std::string header;
            while (std::getline(response_stream, header) && header != "\r")
            {
                //SMPTE_SYNC_LOG << header;
            }
            //SMPTE_SYNC_LOG;
            
            // Write whatever content we already have to output.
            if (response_.size() > 0)
            {
                std::ostringstream ss;
                ss << &response_;
                
                responsePayload_ = ss.str();

                //SMPTE_SYNC_LOG << "SS_Client::handle_read_headers\n";
                //SMPTE_SYNC_LOG << responsePayload_;
            }
            // Start reading remaining data until EOF.
            boost::asio::async_read(socket_, response_,
                                    boost::asio::transfer_at_least(1),
                                    boost::bind(&SS_Client::handle_read_content, this,
                                                boost::asio::placeholders::error));
        }
        else
        {
            SMPTE_SYNC_LOG << "SS_Client::handle_read_headers Error: " << err;
            responsePayload_.clear();
            this->HandleError();
        }
    }
    
    void SS_Client::handle_read_content(const boost::system::error_code& err)
    {
        if (!err)
        {
            // Write all of the data that has been read so far.
            std::ostringstream ss;
            ss << &response_;
            
            responsePayload_ += ss.str();

            //SMPTE_SYNC_LOG << "SS_Client::handle_read_content\n";
            //SMPTE_SYNC_LOG << responsePayload_;

            // Continue reading remaining data until EOF.
            boost::asio::async_read(socket_, response_,
                                    boost::asio::transfer_at_least(1),
                                    boost::bind(&SS_Client::handle_read_content, this,
                                                boost::asio::placeholders::error));
        }
        else if (err == boost::asio::error::eof)
        {
            //SMPTE_SYNC_LOG << "SS_Client::handle_read_content EOF\n" << std::flush;
            //SMPTE_SYNC_LOG << "responsePayload_\n" << responsePayload_ << std::endl;
            
            int32_t bytesConsumed = 0;
            int32_t payloadLength = static_cast<int32_t>(responsePayload_.length());
            uint8_t *dataBuf = new uint8_t[payloadLength];
            uint8_t *dataBufOrig = dataBuf;
            
            memcpy(dataBuf, responsePayload_.data(), payloadLength);

            // Read the header
            //
            AuxDataBlockTransferHeader header;
            header.read(&dataBuf);
            bytesConsumed += header.GetSizeInBytes();

            while (bytesConsumed < payloadLength)
            {
                AuxDataBlock *item = new AuxDataBlock();
                item->read(&dataBuf);
                bytesConsumed += item->GetSizeInBytes();
                
                auxDataMgr_->AddDataItem(item);
            }
            
            {
                boost::mutex::scoped_lock path_lock(buildPathMutex_);
                
                if (header.editUnitRangeCount_ > 0)
                {
                    startEditUnit_ = header.editUnitRangeStartIndex_ + header.editUnitRangeCount_;
                    SMPTE_SYNC_LOG << "SS_Client::handle_read_content startEditUnit_ - " << startEditUnit_ << std::endl;
                    SMPTE_SYNC_LOG << "SS_Client::handle_read_content header.editUnitRangeStartIndex_ - " << header.editUnitRangeStartIndex_ << " header.editUnitRangeCount_ - " << header.editUnitRangeCount_ << std::endl;
                }
                else
                {
                    startEditUnit_ = header.editUnitRangeStartIndex_;
                    SMPTE_SYNC_LOG << "SS_Client::handle_read_content startEditUnit_ - " << startEditUnit_ << std::endl;
                    SMPTE_SYNC_LOG << "SS_Client::handle_read_content header.editUnitRangeStartIndex_ - " << header.editUnitRangeStartIndex_ << std::endl;
                }
                
                // Clear the flag for waiting on a response
                // such that we can make another request
                //
                getInProgress_ = false;
            }

            delete [] dataBufOrig;
            responsePayload_.clear();
        }
        else if (err != boost::asio::error::eof)
        {
            SMPTE_SYNC_LOG << "SS_Client::handle_read_content Error: " << err;
            responsePayload_.clear();
            this->HandleError();
        }
    }

    std::string SS_Client::BuildPath(void)
    {
        std::string path = "";
        
        boost::mutex::scoped_lock path_lock(buildPathMutex_);

        path = std::string("/v1/auxdata/editunits?coding_UL=")
        + codingUL_
        + std::string("&start=") + std::to_string(startEditUnit_)
        + std::string("&count=") + std::to_string(editUnitsPerRequest_)
        + std::string("&accept=") + encryptionType_
        ;
        
        //SMPTE_SYNC_LOG << "SS_Client::BuildPath requesting data starting with startEditUnit_ = " << startEditUnit_;

        return path;
    }

    void SS_Client::SetEditUnitsPerRequest(int32_t iUnits)
    {
        {
            boost::mutex::scoped_lock path_lock(buildPathMutex_);
            editUnitsPerRequest_ = iUnits;
        }
        this->BuildPath();
    }
    
    int32_t SS_Client::GetEditUnitsPerRequest(void)
    {
        return editUnitsPerRequest_;
    }
    
    void SS_Client::SetEditUnitsAheadOfCurrentEditUnitToRequest(int32_t iUnits)
    {
        editUnitsAheadOfCurrentEditUnitToRequest_ = iUnits;
        this->BuildPath();
    }

    int32_t SS_Client::GetEditUnitsAheadOfCurrentEditUnitToRequest(void)
    {
        return editUnitsAheadOfCurrentEditUnitToRequest_;
    }
    
    void SS_Client::SetEditUnitsAheadOfCurrentEditUnitToInitiateRequest(int32_t iUnits)
    {
        editUnitsAheadOfCurrentEditUnitToInitiateRequest_ = iUnits;
    }
    
    int32_t SS_Client::GetEditUnitsAheadOfCurrentEditUnitToInitiateRequest(void)
    {
        return editUnitsAheadOfCurrentEditUnitToInitiateRequest_;
    }

    void SS_Client::SetMillisecondsPerFrame(int32_t iMilliseconds)
    {
        millisecondsPerFrame_ = iMilliseconds;
    }
    
    int32_t SS_Client::GetMillisecondsPerFrame(void)
    {
        return millisecondsPerFrame_;
    }

    void SS_Client::SetMilliscondsPerFrameWithFrameRate(int32_t iNumerator, int32_t iDenominator)
    {
        float frameRate = static_cast<float>(iNumerator) / static_cast<float>(iDenominator);
        
        millisecondsPerFrame_ = 1000 / frameRate;
    }
    
    void SS_Client::SetCodingUL(const std::string &iCodingUL)
    {
        {
            boost::mutex::scoped_lock path_lock(buildPathMutex_);
            codingUL_ = iCodingUL;
        }
        this->BuildPath();
    }

    const std::string& SS_Client::GetCodingUL(void)
    {
        return codingUL_;
    }
    
    void SS_Client::SetEncryptionType(const std::string &iEncryptionType)
    {
        {
            boost::mutex::scoped_lock path_lock(buildPathMutex_);
            encryptionType_ = iEncryptionType;
        }
        this->BuildPath();
    }

    const std::string& SS_Client::GetEncryptionType(void)
    {
        return encryptionType_;
    }

    void SS_Client::RequestAuxDataItem(void)
    {
        assert(auxDataMgr_ != nullptr);

        while (keepRequestingAuxDataItem_)
        {
            {
                boost::mutex::scoped_lock scoped_lock(runAuxDataItemMutex_);
                while (pauseRequestAuxDataItem_ && keepRequestingAuxDataItem_)
                {
                    int32_t currentFrame = 0;
                    if (currentFrameCallback_)
                        currentFrame = currentFrameCallback_();
                    
                    SMPTE_SYNC_LOG << "SS_Client::RequestAuxDataItem"
                    << " getInProgress_ = " << (getInProgress_ ? "true" : "false")
                    << " startEditUnit_ - " << startEditUnit_
                    << " currentFrame - " << currentFrame
                    << std::endl;

                    if (getInProgress_)
                    {
                        // A get has already been sent and we are still waiting for a response
                        //
                        if (currentFrame > startEditUnit_)
                        {
                            // We've blown our deadline
                            // Just keep checking and warning
                            //
                            SMPTE_SYNC_LOG << "SS_Client::RequestAuxDataItem deadline missed, keep looping " << std::endl;
                        }
                        else
                        {
                            // Go back to sleep
                            //
                            // Compute the time to fetch data.
                            // This will be the data point we need to fetch minus our fetch ahead buffer
                            //
                            int32_t frameToStartNextFetchOn = 0;

                            {
                                boost::mutex::scoped_lock path_lock(buildPathMutex_);
                                frameToStartNextFetchOn = (startEditUnit_ + editUnitsAheadOfCurrentEditUnitToRequest_) - editUnitsAheadOfCurrentEditUnitToInitiateRequest_;
                            }

                            int32_t framesToWait = 0;
                            if (currentFrame <= frameToStartNextFetchOn)
                            {
                                framesToWait = frameToStartNextFetchOn - currentFrame;
                            }
                            else
                            {
                                //SMPTE_SYNC_LOG << "SS_Client::RequestAuxDataItem Less time to fetch than required!\n";
                            }
                            
                            int32_t millisecondsToWait = framesToWait * millisecondsPerFrame_;
                            boost::posix_time::milliseconds wait_duration(millisecondsToWait);

                            boost::system_time currentTime = boost::get_system_time();
                            boost::system_time const timeout = currentTime + wait_duration;
                            
                            runRequestAuxDataItem_.timed_wait(scoped_lock, timeout);

                            boost::posix_time::time_duration diff = boost::get_system_time() - currentTime;
                            SMPTE_SYNC_LOG << "SS_Client::RequestAuxDataItem slept = " << diff.total_milliseconds() << "ms";
                        }
                    }
                    else
                    {
                        int32_t frameToStartNextFetchOn = 0;
                        {
                            boost::mutex::scoped_lock path_lock(buildPathMutex_);

                            // If our position of our next aux data item fetch is
                            // less than the current position, we are in an
                            // underflow situation.
                            //
                            // We need to update the value for what we aux data item
                            // we need to fecth.
                            //
                            if (currentFrame > startEditUnit_)
                                startEditUnit_ = currentFrame + editUnitsAheadOfCurrentEditUnitToRequest_;
                            
                            // Compute the time to fetch data.
                            // This will be the data point we need to fetch minus our fetch ahead buffer
                            //
                            frameToStartNextFetchOn = startEditUnit_ - editUnitsAheadOfCurrentEditUnitToInitiateRequest_;

                            if (frameToStartNextFetchOn < 0)
                                frameToStartNextFetchOn = 0;
                        }
                        
                        boost::system_time currentTime = boost::get_system_time();

                        int32_t framesToWait = 0;
                        if (currentFrame <= frameToStartNextFetchOn)
                        {
                            framesToWait = frameToStartNextFetchOn - currentFrame;
                        }
                        else
                        {
                            //SMPTE_SYNC_LOG << "SS_Client::RequestAuxDataItem Less time to fetch than required!\n";
                        }
                        
                        int32_t millisecondsToWait = framesToWait * millisecondsPerFrame_;
                        boost::posix_time::milliseconds wait_duration(millisecondsToWait);
                        
                        SMPTE_SYNC_LOG << "SS_Client::RequestAuxDataItem about to sleep "
                        << "wait_duration = "
                        << wait_duration.total_milliseconds()
                        << " currentFrame = "
                        << currentFrame
                        << " frameToStartNextFetchOn = "
                        << frameToStartNextFetchOn
                        << " startEditUnit_ = "
                        << startEditUnit_;
                        
                        boost::system_time const timeout = currentTime + wait_duration;

                        // We aren't checking the return value since
                        // we really just want to make a new request
                        // if for some reason we request too soon
                        // the request will do the right thing and return
                        // an error code.
                        //
                        if (framesToWait > 0)
                        {
                            this->SetState(eState_Buffered);
                            runRequestAuxDataItem_.timed_wait(scoped_lock, timeout);

                            // Check time of the frames
                            // now that we have woken up
                            //
                            if (currentFrameCallback_)
                                currentFrame = currentFrameCallback_();
                        }
                        else
                        {
                            SMPTE_SYNC_LOG << "SS_Client::RequestAuxDataItem framesToWait = 0 Skipped runRequestAuxDataItem_.timed_wait\n";
                        }
                        
                        boost::posix_time::time_duration diff = boost::get_system_time() - currentTime;
                        SMPTE_SYNC_LOG << "SS_Client::RequestAuxDataItem slept = " << diff.total_milliseconds() << "ms";
                        
                        // Now that the thread has woken up
                        // See if we need to fetch new frames
                        //
                        if (currentFrame >= frameToStartNextFetchOn)
                        {
                            pauseRequestAuxDataItem_ = false;
                        }
                        
                        SMPTE_SYNC_LOG << "SS_Client::RequestAuxDataItem pauseRequestAuxDataItem_ = "
                        << (pauseRequestAuxDataItem_ ? "true" : "false")
                        << " startEditUnit_ - " << startEditUnit_;
                        
                        SMPTE_SYNC_LOG << "SS_Client::RequestAuxDataItem"
                        << " currentFrame - " << currentFrame
                        << " frameToStartNextFetchOn - " << frameToStartNextFetchOn;
                    }
                }
                // Now that we are outside of the timing, pause loop
                // Set the pause flag to true such that we execute
                // the pause code after fetching our aux data items
                //
                pauseRequestAuxDataItem_ = true;
            }

            if (!keepRequestingAuxDataItem_)
                break;
            
            // Only initiate the request for the startEditUnit_ once
            //
            if (!getInProgress_)
            {
                std::string path = this->BuildPath();
                this->GET(path);
            }
        }
    }
    
    void SS_Client::HandleError(void)
    {
        this->SetState(eState_Disconnected);
        // If we have an error, we need to decrement the startEditUnit_
        // Since we incremented it before initiating the GET request
        //
        boost::mutex::scoped_lock scoped_lock(buildPathMutex_);
        startEditUnit_ -= editUnitsPerRequest_;
        
        // Clear the flag for waiting on a response
        // such that we can make another request
        //
        getInProgress_ = false;
    }
}  // namespace SMPTE_SYNC
