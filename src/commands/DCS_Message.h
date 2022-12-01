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

#ifndef DCS_MESSAGE_H
#define DCS_MESSAGE_H

#include <assert.h>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <vector>

#include "boost/shared_ptr.hpp"

#include "SerializationUtils.h"

namespace SMPTE_SYNC
{
    /**
     * @enum ResponseKey
     *
     * @brief Defines the possible values for the ResponseKey as defined by 
     * SMPTE ST 430-10:2010 D-Cinema Operations — Auxiliary Content Synchronization Protocol
     *
     */
    typedef enum ResponseKey {
        eResponseKey_OutOfRange = -1,
        eResponseKey_RRPSuccessful = 0,
        eResponseKey_RRPFailed = 1,
        eResponseKey_RRPInvalid = 2,
        eResponseKey_ACSBusy = 3,
        eResponseKey_LeaseTimeout = 4,
        eResponseKey_PlayoutIDMismatch = 5,
        eResponseKey_GeneralError = 6,
        eResponseKey_RecoverableError = 7,
        eResponseKey_RPLError = 8,
        eResponseKey_ResourceError = 9,
        eResponseKey_Processing = 10,
        eResponseKey_Reserved = 11,
    } ResponseKey;
    
    /**
     * @brief MessageHeader class implements the C++ object and serialization for the header for a DCS message as defined by
     * SMPTE ST 430-10:2010 D-Cinema Operations — Auxiliary Content Synchronization Protocol
     *
     */

    class MessageHeader
    {
    public:

        /**
         *
         * Writes the MessageHeader data to a buffer.
         * Advances the iCommandBuffer to the byte just past the last data member written
         *
         * @param iCommandBuffer is a buffer to write the MessageHeader into
         * @return true/false if the MessageHeader was successfully written
         *
         */
        bool Write(uint8_t **iCommandBuffer);

        /**
         *
         * Reads the MessageHeader data from a buffer.
         * Advances the iCommandBuffer to the byte just past the last data member read
         *
         * @param iCommandBuffer is a buffer from where the MessageHeader is read
         * @return true/false if the MessageHeader was successfully read
         *
         */
        bool Read(uint8_t **iCommandBuffer);
        
        int8_t      objectID_ = 0x06;
        int8_t      labelSize_ = 0x0E;
        int8_t      designator1_ = 0x2B;
        int8_t      designator2_ = 0x34;
        int8_t      registryCategoryDesignator_ = 0x02;
        int8_t      registryDesignator_ = 0x05;
        int8_t      structureDesignator_ = 0x01;
        int8_t      versionNumber_ = 0x01;
        int8_t      itemDesignator_ = 0x02;
        int8_t      organization_ = 0x07;
        int8_t      application_ = 0x02;
        int8_t      kind1_ = 0x00;
        int8_t      kind2_ = 0x00;
        int8_t      reserved1_ = 0x00;
        int8_t      reserved2_ = 0x00;
        int8_t      reserved3_ = 0x00;

        /**
         *
         * The length value is technically not part of the header but for 
         * practical purposes and implementation, it is useful to consider the
         * length as part of the header when reading and writing the buffer.
         *
         */
        int32_t     length_ = 0;

        /**
         *
         * The header size including the length_ is 16 + 4 == 20
         *
         */
        static const int8_t      headerSize_ = 20;
    };

    /**
     * @brief KLV class implements the C++ object and serialization for the KLV values stored in a DCS_Message
     *
     * @struct KLV
     */

    struct KLV
    {
    public:

        /**
         *
         * Writes the MessageHeader data to a buffer.
         * Advances the iCommandBuffer to the byte just past the last data member written
         *
         * @param iCommandBuffer is a buffer to write the MessageHeader into
         * @return true/false if the MessageHeader was successfully written
         *
         */
        bool Write(uint8_t **iCommandBuffer);

        /**
         *
         * Reads the MessageHeader data from a buffer.
         * Advances the iCommandBuffer to the byte just past the last data member read
         *
         * @param iCommandBuffer is a buffer from where the MessageHeader is read
         * @return true/false if the MessageHeader was successfully read
         *
         */
        bool Read(uint8_t **iCommandBuffer);

        /**
         *
         * Computes the serialized size in bytes of a spefic KLV value.
         * This is 1 + 4 + the number of bytes/chars stored in text_
         *
         * @return size of the KLV when serialized
         *
         */
        int32_t size(void);
        
        int8_t          key_ = 0;
        int32_t         length_ = 0;
        std::string     text_ = "";
    };

    /**
     * @brief The StatusResponse is of KVL type
     *
     */
    typedef KLV StatusResponse;

    /**
     * @brief The TimelineExtension is of KVL type
     *
     */
    typedef KLV TimelineExtension;
    
    /**
     * @brief DCS_Message class implements the C++ object and serialization of all DCS_Messages
     * all DCS_Messages inherit from this base class
     * DCS_Messages are defined in SMPTE ST 430-10:2010 D-Cinema Operations — Auxiliary Content Synchronization Protocol
     *
     */

    class DCS_Message
    {
    public:

        /// Constructor
        DCS_Message();
        
        /**
         *
         * Destructor
         * Note the use of virtual as all messages inherit from DCS_Message and have virtual functions for 
         * polymorphic behavior to determine what type of object they are and methods to call. 
         * See the MessageFactory for additional details and usage
         *
         * @param iHeader is a MessageHeader
         *
         */
        virtual ~DCS_Message();
        
        /**
         *
         * Returns the header size in bytes which is fixed by MessageHeader::headerSize_
         *
         * @return size of the header when serialized
         *
         */
        static int32_t GetHeaderSize(void);

        /**
         *
         * Returns the size in bytes of the full message which includes the header size and payload size
         *
         * @return size of the message when serialized
         *
         */
        virtual int32_t GetMessageSize(void);

        /**
         *
         * Returns the size in bytes of the payload
         *
         * @return size of the payload when serialized
         *
         */
        virtual int32_t GetPayloadSize(void);

        /**
         *
         * Returns the Kind1 value of the message. The Message type is identified by the Kind1 and Kind2 bytes
         *
         * @return vallue of the Kind1 data
         *
         */
        virtual uint8_t GetKind1();

        /**
         *
         * Returns the Kind2 value of the message. The Message type is identified by the Kind1 and Kind2 bytes
         *
         * @return vallue of the Kind2 data
         *
         */
        virtual uint8_t GetKind2();
        
        /**
         *
         * Reads the header data of the MessageHeader into the object.
         * The actual implementation is to copy the data to the messageHeader_
         *
         * @param iHeader is a MessageHeader
         *
         */
        virtual void ReadHeader(const MessageHeader &iHeader);

        /**
         *
         * Reads the header data from a buffer into the object.
         * The actual implementation is to call read on the messageHeader_
         *
         * @param iCommandBuffer is a buffer of data
         *
         */
        virtual void ReadHeader(uint8_t **iCommandBuffer);
        
        /**
         *
         * Reads the DCS_Message payload. The base class has no implementation.
         * Derived classes provide a specific implementation for their specific data.
         *
         * @param iCommandBuffer is a buffer of data
         *
         */
        virtual void ReadPayload(uint8_t **iCommandBuffer);

        /**
         *
         * Writes the header data into a buffer.
         * The length is calculated by calling GetPayloadSize and setting the messageHeader_.length_ to this value
         * and then calling messageHeader_.Write to serialize the header
         *
         * @param iCommandBuffer is a buffer of data
         *
         */
        virtual void WriteHeader(uint8_t **iCommandBuffer);

        /**
         *
         * Writes the DCS_Message payload. The base class has no implementation.
         * Derived classes provide a specific implementation for their specific data.
         *
         * @param iCommandBuffer is a buffer of data
         *
         */
        virtual void WritePayload(uint8_t **iCommandBuffer);

        /**
         *
         * Gets a copy of the MessageHeader.
         * Used in unit testing
         *
         * @return const reference to a MessageHeader object
         *
         */
        virtual const MessageHeader& GetMessageHeader(void);

    protected:
        
        /// The header that is common and required by add DCS_Message objects
        MessageHeader       messageHeader_;
    };

    /**
     * @brief All DCS_Message objects are wrapped in a boost::share_ptr to provide automatic reference counting and memory management.
     * The DCS_Messages are sent using boost::asio and its queuing system. This typicall requires reference counted objects that 
     * are automatically deleted when the boost::asio system is finished sending or receiving them.
     *
     */
    typedef boost::shared_ptr<DCS_Message> DCS_Message_Ptr;

    /**
     * @brief DCS_Message_BasicResponse class provides a shared implementation for certain trivial response messages to use.
     *
     */

    class DCS_Message_BasicResponse : public DCS_Message
    {
    public:

        /// Constructor
        DCS_Message_BasicResponse();

        /// Destructor
        virtual ~DCS_Message_BasicResponse();
        
        /**
         *
         * Computes and returns the size of a serialized DCS_Message_BasicResponse
         *
         * @return size of the serialized DCS_Message_BasicResponse message
         *
         */
        virtual int32_t GetPayloadSize(void);
        
        /// Gets the ResponseKey for this message
        virtual ResponseKey GetResponseKey(void);

        /// Sets the ResponseKey for this message
        virtual void SetResponseKey(ResponseKey iKey);
        
        /// Gets the response text for this message
        virtual std::string GetResponseText(void);

        /// Sets the response text for this message
        virtual void SetResponseText(std::string iVal);

        /**
         *
         * Reads the DCS_Message_BasicResponse specific payload
         *
         * @param iCommandBuffer is a buffer of data
         *
         */
        virtual void ReadPayload(uint8_t **iCommandBuffer);

        /**
         *
         * Writes the DCS_Message_BasicResponse specific payload
         *
         * @param iCommandBuffer is a buffer of data
         *
         */
        virtual void WritePayload(uint8_t **iCommandBuffer);
        
        /// Gets the Request ID of the message
        virtual uint32_t GetRequestID(void);

        /// Sets the Request ID of the message
        virtual void SetRequestID(uint32_t iID);
        
    private:

        /// Stores the specific request ID of this message. Generated by the server, returned by the client
        uint32_t        requestID_;
        
        /// Stores the response code of the result of this message
        StatusResponse  statusResponse_;
    };

    class DCS_Message_AnnounceRequest : public DCS_Message
    {
    public:
        static const int8_t kind1_;
        static const int8_t kind2_;
        
        DCS_Message_AnnounceRequest();
        virtual ~DCS_Message_AnnounceRequest();
        
        virtual int32_t GetPayloadSize(void);
        
        virtual void ReadPayload(uint8_t **iCommandBuffer);
        virtual void WritePayload(uint8_t **iCommandBuffer);

        virtual uint32_t GetRequestID(void);
        virtual void SetRequestID(uint32_t iID);

        virtual int64_t GetCurrentTime(void);
        virtual void SetCurrentTime(int64_t iCurrentTime);

        virtual std::string GetDeviceDescription(void);
        virtual void SetDeviceDescription(std::string iStr);

    private:
        uint32_t    requestID_;
        int64_t     currentTime_;
        std::string deviceDescription_;
    };

    class DCS_Message_AnnounceResponse : public DCS_Message
    {
    public:
        static const int8_t kind1_;
        static const int8_t kind2_;

        DCS_Message_AnnounceResponse();
        virtual ~DCS_Message_AnnounceResponse();
        
        virtual int32_t GetPayloadSize(void);
        
        virtual ResponseKey GetResponseKey(void);
        virtual void SetResponseKey(ResponseKey iKey);

        virtual std::string GetResponseText(void);
        virtual void SetResponseText(std::string iVal);

        virtual void ReadPayload(uint8_t **iCommandBuffer);
        virtual void WritePayload(uint8_t **iCommandBuffer);
        
        virtual uint32_t GetRequestID(void);
        virtual void SetRequestID(uint32_t iID);
        
        virtual int64_t GetCurrentTime(void);
        virtual void SetCurrentTime(int64_t iCurrentTime);
        
        virtual std::string GetDeviceDescription(void);
        virtual void SetDeviceDescription(std::string iStr);

    private:
        uint32_t        requestID_;
        int64_t         currentTime_;
        int32_t         deviceDescriptionLength_;
        std::string     deviceDescription_;
        StatusResponse  statusResponse_;
    };

    class DCS_Message_GetNewLeaseRequest : public DCS_Message
    {
    public:
        static const int8_t kind1_;
        static const int8_t kind2_;

        DCS_Message_GetNewLeaseRequest();
        virtual ~DCS_Message_GetNewLeaseRequest();
        
        virtual int32_t GetPayloadSize(void);
        
        virtual void ReadPayload(uint8_t **iCommandBuffer);
        virtual void WritePayload(uint8_t **iCommandBuffer);
        
        virtual uint32_t GetRequestID(void);
        virtual void SetRequestID(uint32_t iID);
        
        virtual uint32_t GetLeaseDuration(void);
        virtual void SetLeaseDuration(uint32_t iLeaseDuration);
        
    private:
        uint32_t    requestID_;
        uint32_t    leaseDuration_;
    };
    
    class DCS_Message_GetNewLeaseResponse : public DCS_Message_BasicResponse
    {
    public:
        static const int8_t kind1_;
        static const int8_t kind2_;

        DCS_Message_GetNewLeaseResponse();
        virtual ~DCS_Message_GetNewLeaseResponse();
    };

    class DCS_Message_GetStatusRequest : public DCS_Message
    {
    public:
        static const int8_t kind1_;
        static const int8_t kind2_;

        DCS_Message_GetStatusRequest();
        virtual ~DCS_Message_GetStatusRequest();
        
        virtual int32_t GetPayloadSize(void);
        
        virtual void ReadPayload(uint8_t **iCommandBuffer);
        virtual void WritePayload(uint8_t **iCommandBuffer);
        
        virtual uint32_t GetRequestID(void);
        virtual void SetRequestID(uint32_t iID);
        
    private:
        uint32_t    requestID_;
    };
    
    class DCS_Message_GetStatusResponse : public DCS_Message_BasicResponse
    {
    public:
        static const int8_t kind1_;
        static const int8_t kind2_;

        DCS_Message_GetStatusResponse();
        virtual ~DCS_Message_GetStatusResponse();
    };

    class DCS_Message_SetRPLLocationRequest : public DCS_Message
    {
    public:
        static const int8_t kind1_;
        static const int8_t kind2_;

        DCS_Message_SetRPLLocationRequest();
        virtual ~DCS_Message_SetRPLLocationRequest();
        
        virtual int32_t GetPayloadSize(void);
        
        virtual void ReadPayload(uint8_t **iCommandBuffer);
        virtual void WritePayload(uint8_t **iCommandBuffer);
        
        virtual uint32_t GetRequestID(void);
        virtual void SetRequestID(uint32_t iID);

        virtual uint32_t GetPlayoutID(void);
        virtual void SetPlayoutID(uint32_t iID);

        virtual std::string GetResourceURL(void);
        virtual void SetResourceURL(std::string iURL);

    private:
        uint32_t    requestID_;
        uint32_t    playoutID_;
        std::string url_;
    };
    
    class DCS_Message_SetRPLLocationResponse : public DCS_Message_BasicResponse
    {
    public:
        static const int8_t kind1_;
        static const int8_t kind2_;

        DCS_Message_SetRPLLocationResponse();
        virtual ~DCS_Message_SetRPLLocationResponse();
    };
    
    class DCS_Message_SetOutputModeRequest : public DCS_Message
    {
    public:
        static const int8_t kind1_;
        static const int8_t kind2_;
        
        DCS_Message_SetOutputModeRequest();
        virtual ~DCS_Message_SetOutputModeRequest();
        
        virtual int32_t GetPayloadSize(void);
        
        virtual void ReadPayload(uint8_t **iCommandBuffer);
        virtual void WritePayload(uint8_t **iCommandBuffer);
        
        virtual uint32_t GetRequestID(void);
        virtual void SetRequestID(uint32_t iID);
        
        virtual bool GetOutputMode(void);
        virtual void SetOutputMode(bool iEnable);
        
    private:
        uint32_t    requestID_;
        bool        outputMode_;
    };
    
    class DCS_Message_SetOutputModeResponse : public DCS_Message_BasicResponse
    {
    public:
        static const int8_t kind1_;
        static const int8_t kind2_;
        
        DCS_Message_SetOutputModeResponse();
        virtual ~DCS_Message_SetOutputModeResponse();
    };

    class DCS_Message_UpdateTimelineRequest : public DCS_Message
    {
    public:
        static const int8_t kind1_;
        static const int8_t kind2_;
        
        DCS_Message_UpdateTimelineRequest();
        virtual ~DCS_Message_UpdateTimelineRequest();
        
        virtual int32_t GetPayloadSize(void);
        
        virtual void ReadPayload(uint8_t **iCommandBuffer);
        virtual void WritePayload(uint8_t **iCommandBuffer);
        
        virtual uint32_t GetRequestID(void);
        virtual void SetRequestID(uint32_t iID);
        
        virtual uint32_t GetPlayoutID(void);
        virtual void SetPlayoutID(uint32_t iID);
        
        virtual uint64_t GetTinelinePosition(void);
        virtual void SetTinelinePosition(uint64_t iTimelinePosition);
        
        virtual uint64_t GetEditRateNumerator(void);
        virtual void SetEditRateNumerator(uint64_t iRate);
        
        virtual uint64_t GetEditRateDenominator(void);
        virtual void SetEditRateDenominator(uint64_t iRate);
        
        virtual std::vector<TimelineExtension> GetTimelineExtensions(void);
        virtual void AddTimelineExtensions(TimelineExtension iTimelineExtension);
        virtual void ClearTimelineExtensions(void);
        
    private:
        uint32_t    requestID_;
        uint32_t    playoutID_;
        
        uint64_t    timelinePosition_;
        
        uint64_t    editRateNumerator_;
        uint64_t    editRateDenominator_;
        
        uint32_t    timelineExtensionCount_;
        std::vector<TimelineExtension>  timelineExtensions_;
    };
    
    class DCS_Message_UpdateTimelineResponse : public DCS_Message_BasicResponse
    {
    public:
        static const int8_t kind1_;
        static const int8_t kind2_;
        
        DCS_Message_UpdateTimelineResponse();
        virtual ~DCS_Message_UpdateTimelineResponse();
    };

    class DCS_Message_TerminateLeaseRequest : public DCS_Message
    {
    public:
        static const int8_t kind1_;
        static const int8_t kind2_;
        
        DCS_Message_TerminateLeaseRequest();
        virtual ~DCS_Message_TerminateLeaseRequest();
        
        virtual int32_t GetPayloadSize(void);
        
        virtual void ReadPayload(uint8_t **iCommandBuffer);
        virtual void WritePayload(uint8_t **iCommandBuffer);
        
        virtual uint32_t GetRequestID(void);
        virtual void SetRequestID(uint32_t iID);
        
    private:
        uint32_t    requestID_;
    };
    
    class DCS_Message_TerminateLeaseResponse : public DCS_Message_BasicResponse
    {
    public:
        static const int8_t kind1_;
        static const int8_t kind2_;
        
        DCS_Message_TerminateLeaseResponse();
        virtual ~DCS_Message_TerminateLeaseResponse();
    };
    
    class DCS_Message_GetLogEventListRequest : public DCS_Message
    {
    public:
        static const int8_t kind1_;
        static const int8_t kind2_;
        
        DCS_Message_GetLogEventListRequest();
        virtual ~DCS_Message_GetLogEventListRequest();
        
        virtual int32_t GetPayloadSize(void);
        
        virtual void ReadPayload(uint8_t **iCommandBuffer);
        virtual void WritePayload(uint8_t **iCommandBuffer);
        
        virtual uint32_t GetRequestID(void);
        virtual void SetRequestID(uint32_t iID);

        virtual int64_t GetTimeStart(void);
        virtual void SetTimeStart(int64_t iTime);

        virtual int64_t GetTimeStop(void);
        virtual void SetTimeStop(int64_t iTime);

    private:
        uint32_t    requestID_;

        int64_t    timeStart_;
        int64_t    timeStop_;
    };
    
    class DCS_Message_GetLogEventListResponse : public DCS_Message
    {
    public:
        static const int8_t kind1_;
        static const int8_t kind2_;
        
        DCS_Message_GetLogEventListResponse();
        virtual ~DCS_Message_GetLogEventListResponse();

        virtual int32_t GetPayloadSize(void);
        
        virtual ResponseKey GetResponseKey(void);
        virtual void SetResponseKey(ResponseKey iKey);
        
        virtual std::string GetResponseText(void);
        virtual void SetResponseText(std::string iVal);
        
        virtual void ReadPayload(uint8_t **iCommandBuffer);
        virtual void WritePayload(uint8_t **iCommandBuffer);
        
        virtual uint32_t GetRequestID(void);
        virtual void SetRequestID(uint32_t iID);
        
        virtual uint32_t GetNumberOfItems(void);
        virtual void SetNumberOfItems(uint32_t iNumberOfItems);

        virtual std::vector<uint32_t> GetEventIDs(void);
        virtual void AddEventID(uint32_t iEventID);
        virtual void ClearEventID(void);

    private:
        uint32_t        requestID_;
        
        // Event ID Batch
        //
        uint32_t                numberOfItems_;
        int32_t                 itemLength_;
        std::vector<uint32_t>   eventIDs_;

        StatusResponse  statusResponse_;
    };
    
    class DCS_Message_GetLogEventRequest : public DCS_Message
    {
    public:
        static const int8_t kind1_;
        static const int8_t kind2_;
        
        DCS_Message_GetLogEventRequest();
        virtual ~DCS_Message_GetLogEventRequest();
        
        virtual int32_t GetPayloadSize(void);
        
        virtual void ReadPayload(uint8_t **iCommandBuffer);
        virtual void WritePayload(uint8_t **iCommandBuffer);
        
        virtual uint32_t GetRequestID(void);
        virtual void SetRequestID(uint32_t iID);

        virtual uint32_t GetEventID(void);
        virtual void SetEventID(uint32_t iID);

    private:
        uint32_t    requestID_;
        uint32_t    eventID_;
    };
    
    class DCS_Message_GetLogEventResponse : public DCS_Message
    {
    public:
        static const int8_t kind1_;
        static const int8_t kind2_;
        
        DCS_Message_GetLogEventResponse();
        virtual ~DCS_Message_GetLogEventResponse();
        
        virtual int32_t GetPayloadSize(void);
        
        virtual ResponseKey GetResponseKey(void);
        virtual void SetResponseKey(ResponseKey iKey);
        
        virtual std::string GetResponseText(void);
        virtual void SetResponseText(std::string iVal);
        
        virtual void ReadPayload(uint8_t **iCommandBuffer);
        virtual void WritePayload(uint8_t **iCommandBuffer);
        
        virtual uint32_t GetRequestID(void);
        virtual void SetRequestID(uint32_t iID);
        
        virtual int32_t GetLogEventTextLength(void);
        virtual void SetLogEventTextLength(int32_t iLength);
        
        virtual std::string GetLogEventText(void);
        virtual void SetLogEventText(std::string iText);
        
    private:
        uint32_t        requestID_;
        
        int32_t         logEventTextLength_;
        std::string     logEventText_;
        
        StatusResponse  statusResponse_;
    };

}  // namespace SMPTE_SYNC

#endif // DCS_MESSAGE_H
