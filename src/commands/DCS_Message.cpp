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

#include "DCS_Message.h"

#include <assert.h>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <vector>

#include "Logger.h"

namespace SMPTE_SYNC
{

    bool MessageHeader::Write(uint8_t **iCommandBuffer)
    {
        SMPTE_SYNC_LOG_LEVEL(trace) << "MessageHeader::write";

        bool success = true;
        
        SMPTE_SYNC::Write(iCommandBuffer, objectID_);
        SMPTE_SYNC::Write(iCommandBuffer, labelSize_);
        SMPTE_SYNC::Write(iCommandBuffer, designator1_);
        SMPTE_SYNC::Write(iCommandBuffer, designator2_);
        SMPTE_SYNC::Write(iCommandBuffer, registryCategoryDesignator_);
        SMPTE_SYNC::Write(iCommandBuffer, registryDesignator_);
        SMPTE_SYNC::Write(iCommandBuffer, structureDesignator_);
        SMPTE_SYNC::Write(iCommandBuffer, versionNumber_);
        SMPTE_SYNC::Write(iCommandBuffer, itemDesignator_);
        SMPTE_SYNC::Write(iCommandBuffer, organization_);
        SMPTE_SYNC::Write(iCommandBuffer, application_);
        SMPTE_SYNC::Write(iCommandBuffer, kind1_);
        SMPTE_SYNC::Write(iCommandBuffer, kind2_);
        SMPTE_SYNC::Write(iCommandBuffer, reserved1_);
        SMPTE_SYNC::Write(iCommandBuffer, reserved2_);
        SMPTE_SYNC::Write(iCommandBuffer, reserved3_);
        SMPTE_SYNC::WriteBER4(iCommandBuffer, length_);

        return success;
    }
    
    bool MessageHeader::Read(uint8_t **iCommandBuffer)
    {
        SMPTE_SYNC_LOG_LEVEL(trace) << "MessageHeader::read";

        bool success = true;
        
        SMPTE_SYNC::Read(iCommandBuffer, objectID_);
        SMPTE_SYNC::Read(iCommandBuffer, labelSize_);
        SMPTE_SYNC::Read(iCommandBuffer, designator1_);
        SMPTE_SYNC::Read(iCommandBuffer, designator2_);
        SMPTE_SYNC::Read(iCommandBuffer, registryCategoryDesignator_);
        SMPTE_SYNC::Read(iCommandBuffer, registryDesignator_);
        SMPTE_SYNC::Read(iCommandBuffer, structureDesignator_);
        SMPTE_SYNC::Read(iCommandBuffer, versionNumber_);
        SMPTE_SYNC::Read(iCommandBuffer, itemDesignator_);
        SMPTE_SYNC::Read(iCommandBuffer, organization_);
        SMPTE_SYNC::Read(iCommandBuffer, application_);
        SMPTE_SYNC::Read(iCommandBuffer, kind1_);
        SMPTE_SYNC::Read(iCommandBuffer, kind2_);
        SMPTE_SYNC::Read(iCommandBuffer, reserved1_);
        SMPTE_SYNC::Read(iCommandBuffer, reserved2_);
        SMPTE_SYNC::Read(iCommandBuffer, reserved3_);
        SMPTE_SYNC::ReadBER4(iCommandBuffer, length_);
        
        return success;
    }

    bool KLV::Write(uint8_t **iCommandBuffer)
    {
        SMPTE_SYNC_LOG_LEVEL(trace) << "KLV::Write";

        bool success = true;
        
        length_ = static_cast<int32_t>(text_.length());
        
        SMPTE_SYNC::Write(iCommandBuffer, key_);
        WriteBER4(iCommandBuffer, length_);
        if (length_ > 0)
            SMPTE_SYNC::Write(iCommandBuffer, text_);

        return success;
    }
    
    bool KLV::Read(uint8_t **iCommandBuffer)
    {
        SMPTE_SYNC_LOG_LEVEL(trace) << "KLV::Read";

        bool success = true;
        
        SMPTE_SYNC::Read(iCommandBuffer, key_);
        ReadBER4(iCommandBuffer, length_);
        if (length_ > 0)
            SMPTE_SYNC::Read(iCommandBuffer, length_, text_);
        
        return success;
    }
    
    int32_t KLV::size(void)
    {
        int32_t size = 0;
        
        size += static_cast<int32_t>(sizeof(key_));
        size += static_cast<int32_t>(sizeof(length_));
        size += static_cast<int32_t>(text_.length());
        
        return size;
    }

    DCS_Message::DCS_Message()
    {
        SMPTE_SYNC_LOG_LEVEL(trace) << "DCS_Message::DCS_Message";
    }
    
    DCS_Message::~DCS_Message()
    {
        SMPTE_SYNC_LOG_LEVEL(trace) << "DCS_Message::~DCS_Message";
    }
    
    uint8_t DCS_Message::GetKind1()
    {
        return messageHeader_.kind1_;
    }
    
    uint8_t DCS_Message::GetKind2()
    {
        return messageHeader_.kind2_;
    }

    int32_t DCS_Message::GetHeaderSize(void)
    {
        return MessageHeader::headerSize_;
    }

    int32_t DCS_Message::GetMessageSize(void)
    {
        return this->GetHeaderSize() + this->GetPayloadSize();
    }
    
    int32_t DCS_Message::GetPayloadSize(void)
    {
        return messageHeader_.length_;
    }

    void DCS_Message::ReadHeader(const MessageHeader &iHeader)
    {
        SMPTE_SYNC_LOG_LEVEL(trace) << "DCS_Message::ReadHeader";
        
        messageHeader_ = iHeader;
    }

    void DCS_Message::ReadHeader(uint8_t **iCommandBuffer)
    {
        SMPTE_SYNC_LOG_LEVEL(trace) << "DCS_Message::ReadHeader";

        messageHeader_.Read(iCommandBuffer);
    }
    
    void DCS_Message::ReadPayload(uint8_t **iCommandBuffer)
    {
        
    }

    void DCS_Message::WriteHeader(uint8_t **iCommandBuffer)
    {
        SMPTE_SYNC_LOG_LEVEL(trace) << "DCS_Message::WriteHeader";

        messageHeader_.length_ = this->GetPayloadSize();
        messageHeader_.Write(iCommandBuffer);
    }
    
    void DCS_Message::WritePayload(uint8_t **iCommandBuffer)
    {
        
    }

    const MessageHeader& DCS_Message::GetMessageHeader(void)
    {
        return messageHeader_;
    }

    DCS_Message_BasicResponse::DCS_Message_BasicResponse()
    {
        SMPTE_SYNC_LOG_LEVEL(trace) << "DCS_Message_BasicResponse::DCS_Message_BasicResponse";
        messageHeader_.kind1_ = 0;
        messageHeader_.kind2_ = 0;
    }
    
    DCS_Message_BasicResponse::~DCS_Message_BasicResponse()
    {
        SMPTE_SYNC_LOG_LEVEL(trace) << "DCS_Message_BasicResponse::~DCS_Message_BasicResponse";
    }
    
    int32_t DCS_Message_BasicResponse::GetPayloadSize(void)
    {
        int32_t payloadSize = 0;
        
        payloadSize += static_cast<int32_t>(sizeof(requestID_));
        payloadSize += statusResponse_.size();
        
        return payloadSize;
    }
    
    ResponseKey DCS_Message_BasicResponse::GetResponseKey(void)
    {
        assert(statusResponse_.key_ > eResponseKey_OutOfRange && statusResponse_.key_ < eResponseKey_Reserved);
        
        return static_cast<ResponseKey>(statusResponse_.key_);
    }
    
    void DCS_Message_BasicResponse::SetResponseKey(ResponseKey iKey)
    {
        assert(iKey > eResponseKey_OutOfRange && iKey < eResponseKey_Reserved);
        
        statusResponse_.key_ = iKey;
    }
    
    std::string DCS_Message_BasicResponse::GetResponseText(void)
    {
        return statusResponse_.text_;
    }
    
    void DCS_Message_BasicResponse::SetResponseText(std::string iVal)
    {
        statusResponse_.text_ = iVal;
        statusResponse_.length_ = static_cast<int32_t>(statusResponse_.text_.length());
    }
    
    void DCS_Message_BasicResponse::ReadPayload(uint8_t **iCommandBuffer)
    {
        SMPTE_SYNC_LOG_LEVEL(trace) << "DCS_Message_BasicResponse::ReadPayload";
        Read(iCommandBuffer, requestID_);
        statusResponse_.Read(iCommandBuffer);
    }
    
    void DCS_Message_BasicResponse::WritePayload(uint8_t **iCommandBuffer)
    {
        SMPTE_SYNC_LOG_LEVEL(trace) << "DCS_Message_BasicResponse::WritePayload";
        if (messageHeader_.length_ > 0)
        {
            Write(iCommandBuffer, requestID_);
            statusResponse_.Write(iCommandBuffer);
        }
    }
    
    uint32_t DCS_Message_BasicResponse::GetRequestID(void)
    {
        return requestID_;
    }
    
    void DCS_Message_BasicResponse::SetRequestID(uint32_t iID)
    {
        requestID_ = iID;
    }

    
    
    const int8_t DCS_Message_AnnounceRequest::kind1_ = 0x02;
    const int8_t DCS_Message_AnnounceRequest::kind2_ = 0x00;

    DCS_Message_AnnounceRequest::DCS_Message_AnnounceRequest()
    {
        SMPTE_SYNC_LOG_LEVEL(trace) << "DCS_Message_AnnounceRequest::DCS_Message_AnnounceRequest";
        messageHeader_.kind1_ = kind1_;
        messageHeader_.kind2_ = kind2_;
    }

    DCS_Message_AnnounceRequest::~DCS_Message_AnnounceRequest()
    {
        SMPTE_SYNC_LOG_LEVEL(trace) << "DCS_Message_AnnounceRequest::~DCS_Message_AnnounceRequest";
    }

    int32_t DCS_Message_AnnounceRequest::GetPayloadSize(void)
    {
        int32_t payloadSize = 0;
        
        payloadSize += static_cast<int32_t>(sizeof(requestID_));
        payloadSize += static_cast<int32_t>(sizeof(currentTime_));
        payloadSize += static_cast<int32_t>(deviceDescription_.length());
        
        return payloadSize;
    }
    
    void DCS_Message_AnnounceRequest::ReadPayload(uint8_t **iCommandBuffer)
    {
        SMPTE_SYNC_LOG_LEVEL(trace) << "DCS_Message_AnnounceRequest::ReadPayload";
        Read(iCommandBuffer, requestID_);
        Read(iCommandBuffer, currentTime_);
        
        // Use the payload size as determined in the messageHeader
        // rather than calling GetPayloadSize
        //
        int32_t payloadSize = messageHeader_.length_;

        // Compute the string length
        //
        int32_t strLen = payloadSize - (static_cast<int32_t>(sizeof(requestID_)) + static_cast<int32_t>(sizeof(currentTime_)));
        Read(iCommandBuffer, strLen, deviceDescription_);
    }
    
    void DCS_Message_AnnounceRequest::WritePayload(uint8_t **iCommandBuffer)
    {
        SMPTE_SYNC_LOG_LEVEL(trace) << "DCS_Message_AnnounceRequest::WritePayload";
        if (messageHeader_.length_ > 0)
        {
            Write(iCommandBuffer, requestID_);
            Write(iCommandBuffer, currentTime_);
            if (deviceDescription_.length() > 0)
                Write(iCommandBuffer, deviceDescription_);
        }
    }

    uint32_t DCS_Message_AnnounceRequest::GetRequestID(void)
    {
        return requestID_;
    }

    void DCS_Message_AnnounceRequest::SetRequestID(uint32_t iID)
    {
        requestID_ = iID;
    }

    int64_t DCS_Message_AnnounceRequest::GetCurrentTime(void)
    {
        return currentTime_;
    }
    
    void DCS_Message_AnnounceRequest::SetCurrentTime(int64_t iCurrentTime)
    {
        currentTime_ = iCurrentTime;
    }

    std::string DCS_Message_AnnounceRequest::GetDeviceDescription(void)
    {
        return deviceDescription_;
    }
    
    void DCS_Message_AnnounceRequest::SetDeviceDescription(std::string iStr)
    {
        deviceDescription_ = iStr;
    }

    const int8_t DCS_Message_AnnounceResponse::kind1_ = 0x02;
    const int8_t DCS_Message_AnnounceResponse::kind2_ = 0x01;

    DCS_Message_AnnounceResponse::DCS_Message_AnnounceResponse()
    {
        SMPTE_SYNC_LOG_LEVEL(trace) << "DCS_Message_AnnounceResponse::DCS_Message_AnnounceResponse";
        messageHeader_.kind1_ = kind1_;
        messageHeader_.kind2_ = kind2_;
    }
    
    DCS_Message_AnnounceResponse::~DCS_Message_AnnounceResponse()
    {
        SMPTE_SYNC_LOG_LEVEL(trace) << "DCS_Message_AnnounceResponse::~DCS_Message_AnnounceResponse";
    }
    
    int32_t DCS_Message_AnnounceResponse::GetPayloadSize(void)
    {
        int32_t payloadSize = 0;
        
        payloadSize += static_cast<int32_t>(sizeof(requestID_));
        payloadSize += static_cast<int32_t>(sizeof(currentTime_));
        payloadSize += static_cast<int32_t>(sizeof(deviceDescriptionLength_));
        payloadSize += static_cast<int32_t>(deviceDescription_.length());
        payloadSize += statusResponse_.size();
        
        return payloadSize;
    }
    
    ResponseKey DCS_Message_AnnounceResponse::GetResponseKey(void)
    {
        assert(statusResponse_.key_ > eResponseKey_OutOfRange && statusResponse_.key_ < eResponseKey_Reserved);
        
        return static_cast<ResponseKey>(statusResponse_.key_);
    }
    
    void DCS_Message_AnnounceResponse::SetResponseKey(ResponseKey iKey)
    {
        assert(iKey > eResponseKey_OutOfRange && iKey < eResponseKey_Reserved);

        statusResponse_.key_ = iKey;
    }

    std::string DCS_Message_AnnounceResponse::GetResponseText(void)
    {
        return statusResponse_.text_;
    }
    
    void DCS_Message_AnnounceResponse::SetResponseText(std::string iVal)
    {
        statusResponse_.text_ = iVal;
        statusResponse_.length_ = static_cast<int32_t>(statusResponse_.text_.length());
    }

    void DCS_Message_AnnounceResponse::ReadPayload(uint8_t **iCommandBuffer)
    {
        SMPTE_SYNC_LOG_LEVEL(trace) << "DCS_Message_AnnounceResponse::ReadPayload";
        Read(iCommandBuffer, requestID_);
        Read(iCommandBuffer, currentTime_);
        ReadBER4(iCommandBuffer, deviceDescriptionLength_);
        if (deviceDescriptionLength_ > 0)
            Read(iCommandBuffer, deviceDescriptionLength_, deviceDescription_);
        statusResponse_.Read(iCommandBuffer);
    }
    
    void DCS_Message_AnnounceResponse::WritePayload(uint8_t **iCommandBuffer)
    {
        SMPTE_SYNC_LOG_LEVEL(trace) << "DCS_Message_AnnounceResponse::WritePayload";
        if (messageHeader_.length_ > 0)
        {
            Write(iCommandBuffer, requestID_);
            Write(iCommandBuffer, currentTime_);
            deviceDescriptionLength_ = static_cast<int32_t>(deviceDescription_.length());
            WriteBER4(iCommandBuffer, deviceDescriptionLength_);
            if (deviceDescriptionLength_ > 0)
                Write(iCommandBuffer, deviceDescription_);
            statusResponse_.Write(iCommandBuffer);
        }
    }
    
    uint32_t DCS_Message_AnnounceResponse::GetRequestID(void)
    {
        return requestID_;
    }
    
    void DCS_Message_AnnounceResponse::SetRequestID(uint32_t iID)
    {
        requestID_ = iID;
    }
    
    int64_t DCS_Message_AnnounceResponse::GetCurrentTime(void)
    {
        return currentTime_;
    }
    
    void DCS_Message_AnnounceResponse::SetCurrentTime(int64_t iCurrentTime)
    {
        currentTime_ = iCurrentTime;
    }
    
    std::string DCS_Message_AnnounceResponse::GetDeviceDescription(void)
    {
        return deviceDescription_;
    }
    
    void DCS_Message_AnnounceResponse::SetDeviceDescription(std::string iStr)
    {
        deviceDescription_ = iStr;
    }

    const int8_t DCS_Message_GetNewLeaseRequest::kind1_ = 0x02;
    const int8_t DCS_Message_GetNewLeaseRequest::kind2_ = 0x02;

    DCS_Message_GetNewLeaseRequest::DCS_Message_GetNewLeaseRequest()
    {
        SMPTE_SYNC_LOG_LEVEL(trace) << "DCS_Message_GetNewLeaseRequest::DCS_Message_GetNewLeaseRequest";
        messageHeader_.kind1_ = kind1_;
        messageHeader_.kind2_ = kind2_;
    }
    
    DCS_Message_GetNewLeaseRequest::~DCS_Message_GetNewLeaseRequest()
    {
        SMPTE_SYNC_LOG_LEVEL(trace) << "DCS_Message_GetNewLeaseRequest::~DCS_Message_GetNewLeaseRequest";
    }
    
    int32_t DCS_Message_GetNewLeaseRequest::GetPayloadSize(void)
    {
        int32_t payloadSize = 0;
        
        payloadSize += static_cast<int32_t>(sizeof(requestID_));
        payloadSize += static_cast<int32_t>(sizeof(leaseDuration_));
        
        return payloadSize;
    }
    
    void DCS_Message_GetNewLeaseRequest::ReadPayload(uint8_t **iCommandBuffer)
    {
        SMPTE_SYNC_LOG_LEVEL(trace) << "DCS_Message_GetNewLeaseRequest::ReadPayload";
        Read(iCommandBuffer, requestID_);
        Read(iCommandBuffer, leaseDuration_);
    }

    void DCS_Message_GetNewLeaseRequest::WritePayload(uint8_t **iCommandBuffer)
    {
        SMPTE_SYNC_LOG_LEVEL(trace) << "DCS_Message_GetNewLeaseRequest::WritePayload";
        if (messageHeader_.length_ > 0)
        {
            Write(iCommandBuffer, requestID_);
            Write(iCommandBuffer, leaseDuration_);
        }
    }
    
    uint32_t DCS_Message_GetNewLeaseRequest::GetRequestID(void)
    {
        return requestID_;
    }

    void DCS_Message_GetNewLeaseRequest::SetRequestID(uint32_t iID)
    {
        requestID_ = iID;
    }
    
    uint32_t DCS_Message_GetNewLeaseRequest::GetLeaseDuration(void)
    {
        return leaseDuration_;
    }

    void DCS_Message_GetNewLeaseRequest::SetLeaseDuration(uint32_t iLeaseDuration)
    {
        leaseDuration_ = iLeaseDuration;
    }

    const int8_t DCS_Message_GetNewLeaseResponse::kind1_ = 0x02;
    const int8_t DCS_Message_GetNewLeaseResponse::kind2_ = 0x03;

    DCS_Message_GetNewLeaseResponse::DCS_Message_GetNewLeaseResponse()
    {
        SMPTE_SYNC_LOG_LEVEL(trace) << "DCS_Message_GetNewLeaseRequest::DCS_Message_GetNewLeaseRequest";
        messageHeader_.kind1_ = kind1_;
        messageHeader_.kind2_ = kind2_;
    }
    
    DCS_Message_GetNewLeaseResponse::~DCS_Message_GetNewLeaseResponse()
    {
        SMPTE_SYNC_LOG_LEVEL(trace) << "DCS_Message_GetNewLeaseResponse::~DCS_Message_GetNewLeaseResponse";
    }

    
    const int8_t DCS_Message_GetStatusRequest::kind1_ = 0x02;
    const int8_t DCS_Message_GetStatusRequest::kind2_ = 0x04;

    DCS_Message_GetStatusRequest::DCS_Message_GetStatusRequest()
    {
        SMPTE_SYNC_LOG_LEVEL(trace) << "DCS_Message_GetStatusRequest::DCS_Message_GetStatusRequest";
        messageHeader_.kind1_ = kind1_;
        messageHeader_.kind2_ = kind2_;
    }
    
    DCS_Message_GetStatusRequest::~DCS_Message_GetStatusRequest()
    {
        SMPTE_SYNC_LOG_LEVEL(trace) << "DCS_Message_GetStatusRequest::~DCS_Message_GetStatusRequest";
    }
    
    int32_t DCS_Message_GetStatusRequest::GetPayloadSize(void)
    {
        int32_t payloadSize = 0;
        
        payloadSize = static_cast<int32_t>(sizeof(requestID_));
        
        return payloadSize;
    }
    
    void DCS_Message_GetStatusRequest::ReadPayload(uint8_t **iCommandBuffer)
    {
        SMPTE_SYNC_LOG_LEVEL(trace) << "DCS_Message_GetStatusRequest::ReadPayload";
        Read(iCommandBuffer, requestID_);
    }
    
    void DCS_Message_GetStatusRequest::WritePayload(uint8_t **iCommandBuffer)
    {
        SMPTE_SYNC_LOG_LEVEL(trace) << "DCS_Message_GetStatusRequest::WritePayload";
        if (messageHeader_.length_ > 0)
        {
            Write(iCommandBuffer, requestID_);
        }
    }
    
    uint32_t DCS_Message_GetStatusRequest::GetRequestID(void)
    {
        return requestID_;
    }
    
    void DCS_Message_GetStatusRequest::SetRequestID(uint32_t iID)
    {
        requestID_ = iID;
    }
    
    const int8_t DCS_Message_GetStatusResponse::kind1_ = 0x02;
    const int8_t DCS_Message_GetStatusResponse::kind2_ = 0x05;

    DCS_Message_GetStatusResponse::DCS_Message_GetStatusResponse()
    {
        SMPTE_SYNC_LOG_LEVEL(trace) << "DCS_Message_GetStatusResponse::DCS_Message_GetStatusResponse";
        messageHeader_.kind1_ = kind1_;
        messageHeader_.kind2_ = kind2_;
    }
    
    DCS_Message_GetStatusResponse::~DCS_Message_GetStatusResponse()
    {
        SMPTE_SYNC_LOG_LEVEL(trace) << "DCS_Message_GetStatusResponse::~DCS_Message_GetStatusResponse";
    }
    
    
    const int8_t DCS_Message_SetRPLLocationRequest::kind1_ = 0x02;
    const int8_t DCS_Message_SetRPLLocationRequest::kind2_ = 0x06;

    DCS_Message_SetRPLLocationRequest::DCS_Message_SetRPLLocationRequest()
    {
        SMPTE_SYNC_LOG_LEVEL(trace) << "DCS_Message_SetRPLLocationRequest::DCS_Message_SetRPLLocationRequest";
        messageHeader_.kind1_ = kind1_;
        messageHeader_.kind2_ = kind2_;
    }
    
    DCS_Message_SetRPLLocationRequest::~DCS_Message_SetRPLLocationRequest()
    {
        SMPTE_SYNC_LOG_LEVEL(trace) << "DCS_Message_SetRPLLocationRequest::~DCS_Message_SetRPLLocationRequest";
    }
    
    int32_t DCS_Message_SetRPLLocationRequest::GetPayloadSize(void)
    {
        int32_t payloadSize = 0;
        
        payloadSize += static_cast<int32_t>(sizeof(requestID_));
        payloadSize += static_cast<int32_t>(sizeof(playoutID_));
        payloadSize += url_.length();
        
        return payloadSize;
    }
    
    void DCS_Message_SetRPLLocationRequest::ReadPayload(uint8_t **iCommandBuffer)
    {
        SMPTE_SYNC_LOG_LEVEL(trace) << "DCS_Message_SetRPLLocationRequest::ReadPayload";
        Read(iCommandBuffer, requestID_);
        Read(iCommandBuffer, playoutID_);

        // Use the payload size as determined in the messageHeader
        // rather than calling GetPayloadSize
        //
        int32_t payloadSize = messageHeader_.length_;

        // Compute the string length
        //
        int32_t strLen = payloadSize - ((static_cast<int32_t>(sizeof(requestID_)) + static_cast<int32_t>(sizeof(playoutID_))));
        if (strLen > 0)
            Read(iCommandBuffer, strLen, url_);
    }
    
    void DCS_Message_SetRPLLocationRequest::WritePayload(uint8_t **iCommandBuffer)
    {
        SMPTE_SYNC_LOG_LEVEL(trace) << "DCS_Message_SetRPLLocationRequest::WritePayload";
        if (messageHeader_.length_ > 0)
        {
            Write(iCommandBuffer, requestID_);
            Write(iCommandBuffer, playoutID_);
            if (url_.length() > 0)
                Write(iCommandBuffer, url_);
        }
    }
    
    uint32_t DCS_Message_SetRPLLocationRequest::GetRequestID(void)
    {
        return requestID_;
    }
    
    void DCS_Message_SetRPLLocationRequest::SetRequestID(uint32_t iID)
    {
        requestID_ = iID;
    }

    uint32_t DCS_Message_SetRPLLocationRequest::GetPlayoutID(void)
    {
        return playoutID_;
    }
    
    void DCS_Message_SetRPLLocationRequest::SetPlayoutID(uint32_t iID)
    {
        playoutID_ = iID;
    }

    std::string DCS_Message_SetRPLLocationRequest::GetResourceURL(void)
    {
        return url_;
    }
    
    void DCS_Message_SetRPLLocationRequest::SetResourceURL(std::string iURL)
    {
        url_ = iURL;
    }
    
    const int8_t DCS_Message_SetRPLLocationResponse::kind1_ = 0x02;
    const int8_t DCS_Message_SetRPLLocationResponse::kind2_ = 0x07;

    DCS_Message_SetRPLLocationResponse::DCS_Message_SetRPLLocationResponse()
    {
        SMPTE_SYNC_LOG_LEVEL(trace) << "DCS_Message_SetRPLLocationResponse::DCS_Message_SetRPLLocationResponse";
        messageHeader_.kind1_ = kind1_;
        messageHeader_.kind2_ = kind2_;
    }
    
    DCS_Message_SetRPLLocationResponse::~DCS_Message_SetRPLLocationResponse()
    {
        SMPTE_SYNC_LOG_LEVEL(trace) << "DCS_Message_SetRPLLocationResponse::~DCS_Message_SetRPLLocationResponse";
    }


    const int8_t DCS_Message_UpdateTimelineRequest::kind1_ = 0x02;
    const int8_t DCS_Message_UpdateTimelineRequest::kind2_ = 0x0A;
    
    DCS_Message_UpdateTimelineRequest::DCS_Message_UpdateTimelineRequest()
    {
        SMPTE_SYNC_LOG_LEVEL(trace) << "DCS_Message_UpdateTimelineRequest::DCS_Message_UpdateTimelineRequest";
        messageHeader_.kind1_ = kind1_;
        messageHeader_.kind2_ = kind2_;
    }
    
    DCS_Message_UpdateTimelineRequest::~DCS_Message_UpdateTimelineRequest()
    {
        SMPTE_SYNC_LOG_LEVEL(trace) << "DCS_Message_UpdateTimelineRequest::~DCS_Message_UpdateTimelineRequest";
    }
    
    int32_t DCS_Message_UpdateTimelineRequest::GetPayloadSize(void)
    {
        int32_t payloadSize = 0;
        
        payloadSize += static_cast<int32_t>(sizeof(requestID_));
        payloadSize += static_cast<int32_t>(sizeof(playoutID_));
        payloadSize += static_cast<int32_t>(sizeof(timelinePosition_));
        payloadSize += static_cast<int32_t>(sizeof(editRateNumerator_));
        payloadSize += static_cast<int32_t>(sizeof(editRateDenominator_));
        payloadSize += static_cast<int32_t>(sizeof(timelineExtensionCount_));
        payloadSize += timelineExtensions_.size();
        
        return payloadSize;
    }
    
    void DCS_Message_UpdateTimelineRequest::ReadPayload(uint8_t **iCommandBuffer)
    {
        SMPTE_SYNC_LOG_LEVEL(trace) << "DCS_Message_UpdateTimelineRequest::ReadPayload";

        Read(iCommandBuffer, requestID_);
        Read(iCommandBuffer, playoutID_);
        Read(iCommandBuffer, timelinePosition_);
        Read(iCommandBuffer, editRateNumerator_);
        Read(iCommandBuffer, editRateDenominator_);

        Read(iCommandBuffer, timelineExtensionCount_);

        for (int32_t i = 0; i < timelineExtensionCount_; i++)
        {
            TimelineExtension extension;
            if (extension.Read(iCommandBuffer))
                timelineExtensions_.push_back(extension);
        }
    }
    
    void DCS_Message_UpdateTimelineRequest::WritePayload(uint8_t **iCommandBuffer)
    {
        SMPTE_SYNC_LOG_LEVEL(trace) << "DCS_Message_UpdateTimelineRequest::WritePayload";
        if (messageHeader_.length_ > 0)
        {
            Write(iCommandBuffer, requestID_);
            Write(iCommandBuffer, playoutID_);
            Write(iCommandBuffer, timelinePosition_);
            Write(iCommandBuffer, editRateNumerator_);
            Write(iCommandBuffer, editRateDenominator_);
            
            Write(iCommandBuffer, timelineExtensionCount_);
            
            for (int32_t i = 0; i < timelineExtensionCount_; i++)
            {
                timelineExtensions_[i].Write(iCommandBuffer);
            }
        }
    }
    
    uint32_t DCS_Message_UpdateTimelineRequest::GetRequestID(void)
    {
        return requestID_;
    }
    
    void DCS_Message_UpdateTimelineRequest::SetRequestID(uint32_t iID)
    {
        requestID_ = iID;
    }
    
    uint32_t DCS_Message_UpdateTimelineRequest::GetPlayoutID(void)
    {
        return playoutID_;
    }
    
    void DCS_Message_UpdateTimelineRequest::SetPlayoutID(uint32_t iID)
    {
        playoutID_ = iID;
    }
    
    uint64_t DCS_Message_UpdateTimelineRequest::GetTinelinePosition(void)
    {
        return timelinePosition_;
    }
    
    void DCS_Message_UpdateTimelineRequest::SetTinelinePosition(uint64_t iTimelinePosition)
    {
        timelinePosition_ = iTimelinePosition;
    }
    
    uint64_t DCS_Message_UpdateTimelineRequest::GetEditRateNumerator(void)
    {
        return editRateNumerator_;
    }
    
    void DCS_Message_UpdateTimelineRequest::SetEditRateNumerator(uint64_t iRate)
    {
        editRateNumerator_ = iRate;
    }
    
    uint64_t DCS_Message_UpdateTimelineRequest::GetEditRateDenominator(void)
    {
        return editRateDenominator_;
    }
    
    void DCS_Message_UpdateTimelineRequest::SetEditRateDenominator(uint64_t iRate)
    {
        editRateDenominator_ = iRate;
    }
    
    std::vector<TimelineExtension> DCS_Message_UpdateTimelineRequest::GetTimelineExtensions(void)
    {
        return timelineExtensions_;
    }
    
    void DCS_Message_UpdateTimelineRequest::AddTimelineExtensions(TimelineExtension iTimelineExtension)
    {
        timelineExtensions_.push_back(iTimelineExtension);
        timelineExtensionCount_ = static_cast<uint32_t>(timelineExtensions_.size());
    }

    void DCS_Message_UpdateTimelineRequest::ClearTimelineExtensions(void)
    {
        timelineExtensions_.clear();
        timelineExtensionCount_ = 0;
    }

    
    const int8_t DCS_Message_UpdateTimelineResponse::kind1_ = 0x02;
    const int8_t DCS_Message_UpdateTimelineResponse::kind2_ = 0x0B;
    
    DCS_Message_UpdateTimelineResponse::DCS_Message_UpdateTimelineResponse()
    {
        SMPTE_SYNC_LOG_LEVEL(trace) << "DCS_Message_UpdateTimelineResponse::DCS_Message_UpdateTimelineResponse";
        messageHeader_.kind1_ = kind1_;
        messageHeader_.kind2_ = kind2_;
    }
    
    DCS_Message_UpdateTimelineResponse::~DCS_Message_UpdateTimelineResponse()
    {
        SMPTE_SYNC_LOG_LEVEL(trace) << "DCS_Message_UpdateTimelineResponse::~DCS_Message_UpdateTimelineResponse";
    }

    
    const int8_t DCS_Message_SetOutputModeRequest::kind1_ = 0x02;
    const int8_t DCS_Message_SetOutputModeRequest::kind2_ = 0x08;
    
    DCS_Message_SetOutputModeRequest::DCS_Message_SetOutputModeRequest()
    {
        SMPTE_SYNC_LOG_LEVEL(trace) << "DCS_Message_SetOutputModeRequest::DCS_Message_SetOutputModeRequest";
        messageHeader_.kind1_ = kind1_;
        messageHeader_.kind2_ = kind2_;
    }
    
    DCS_Message_SetOutputModeRequest::~DCS_Message_SetOutputModeRequest()
    {
        SMPTE_SYNC_LOG_LEVEL(trace) << "DCS_Message_SetOutputModeRequest::~DCS_Message_SetOutputModeRequest";
    }
    
    int32_t DCS_Message_SetOutputModeRequest::GetPayloadSize(void)
    {
        int32_t payloadSize = 0;
        
        payloadSize += static_cast<int32_t>(sizeof(requestID_));
        payloadSize += static_cast<int32_t>(sizeof(uint8_t)); // outputMode_ - we write this as a char
        
        return payloadSize;
    }
    
    void DCS_Message_SetOutputModeRequest::ReadPayload(uint8_t **iCommandBuffer)
    {
        SMPTE_SYNC_LOG_LEVEL(trace) << "DCS_Message_SetOutputModeRequest::ReadPayload";
        
        Read(iCommandBuffer, requestID_);
        Read(iCommandBuffer, outputMode_);
    }
    
    void DCS_Message_SetOutputModeRequest::WritePayload(uint8_t **iCommandBuffer)
    {
        SMPTE_SYNC_LOG_LEVEL(trace) << "DCS_Message_SetOutputModeRequest::WritePayload";
        if (messageHeader_.length_ > 0)
        {
            Write(iCommandBuffer, requestID_);
            Write(iCommandBuffer, outputMode_);
        }
    }
    
    uint32_t DCS_Message_SetOutputModeRequest::GetRequestID(void)
    {
        return requestID_;
    }
    
    void DCS_Message_SetOutputModeRequest::SetRequestID(uint32_t iID)
    {
        requestID_ = iID;
    }

    bool DCS_Message_SetOutputModeRequest::GetOutputMode(void)
    {
        return outputMode_;
    }
    
    void DCS_Message_SetOutputModeRequest::SetOutputMode(bool iEnable)
    {
        outputMode_ = iEnable;
    }

    const int8_t DCS_Message_SetOutputModeResponse::kind1_ = 0x02;
    const int8_t DCS_Message_SetOutputModeResponse::kind2_ = 0x09;
    
    DCS_Message_SetOutputModeResponse::DCS_Message_SetOutputModeResponse()
    {
        SMPTE_SYNC_LOG_LEVEL(trace) << "DCS_Message_SetOutputModeResponse::DCS_Message_SetOutputModeResponse";
        messageHeader_.kind1_ = kind1_;
        messageHeader_.kind2_ = kind2_;
    }
    
    DCS_Message_SetOutputModeResponse::~DCS_Message_SetOutputModeResponse()
    {
        SMPTE_SYNC_LOG_LEVEL(trace) << "DCS_Message_SetOutputModeResponse::~DCS_Message_SetOutputModeResponse";
    }


    const int8_t DCS_Message_TerminateLeaseRequest::kind1_ = 0x02;
    const int8_t DCS_Message_TerminateLeaseRequest::kind2_ = 0x0C;
    
    DCS_Message_TerminateLeaseRequest::DCS_Message_TerminateLeaseRequest()
    {
        SMPTE_SYNC_LOG_LEVEL(trace) << "DCS_Message_TerminateLeaseRequest::DCS_Message_TerminateLeaseRequest";
        messageHeader_.kind1_ = kind1_;
        messageHeader_.kind2_ = kind2_;
    }
    
    DCS_Message_TerminateLeaseRequest::~DCS_Message_TerminateLeaseRequest()
    {
        SMPTE_SYNC_LOG_LEVEL(trace) << "DCS_Message_TerminateLeaseRequest::~DCS_Message_TerminateLeaseRequest";
    }
    
    int32_t DCS_Message_TerminateLeaseRequest::GetPayloadSize(void)
    {
        int32_t payloadSize = 0;
        
        payloadSize += static_cast<int32_t>(sizeof(requestID_));
        
        return payloadSize;
    }
    
    void DCS_Message_TerminateLeaseRequest::ReadPayload(uint8_t **iCommandBuffer)
    {
        SMPTE_SYNC_LOG_LEVEL(trace) << "DCS_Message_TerminateLeaseRequest::ReadPayload";
        Read(iCommandBuffer, requestID_);
    }
    
    void DCS_Message_TerminateLeaseRequest::WritePayload(uint8_t **iCommandBuffer)
    {
        SMPTE_SYNC_LOG_LEVEL(trace) << "DCS_Message_TerminateLeaseRequest::WritePayload";
        if (messageHeader_.length_ > 0)
        {
            Write(iCommandBuffer, requestID_);
        }
    }
    
    uint32_t DCS_Message_TerminateLeaseRequest::GetRequestID(void)
    {
        return requestID_;
    }
    
    void DCS_Message_TerminateLeaseRequest::SetRequestID(uint32_t iID)
    {
        requestID_ = iID;
    }
    const int8_t DCS_Message_TerminateLeaseResponse::kind1_ = 0x02;
    const int8_t DCS_Message_TerminateLeaseResponse::kind2_ = 0x0D;
    
    DCS_Message_TerminateLeaseResponse::DCS_Message_TerminateLeaseResponse()
    {
        SMPTE_SYNC_LOG_LEVEL(trace) << "DCS_Message_TerminateLeaseResponse::DCS_Message_TerminateLeaseResponse";
        messageHeader_.kind1_ = kind1_;
        messageHeader_.kind2_ = kind2_;
    }
    
    DCS_Message_TerminateLeaseResponse::~DCS_Message_TerminateLeaseResponse()
    {
        SMPTE_SYNC_LOG_LEVEL(trace) << "DCS_Message_TerminateLeaseResponse::~DCS_Message_TerminateLeaseResponse";
    }

    
    const int8_t DCS_Message_GetLogEventListRequest::kind1_ = 0x02;
    const int8_t DCS_Message_GetLogEventListRequest::kind2_ = 0x10;
    
    DCS_Message_GetLogEventListRequest::DCS_Message_GetLogEventListRequest()
    {
        SMPTE_SYNC_LOG_LEVEL(trace) << "DCS_Message_GetLogEventListRequest::DCS_Message_GetLogEventListRequest";
        messageHeader_.kind1_ = kind1_;
        messageHeader_.kind2_ = kind2_;
    }
    
    DCS_Message_GetLogEventListRequest::~DCS_Message_GetLogEventListRequest()
    {
        SMPTE_SYNC_LOG_LEVEL(trace) << "DCS_Message_GetLogEventListRequest::~DCS_Message_GetLogEventListRequest";
    }
    
    int32_t DCS_Message_GetLogEventListRequest::GetPayloadSize(void)
    {
        int32_t payloadSize = 0;
        
        payloadSize += static_cast<int32_t>(sizeof(requestID_));
        payloadSize += static_cast<int32_t>(sizeof(timeStart_));
        payloadSize += static_cast<int32_t>(sizeof(timeStop_));
        
        return payloadSize;
    }
    
    void DCS_Message_GetLogEventListRequest::ReadPayload(uint8_t **iCommandBuffer)
    {
        SMPTE_SYNC_LOG_LEVEL(trace) << "DCS_Message_GetLogEventListRequest::ReadPayload";
        Read(iCommandBuffer, requestID_);
        Read(iCommandBuffer, timeStart_);
        Read(iCommandBuffer, timeStop_);
    }
    
    void DCS_Message_GetLogEventListRequest::WritePayload(uint8_t **iCommandBuffer)
    {
        SMPTE_SYNC_LOG_LEVEL(trace) << "DCS_Message_GetLogEventListRequest::WritePayload";
        if (messageHeader_.length_ > 0)
        {
            Write(iCommandBuffer, requestID_);
            Write(iCommandBuffer, timeStart_);
            Write(iCommandBuffer, timeStop_);
        }
    }
    
    uint32_t DCS_Message_GetLogEventListRequest::GetRequestID(void)
    {
        return requestID_;
    }
    
    void DCS_Message_GetLogEventListRequest::SetRequestID(uint32_t iID)
    {
        requestID_ = iID;
    }
    
    int64_t DCS_Message_GetLogEventListRequest::GetTimeStart(void)
    {
        return timeStart_;
    }
    
    void DCS_Message_GetLogEventListRequest::SetTimeStart(int64_t iTime)
    {
        timeStart_ = iTime;
    }
    
    int64_t DCS_Message_GetLogEventListRequest::GetTimeStop(void)
    {
        return timeStop_;
    }
    
    void DCS_Message_GetLogEventListRequest::SetTimeStop(int64_t iTime)
    {
        timeStop_ = iTime;
    }

    
    const int8_t DCS_Message_GetLogEventListResponse::kind1_ = 0x02;
    const int8_t DCS_Message_GetLogEventListResponse::kind2_ = 0x11;
    
    DCS_Message_GetLogEventListResponse::DCS_Message_GetLogEventListResponse()
    {
        SMPTE_SYNC_LOG_LEVEL(trace) << "DCS_Message_GetLogEventListResponse::DCS_Message_GetLogEventListResponse";
        messageHeader_.kind1_ = kind1_;
        messageHeader_.kind2_ = kind2_;
        itemLength_ = 4;
    }
    
    DCS_Message_GetLogEventListResponse::~DCS_Message_GetLogEventListResponse()
    {
        SMPTE_SYNC_LOG_LEVEL(trace) << "DCS_Message_GetLogEventListResponse::~DCS_Message_GetLogEventListResponse";
    }

    
    int32_t DCS_Message_GetLogEventListResponse::GetPayloadSize(void)
    {
        int32_t payloadSize = 0;
        
        payloadSize += static_cast<int32_t>(sizeof(requestID_));
        payloadSize += static_cast<int32_t>(sizeof(numberOfItems_));
        payloadSize += static_cast<int32_t>(sizeof(itemLength_));
        payloadSize += static_cast<int32_t>(eventIDs_.size() * itemLength_);
        payloadSize += statusResponse_.size();
        
        return payloadSize;
    }
    
    ResponseKey DCS_Message_GetLogEventListResponse::GetResponseKey(void)
    {
        assert(statusResponse_.key_ > eResponseKey_OutOfRange && statusResponse_.key_ < eResponseKey_Reserved);
        
        return static_cast<ResponseKey>(statusResponse_.key_);
    }
    
    void DCS_Message_GetLogEventListResponse::SetResponseKey(ResponseKey iKey)
    {
        assert(iKey > eResponseKey_OutOfRange && iKey < eResponseKey_Reserved);
        
        statusResponse_.key_ = iKey;
    }
    
    std::string DCS_Message_GetLogEventListResponse::GetResponseText(void)
    {
        return statusResponse_.text_;
    }
    
    void DCS_Message_GetLogEventListResponse::SetResponseText(std::string iVal)
    {
        statusResponse_.text_ = iVal;
        statusResponse_.length_ = static_cast<int32_t>(statusResponse_.text_.length());
    }
    
    void DCS_Message_GetLogEventListResponse::ReadPayload(uint8_t **iCommandBuffer)
    {
        SMPTE_SYNC_LOG_LEVEL(trace) << "DCS_Message_GetLogEventListResponse::ReadPayload";
        Read(iCommandBuffer, requestID_);
        Read(iCommandBuffer, numberOfItems_);
        ReadBER4(iCommandBuffer, itemLength_);

        for (int32_t i = 0; i < numberOfItems_; i++)
        {
            uint32_t id = 0;
            Read(iCommandBuffer, id);
            eventIDs_.push_back(id);
        }

        statusResponse_.Read(iCommandBuffer);
    }
    
    void DCS_Message_GetLogEventListResponse::WritePayload(uint8_t **iCommandBuffer)
    {
        SMPTE_SYNC_LOG_LEVEL(trace) << "DCS_Message_GetLogEventListResponse::WritePayload";
        if (messageHeader_.length_ > 0)
        {
            Write(iCommandBuffer, requestID_);
            Write(iCommandBuffer, numberOfItems_);
            WriteBER4(iCommandBuffer, itemLength_);

            for (int32_t i = 0; i < numberOfItems_; i++)
            {
                uint32_t id = eventIDs_[i];
                Write(iCommandBuffer, id);
            }

            statusResponse_.Write(iCommandBuffer);
        }
    }
    
    uint32_t DCS_Message_GetLogEventListResponse::GetRequestID(void)
    {
        return requestID_;
    }
    
    void DCS_Message_GetLogEventListResponse::SetRequestID(uint32_t iID)
    {
        requestID_ = iID;
    }
    
    uint32_t DCS_Message_GetLogEventListResponse::GetNumberOfItems(void)
    {
        return numberOfItems_;
    }
    
    void DCS_Message_GetLogEventListResponse::SetNumberOfItems(uint32_t iNumberOfItems)
    {
        numberOfItems_ = iNumberOfItems;
    }
    
    std::vector<uint32_t> DCS_Message_GetLogEventListResponse::GetEventIDs(void)
    {
        return eventIDs_;
    }
    
    void DCS_Message_GetLogEventListResponse::AddEventID(uint32_t iEventID)
    {
        eventIDs_.push_back(iEventID);
        numberOfItems_ = static_cast<uint32_t>(eventIDs_.size());
    }
    
    void DCS_Message_GetLogEventListResponse::ClearEventID(void)
    {
        eventIDs_.clear();
        numberOfItems_ = 0;
    }


    const int8_t DCS_Message_GetLogEventRequest::kind1_ = 0x02;
    const int8_t DCS_Message_GetLogEventRequest::kind2_ = 0x12;
    
    DCS_Message_GetLogEventRequest::DCS_Message_GetLogEventRequest()
    {
        SMPTE_SYNC_LOG_LEVEL(trace) << "DCS_Message_GetLogEventRequest::DCS_Message_GetLogEventRequest";
        messageHeader_.kind1_ = kind1_;
        messageHeader_.kind2_ = kind2_;
    }
    
    DCS_Message_GetLogEventRequest::~DCS_Message_GetLogEventRequest()
    {
        SMPTE_SYNC_LOG_LEVEL(trace) << "DCS_Message_GetLogEventRequest::~DCS_Message_GetLogEventRequest";
    }
    
    int32_t DCS_Message_GetLogEventRequest::GetPayloadSize(void)
    {
        int32_t payloadSize = 0;
        
        payloadSize += static_cast<int32_t>(sizeof(requestID_));
        payloadSize += static_cast<int32_t>(sizeof(eventID_));
        
        return payloadSize;
    }
    
    void DCS_Message_GetLogEventRequest::ReadPayload(uint8_t **iCommandBuffer)
    {
        SMPTE_SYNC_LOG_LEVEL(trace) << "DCS_Message_GetLogEventRequest::ReadPayload";
        Read(iCommandBuffer, requestID_);
        Read(iCommandBuffer, eventID_);
    }
    
    void DCS_Message_GetLogEventRequest::WritePayload(uint8_t **iCommandBuffer)
    {
        SMPTE_SYNC_LOG_LEVEL(trace) << "DCS_Message_GetLogEventRequest::WritePayload";
        if (messageHeader_.length_ > 0)
        {
            Write(iCommandBuffer, requestID_);
            Write(iCommandBuffer, eventID_);
        }
    }
    
    uint32_t DCS_Message_GetLogEventRequest::GetRequestID(void)
    {
        return requestID_;
    }
    
    void DCS_Message_GetLogEventRequest::SetRequestID(uint32_t iID)
    {
        requestID_ = iID;
    }

    uint32_t DCS_Message_GetLogEventRequest::GetEventID(void)
    {
        return eventID_;
    }
    
    void DCS_Message_GetLogEventRequest::SetEventID(uint32_t iID)
    {
        eventID_ = iID;
    }

    
    const int8_t DCS_Message_GetLogEventResponse::kind1_ = 0x02;
    const int8_t DCS_Message_GetLogEventResponse::kind2_ = 0x13;
    
    DCS_Message_GetLogEventResponse::DCS_Message_GetLogEventResponse()
    {
        SMPTE_SYNC_LOG_LEVEL(trace) << "DCS_Message_GetLogEventResponse::DCS_Message_GetLogEventResponse";
        messageHeader_.kind1_ = kind1_;
        messageHeader_.kind2_ = kind2_;
    }
    
    DCS_Message_GetLogEventResponse::~DCS_Message_GetLogEventResponse()
    {
        SMPTE_SYNC_LOG_LEVEL(trace) << "DCS_Message_GetLogEventResponse::~DCS_Message_GetLogEventResponse";
    }
    
    
    int32_t DCS_Message_GetLogEventResponse::GetPayloadSize(void)
    {
        int32_t payloadSize = 0;
        
        payloadSize += static_cast<int32_t>(sizeof(requestID_));
        payloadSize += static_cast<int32_t>(sizeof(logEventTextLength_));
        payloadSize += static_cast<int32_t>(logEventText_.length());
        payloadSize += statusResponse_.size();
        
        return payloadSize;
    }
    
    ResponseKey DCS_Message_GetLogEventResponse::GetResponseKey(void)
    {
        assert(statusResponse_.key_ > eResponseKey_OutOfRange && statusResponse_.key_ < eResponseKey_Reserved);
        
        return static_cast<ResponseKey>(statusResponse_.key_);
    }
    
    void DCS_Message_GetLogEventResponse::SetResponseKey(ResponseKey iKey)
    {
        assert(iKey > eResponseKey_OutOfRange && iKey < eResponseKey_Reserved);
        
        statusResponse_.key_ = iKey;
    }
    
    std::string DCS_Message_GetLogEventResponse::GetResponseText(void)
    {
        return statusResponse_.text_;
    }
    
    void DCS_Message_GetLogEventResponse::SetResponseText(std::string iVal)
    {
        statusResponse_.text_ = iVal;
        statusResponse_.length_ = static_cast<int32_t>(statusResponse_.text_.length());
    }
    
    void DCS_Message_GetLogEventResponse::ReadPayload(uint8_t **iCommandBuffer)
    {
        SMPTE_SYNC_LOG_LEVEL(trace) << "DCS_Message_GetLogEventResponse::ReadPayload";

        Read(iCommandBuffer, requestID_);
        ReadBER4(iCommandBuffer, logEventTextLength_);
        Read(iCommandBuffer, logEventTextLength_, logEventText_);

        statusResponse_.Read(iCommandBuffer);
    }
    
    void DCS_Message_GetLogEventResponse::WritePayload(uint8_t **iCommandBuffer)
    {
        SMPTE_SYNC_LOG_LEVEL(trace) << "DCS_Message_GetLogEventResponse::WritePayload";
        if (messageHeader_.length_ > 0)
        {
            Write(iCommandBuffer, requestID_);
            WriteBER4(iCommandBuffer, logEventTextLength_);
            Write(iCommandBuffer, logEventText_);

            statusResponse_.Write(iCommandBuffer);
        }
    }
    
    uint32_t DCS_Message_GetLogEventResponse::GetRequestID(void)
    {
        return requestID_;
    }
    
    void DCS_Message_GetLogEventResponse::SetRequestID(uint32_t iID)
    {
        requestID_ = iID;
    }
    
    int32_t DCS_Message_GetLogEventResponse::GetLogEventTextLength(void)
    {
        return logEventTextLength_;
    }
    
    void DCS_Message_GetLogEventResponse::SetLogEventTextLength(int32_t iLength)
    {
        logEventTextLength_ = iLength;
    }
    
    std::string DCS_Message_GetLogEventResponse::GetLogEventText(void)
    {
        return logEventText_;
    }
    
    void DCS_Message_GetLogEventResponse::SetLogEventText(std::string iText)
    {
        logEventText_ = iText;
    }
    

}  // namespace SMPTE_SYNC
