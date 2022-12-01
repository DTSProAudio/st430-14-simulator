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

//
//  Server_BaseTest.cpp
//
//

#include "DCS_Message_Test.h"
#include "gtest/gtest.h"

#include "MessageFactory.h"
#include "DCS_Message.h"

using namespace SMPTE_SYNC;
using namespace std;

void TestHeader(DCS_Message *iWrite, DCS_Message *iRead)
{
    ASSERT_EQ(iWrite->GetHeaderSize(), iRead->GetHeaderSize());
    ASSERT_EQ(iWrite->GetMessageSize(), iRead->GetMessageSize());
    ASSERT_EQ(iWrite->GetPayloadSize(), iRead->GetPayloadSize());
    ASSERT_EQ(iWrite->GetKind1(), iRead->GetKind1());
    ASSERT_EQ(iWrite->GetKind2(), iRead->GetKind2());
    
    ASSERT_EQ(iWrite->GetMessageHeader().objectID_, iRead->GetMessageHeader().objectID_);
    ASSERT_EQ(iWrite->GetMessageHeader().labelSize_, iRead->GetMessageHeader().labelSize_);
    ASSERT_EQ(iWrite->GetMessageHeader().designator1_, iRead->GetMessageHeader().designator1_);
    ASSERT_EQ(iWrite->GetMessageHeader().designator2_, iRead->GetMessageHeader().designator2_);
    ASSERT_EQ(iWrite->GetMessageHeader().registryCategoryDesignator_, iRead->GetMessageHeader().registryCategoryDesignator_);
    ASSERT_EQ(iWrite->GetMessageHeader().registryDesignator_, iRead->GetMessageHeader().registryDesignator_);
    ASSERT_EQ(iWrite->GetMessageHeader().structureDesignator_, iRead->GetMessageHeader().structureDesignator_);
    ASSERT_EQ(iWrite->GetMessageHeader().versionNumber_, iRead->GetMessageHeader().versionNumber_);
    ASSERT_EQ(iWrite->GetMessageHeader().itemDesignator_, iRead->GetMessageHeader().itemDesignator_);
    ASSERT_EQ(iWrite->GetMessageHeader().organization_, iRead->GetMessageHeader().organization_);
    ASSERT_EQ(iWrite->GetMessageHeader().application_, iRead->GetMessageHeader().application_);
    ASSERT_EQ(iWrite->GetMessageHeader().kind1_, iRead->GetMessageHeader().kind1_);
    ASSERT_EQ(iWrite->GetMessageHeader().kind2_, iRead->GetMessageHeader().kind2_);
    ASSERT_EQ(iWrite->GetMessageHeader().reserved1_, iRead->GetMessageHeader().reserved1_);
    ASSERT_EQ(iWrite->GetMessageHeader().reserved2_, iRead->GetMessageHeader().reserved2_);
    ASSERT_EQ(iWrite->GetMessageHeader().reserved3_, iRead->GetMessageHeader().reserved3_);
    
    ASSERT_EQ(iWrite->GetMessageHeader().length_, iRead->GetMessageHeader().length_);
}

uint8_t *messageBuffer = nullptr;
int32_t messageBufferSize = MessageHeader::headerSize_ * 4;

void SetupMessageMemory(void)
{
    messageBuffer = new uint8_t[messageBufferSize];
    memset(messageBuffer, 0x0, messageBufferSize);
}

void TearDownMessageMemory(void)
{
    delete [] messageBuffer;
}

void CheckMessageMemoryForCommand(DCS_Message *iWrite)
{
    if (iWrite->GetMessageSize() > messageBufferSize)
    {
        delete [] messageBuffer;
        messageBufferSize = iWrite->GetMessageSize();
        messageBuffer = new uint8_t[messageBufferSize];
        memset(messageBuffer, 0x0, messageBufferSize);
    }
}

void TestAnnounceRequest(void)
{
    DCS_Message_AnnounceRequest writeMsg;
    writeMsg.SetCurrentTime(1);
    writeMsg.SetRequestID(1);
    writeMsg.SetDeviceDescription("description");

    CheckMessageMemoryForCommand(&writeMsg);

    uint8_t *tmpbuf = messageBuffer;
    writeMsg.WriteHeader(&tmpbuf);
    writeMsg.WritePayload(&tmpbuf);

    DCS_Message_AnnounceRequest readMsg;
    tmpbuf = messageBuffer;
    readMsg.ReadHeader(&tmpbuf);
    readMsg.ReadPayload(&tmpbuf);

    TestHeader(&writeMsg, &readMsg);

    ASSERT_EQ(writeMsg.GetRequestID(), readMsg.GetRequestID());
    ASSERT_EQ(writeMsg.GetCurrentTime(), readMsg.GetCurrentTime());
    ASSERT_EQ(writeMsg.GetDeviceDescription(), readMsg.GetDeviceDescription());
}

void TestAnnounceResponse(void)
{
    DCS_Message_AnnounceResponse writeMsg;
    writeMsg.SetCurrentTime(1);
    writeMsg.SetRequestID(1);
    writeMsg.SetDeviceDescription("description");
    writeMsg.SetResponseKey(eResponseKey_RRPSuccessful);
    writeMsg.SetResponseText("response");
    
    CheckMessageMemoryForCommand(&writeMsg);
    
    uint8_t *tmpbuf = messageBuffer;
    writeMsg.WriteHeader(&tmpbuf);
    writeMsg.WritePayload(&tmpbuf);
    
    DCS_Message_AnnounceResponse readMsg;
    tmpbuf = messageBuffer;
    readMsg.ReadHeader(&tmpbuf);
    readMsg.ReadPayload(&tmpbuf);
    
    TestHeader(&writeMsg, &readMsg);
    
    ASSERT_EQ(writeMsg.GetRequestID(), readMsg.GetRequestID());
    ASSERT_EQ(writeMsg.GetCurrentTime(), readMsg.GetCurrentTime());
    ASSERT_EQ(writeMsg.GetDeviceDescription(), readMsg.GetDeviceDescription());
    ASSERT_EQ(writeMsg.GetResponseKey(), readMsg.GetResponseKey());
    ASSERT_EQ(writeMsg.GetResponseText(), readMsg.GetResponseText());
}

void TestGetNewLeaseRequest(void)
{
    DCS_Message_GetNewLeaseRequest writeMsg;
    writeMsg.SetRequestID(1);
    writeMsg.SetLeaseDuration(100);
    
    CheckMessageMemoryForCommand(&writeMsg);
    
    uint8_t *tmpbuf = messageBuffer;
    writeMsg.WriteHeader(&tmpbuf);
    writeMsg.WritePayload(&tmpbuf);
    
    DCS_Message_GetNewLeaseRequest readMsg;
    tmpbuf = messageBuffer;
    readMsg.ReadHeader(&tmpbuf);
    readMsg.ReadPayload(&tmpbuf);
    
    TestHeader(&writeMsg, &readMsg);
    
    ASSERT_EQ(writeMsg.GetRequestID(), readMsg.GetRequestID());
    ASSERT_EQ(writeMsg.GetLeaseDuration(), readMsg.GetLeaseDuration());
}

void TestGetNewLeaseResponse(void)
{
    DCS_Message_GetNewLeaseResponse writeMsg;
    writeMsg.SetRequestID(1);
    writeMsg.SetResponseKey(eResponseKey_RRPSuccessful);
    writeMsg.SetResponseText("ok");
    
    CheckMessageMemoryForCommand(&writeMsg);
    
    uint8_t *tmpbuf = messageBuffer;
    writeMsg.WriteHeader(&tmpbuf);
    writeMsg.WritePayload(&tmpbuf);
    
    DCS_Message_GetNewLeaseResponse readMsg;
    tmpbuf = messageBuffer;
    readMsg.ReadHeader(&tmpbuf);
    readMsg.ReadPayload(&tmpbuf);
    
    TestHeader(&writeMsg, &readMsg);
    
    ASSERT_EQ(writeMsg.GetRequestID(), readMsg.GetRequestID());
    ASSERT_EQ(writeMsg.GetResponseKey(), readMsg.GetResponseKey());
    ASSERT_EQ(writeMsg.GetResponseText(), readMsg.GetResponseText());
}

void TestGetStatusRequest(void)
{
    DCS_Message_GetStatusRequest writeMsg;
    writeMsg.SetRequestID(1);
    
    CheckMessageMemoryForCommand(&writeMsg);
    
    uint8_t *tmpbuf = messageBuffer;
    writeMsg.WriteHeader(&tmpbuf);
    writeMsg.WritePayload(&tmpbuf);
    
    DCS_Message_GetStatusRequest readMsg;
    tmpbuf = messageBuffer;
    readMsg.ReadHeader(&tmpbuf);
    readMsg.ReadPayload(&tmpbuf);
    
    TestHeader(&writeMsg, &readMsg);
    
    ASSERT_EQ(writeMsg.GetRequestID(), readMsg.GetRequestID());
}

void TestGetStatusResponse(void)
{
    DCS_Message_GetStatusResponse writeMsg;
    writeMsg.SetRequestID(1);
    writeMsg.SetResponseKey(eResponseKey_RRPSuccessful);
    writeMsg.SetResponseText("ok");
    
    CheckMessageMemoryForCommand(&writeMsg);
    
    uint8_t *tmpbuf = messageBuffer;
    writeMsg.WriteHeader(&tmpbuf);
    writeMsg.WritePayload(&tmpbuf);
    
    DCS_Message_GetStatusResponse readMsg;
    tmpbuf = messageBuffer;
    readMsg.ReadHeader(&tmpbuf);
    readMsg.ReadPayload(&tmpbuf);
    
    TestHeader(&writeMsg, &readMsg);
    
    ASSERT_EQ(writeMsg.GetRequestID(), readMsg.GetRequestID());
    ASSERT_EQ(writeMsg.GetResponseKey(), readMsg.GetResponseKey());
    ASSERT_EQ(writeMsg.GetResponseText(), readMsg.GetResponseText());
}

void TestSetRPLLocationRequest(void)
{
    DCS_Message_SetRPLLocationRequest writeMsg;
    writeMsg.SetRequestID(1);
    writeMsg.SetPlayoutID(1);
    writeMsg.SetResourceURL("http:://www.helloworld.org");
    
    CheckMessageMemoryForCommand(&writeMsg);
    
    uint8_t *tmpbuf = messageBuffer;
    writeMsg.WriteHeader(&tmpbuf);
    writeMsg.WritePayload(&tmpbuf);
    
    DCS_Message_SetRPLLocationRequest readMsg;
    tmpbuf = messageBuffer;
    readMsg.ReadHeader(&tmpbuf);
    readMsg.ReadPayload(&tmpbuf);
    
    TestHeader(&writeMsg, &readMsg);
    
    ASSERT_EQ(writeMsg.GetRequestID(), readMsg.GetRequestID());
    ASSERT_EQ(writeMsg.GetPlayoutID(), readMsg.GetPlayoutID());
    ASSERT_EQ(writeMsg.GetResourceURL(), readMsg.GetResourceURL());
}

void TestSetRPLLocationResponse(void)
{
    DCS_Message_SetRPLLocationResponse writeMsg;
    writeMsg.SetRequestID(1);
    writeMsg.SetResponseKey(eResponseKey_RRPSuccessful);
    writeMsg.SetResponseText("ok");
    
    CheckMessageMemoryForCommand(&writeMsg);
    
    uint8_t *tmpbuf = messageBuffer;
    writeMsg.WriteHeader(&tmpbuf);
    writeMsg.WritePayload(&tmpbuf);
    
    DCS_Message_SetRPLLocationResponse readMsg;
    tmpbuf = messageBuffer;
    readMsg.ReadHeader(&tmpbuf);
    readMsg.ReadPayload(&tmpbuf);
    
    TestHeader(&writeMsg, &readMsg);
    
    ASSERT_EQ(writeMsg.GetRequestID(), readMsg.GetRequestID());
    ASSERT_EQ(writeMsg.GetResponseKey(), readMsg.GetResponseKey());
    ASSERT_EQ(writeMsg.GetResponseText(), readMsg.GetResponseText());
}

void TestSetOutputModeRequest(void)
{
    DCS_Message_SetOutputModeRequest writeMsg;
    writeMsg.SetRequestID(1);
    writeMsg.SetOutputMode(true);
    
    CheckMessageMemoryForCommand(&writeMsg);
    
    uint8_t *tmpbuf = messageBuffer;
    writeMsg.WriteHeader(&tmpbuf);
    writeMsg.WritePayload(&tmpbuf);
    
    DCS_Message_SetOutputModeRequest readMsg;
    tmpbuf = messageBuffer;
    readMsg.ReadHeader(&tmpbuf);
    readMsg.ReadPayload(&tmpbuf);
    
    TestHeader(&writeMsg, &readMsg);
    
    ASSERT_EQ(writeMsg.GetRequestID(), readMsg.GetRequestID());
    ASSERT_EQ(writeMsg.GetOutputMode(), readMsg.GetOutputMode());
}

void TestSetOutputModeResponse(void)
{
    DCS_Message_SetOutputModeResponse writeMsg;
    writeMsg.SetRequestID(1);
    writeMsg.SetResponseKey(eResponseKey_RRPSuccessful);
    writeMsg.SetResponseText("ok");
    
    CheckMessageMemoryForCommand(&writeMsg);
    
    uint8_t *tmpbuf = messageBuffer;
    writeMsg.WriteHeader(&tmpbuf);
    writeMsg.WritePayload(&tmpbuf);
    
    DCS_Message_SetOutputModeResponse readMsg;
    tmpbuf = messageBuffer;
    readMsg.ReadHeader(&tmpbuf);
    readMsg.ReadPayload(&tmpbuf);
    
    TestHeader(&writeMsg, &readMsg);
    
    ASSERT_EQ(writeMsg.GetRequestID(), readMsg.GetRequestID());
    ASSERT_EQ(writeMsg.GetResponseKey(), readMsg.GetResponseKey());
    ASSERT_EQ(writeMsg.GetResponseText(), readMsg.GetResponseText());
}

void TestUpdateTimelineRequest(void)
{
    DCS_Message_UpdateTimelineRequest writeMsg;
    writeMsg.SetRequestID(1);
    writeMsg.SetPlayoutID(1);
    writeMsg.SetTinelinePosition(1);
    writeMsg.SetEditRateNumerator(1);
    writeMsg.SetEditRateDenominator(1);
    
    TimelineExtension te;
    te.key_ = 1;
    te.key_ = 2;
    te.text_ = "ab";
    
    writeMsg.AddTimelineExtensions(te);
    
    CheckMessageMemoryForCommand(&writeMsg);
    
    uint8_t *tmpbuf = messageBuffer;
    writeMsg.WriteHeader(&tmpbuf);
    writeMsg.WritePayload(&tmpbuf);
    
    DCS_Message_UpdateTimelineRequest readMsg;
    tmpbuf = messageBuffer;
    readMsg.ReadHeader(&tmpbuf);
    readMsg.ReadPayload(&tmpbuf);
    
    TestHeader(&writeMsg, &readMsg);
    
    ASSERT_EQ(writeMsg.GetRequestID(), readMsg.GetRequestID());
    ASSERT_EQ(writeMsg.GetPlayoutID(), readMsg.GetPlayoutID());
    ASSERT_EQ(writeMsg.GetTinelinePosition(), readMsg.GetTinelinePosition());
    ASSERT_EQ(writeMsg.GetEditRateNumerator(), readMsg.GetEditRateNumerator());
    ASSERT_EQ(writeMsg.GetEditRateDenominator(), readMsg.GetEditRateDenominator());

    std::vector<TimelineExtension> writeTE = writeMsg.GetTimelineExtensions();
    std::vector<TimelineExtension> readTE = readMsg.GetTimelineExtensions();
    ASSERT_EQ(writeTE.size(), readTE.size());
    ASSERT_EQ(writeTE[0].key_, readTE[0].key_);
    ASSERT_EQ(writeTE[0].length_, readTE[0].length_);
    ASSERT_EQ(writeTE[0].text_, readTE[0].text_);
}

void TestUpdateTimelineResponse(void)
{
    DCS_Message_UpdateTimelineResponse writeMsg;
    writeMsg.SetRequestID(1);
    writeMsg.SetResponseKey(eResponseKey_RRPSuccessful);
    writeMsg.SetResponseText("ok");
    
    CheckMessageMemoryForCommand(&writeMsg);
    
    uint8_t *tmpbuf = messageBuffer;
    writeMsg.WriteHeader(&tmpbuf);
    writeMsg.WritePayload(&tmpbuf);
    
    DCS_Message_UpdateTimelineResponse readMsg;
    tmpbuf = messageBuffer;
    readMsg.ReadHeader(&tmpbuf);
    readMsg.ReadPayload(&tmpbuf);
    
    TestHeader(&writeMsg, &readMsg);
    
    ASSERT_EQ(writeMsg.GetRequestID(), readMsg.GetRequestID());
    ASSERT_EQ(writeMsg.GetResponseKey(), readMsg.GetResponseKey());
    ASSERT_EQ(writeMsg.GetResponseText(), readMsg.GetResponseText());
}

void TestTerminateLeaseRequest(void)
{
    DCS_Message_TerminateLeaseRequest writeMsg;
    writeMsg.SetRequestID(1);
    
    CheckMessageMemoryForCommand(&writeMsg);
    
    uint8_t *tmpbuf = messageBuffer;
    writeMsg.WriteHeader(&tmpbuf);
    writeMsg.WritePayload(&tmpbuf);
    
    DCS_Message_TerminateLeaseRequest readMsg;
    tmpbuf = messageBuffer;
    readMsg.ReadHeader(&tmpbuf);
    readMsg.ReadPayload(&tmpbuf);
    
    TestHeader(&writeMsg, &readMsg);
    
    ASSERT_EQ(writeMsg.GetRequestID(), readMsg.GetRequestID());
}

void TestTerminateLeaseResponse(void)
{
    DCS_Message_TerminateLeaseResponse writeMsg;
    writeMsg.SetRequestID(1);
    writeMsg.SetResponseKey(eResponseKey_RRPSuccessful);
    writeMsg.SetResponseText("ok");
    
    CheckMessageMemoryForCommand(&writeMsg);
    
    uint8_t *tmpbuf = messageBuffer;
    writeMsg.WriteHeader(&tmpbuf);
    writeMsg.WritePayload(&tmpbuf);
    
    DCS_Message_TerminateLeaseResponse readMsg;
    tmpbuf = messageBuffer;
    readMsg.ReadHeader(&tmpbuf);
    readMsg.ReadPayload(&tmpbuf);
    
    TestHeader(&writeMsg, &readMsg);
    
    ASSERT_EQ(writeMsg.GetRequestID(), readMsg.GetRequestID());
    ASSERT_EQ(writeMsg.GetResponseKey(), readMsg.GetResponseKey());
    ASSERT_EQ(writeMsg.GetResponseText(), readMsg.GetResponseText());
}

void TestGetLogEventListRequest(void)
{
    DCS_Message_GetLogEventListRequest writeMsg;
    writeMsg.SetRequestID(1);
    writeMsg.SetTimeStart(1);
    writeMsg.SetTimeStop(1);
    
    CheckMessageMemoryForCommand(&writeMsg);
    
    uint8_t *tmpbuf = messageBuffer;
    writeMsg.WriteHeader(&tmpbuf);
    writeMsg.WritePayload(&tmpbuf);
    
    DCS_Message_GetLogEventListRequest readMsg;
    tmpbuf = messageBuffer;
    readMsg.ReadHeader(&tmpbuf);
    readMsg.ReadPayload(&tmpbuf);
    
    TestHeader(&writeMsg, &readMsg);
    
    ASSERT_EQ(writeMsg.GetRequestID(), readMsg.GetRequestID());
    ASSERT_EQ(writeMsg.GetTimeStart(), readMsg.GetTimeStart());
    ASSERT_EQ(writeMsg.GetTimeStop(), readMsg.GetTimeStop());
}

void TestGetLogEventListResponse(void)
{
    DCS_Message_GetLogEventListResponse writeMsg;
    writeMsg.SetRequestID(1);
    writeMsg.SetResponseKey(eResponseKey_RRPSuccessful);
    writeMsg.SetResponseText("ok");
    writeMsg.SetNumberOfItems(1);
    writeMsg.AddEventID(1);
    
    CheckMessageMemoryForCommand(&writeMsg);
    
    uint8_t *tmpbuf = messageBuffer;
    writeMsg.WriteHeader(&tmpbuf);
    writeMsg.WritePayload(&tmpbuf);
    
    DCS_Message_GetLogEventListResponse readMsg;
    tmpbuf = messageBuffer;
    readMsg.ReadHeader(&tmpbuf);
    readMsg.ReadPayload(&tmpbuf);
    
    TestHeader(&writeMsg, &readMsg);
    
    ASSERT_EQ(writeMsg.GetRequestID(), readMsg.GetRequestID());
    ASSERT_EQ(writeMsg.GetResponseKey(), readMsg.GetResponseKey());
    ASSERT_EQ(writeMsg.GetNumberOfItems(), readMsg.GetNumberOfItems());

    std::vector<uint32_t> writeEID = writeMsg.GetEventIDs();
    std::vector<uint32_t> readEID = readMsg.GetEventIDs();
    ASSERT_EQ(writeEID.size(), readEID.size());
    ASSERT_EQ(writeEID[0], readEID[0]);
}

void TestGetLogEventRequest(void)
{
    DCS_Message_GetLogEventRequest writeMsg;
    writeMsg.SetRequestID(1);
    writeMsg.SetEventID(1);
    
    CheckMessageMemoryForCommand(&writeMsg);
    
    uint8_t *tmpbuf = messageBuffer;
    writeMsg.WriteHeader(&tmpbuf);
    writeMsg.WritePayload(&tmpbuf);
    
    DCS_Message_GetLogEventRequest readMsg;
    tmpbuf = messageBuffer;
    readMsg.ReadHeader(&tmpbuf);
    readMsg.ReadPayload(&tmpbuf);
    
    TestHeader(&writeMsg, &readMsg);
    
    ASSERT_EQ(writeMsg.GetRequestID(), readMsg.GetRequestID());
    ASSERT_EQ(writeMsg.GetEventID(), readMsg.GetEventID());
}

void TestGetLogEventResponse(void)
{
    DCS_Message_GetLogEventResponse writeMsg;
    writeMsg.SetRequestID(1);
    writeMsg.SetResponseKey(eResponseKey_RRPSuccessful);
    writeMsg.SetResponseText("ok");
    writeMsg.SetLogEventTextLength(2);
    writeMsg.SetLogEventText("ok");
    
    CheckMessageMemoryForCommand(&writeMsg);
    
    uint8_t *tmpbuf = messageBuffer;
    writeMsg.WriteHeader(&tmpbuf);
    writeMsg.WritePayload(&tmpbuf);
    
    DCS_Message_GetLogEventResponse readMsg;
    tmpbuf = messageBuffer;
    readMsg.ReadHeader(&tmpbuf);
    readMsg.ReadPayload(&tmpbuf);
    
    TestHeader(&writeMsg, &readMsg);
    
    ASSERT_EQ(writeMsg.GetRequestID(), readMsg.GetRequestID());
    ASSERT_EQ(writeMsg.GetResponseKey(), readMsg.GetResponseKey());
    ASSERT_EQ(writeMsg.GetLogEventTextLength(), readMsg.GetLogEventTextLength());
    ASSERT_EQ(writeMsg.GetLogEventText(), readMsg.GetLogEventText());
}


TEST(DCS_Message_Test, DCS_Message_Test_Case1)
{
    SetupMessageMemory();
    
    TestAnnounceRequest();
    TestAnnounceResponse();
    TestGetNewLeaseRequest();
    TestGetNewLeaseResponse();
    TestGetStatusRequest();
    TestGetStatusResponse();
    TestSetRPLLocationRequest();
    TestSetRPLLocationResponse();
    TestSetOutputModeRequest();
    TestSetOutputModeResponse();
    TestUpdateTimelineRequest();
    TestUpdateTimelineResponse();
    TestTerminateLeaseRequest();
    TestTerminateLeaseResponse();
    TestGetLogEventListRequest();
    TestGetLogEventListResponse();
    TestGetLogEventRequest();
    TestGetLogEventResponse();
    
    TearDownMessageMemory();
}

TEST(DCS_Message_Test, DCS_Message_Test_Case2)
{
    DCS_Message* msg = nullptr;

    MessageHeader msgHeader;
    
    msgHeader.kind1_ = DCS_Message_AnnounceRequest::kind1_;
    msgHeader.kind2_ = DCS_Message_AnnounceRequest::kind2_;
    
    msg = MessageFactory::CreateDCSMessage(msgHeader);
    DCS_Message_AnnounceRequest *msgAnnounceRequest = dynamic_cast<DCS_Message_AnnounceRequest*>(msg);
    ASSERT_NE(msgAnnounceRequest, nullptr);
    delete msg;
    
    msgHeader.kind1_ = DCS_Message_AnnounceResponse::kind1_;
    msgHeader.kind2_ = DCS_Message_AnnounceResponse::kind2_;
    
    msg = MessageFactory::CreateDCSMessage(msgHeader);
    DCS_Message_AnnounceResponse *msgAnnounceResponse = dynamic_cast<DCS_Message_AnnounceResponse*>(msg);
    ASSERT_NE(msgAnnounceResponse, nullptr);
    delete msg;

    msgHeader.kind1_ = DCS_Message_GetNewLeaseRequest::kind1_;
    msgHeader.kind2_ = DCS_Message_GetNewLeaseRequest::kind2_;
    
    msg = MessageFactory::CreateDCSMessage(msgHeader);
    DCS_Message_GetNewLeaseRequest *msgGetNewLeaseRequest = dynamic_cast<DCS_Message_GetNewLeaseRequest*>(msg);
    ASSERT_NE(msgGetNewLeaseRequest, nullptr);
    delete msg;

    msgHeader.kind1_ = DCS_Message_GetNewLeaseResponse::kind1_;
    msgHeader.kind2_ = DCS_Message_GetNewLeaseResponse::kind2_;
    
    msg = MessageFactory::CreateDCSMessage(msgHeader);
    DCS_Message_GetNewLeaseResponse *msgGetNewLeaseResponse = dynamic_cast<DCS_Message_GetNewLeaseResponse*>(msg);
    ASSERT_NE(msgGetNewLeaseResponse, nullptr);
    delete msg;

    msgHeader.kind1_ = DCS_Message_GetStatusRequest::kind1_;
    msgHeader.kind2_ = DCS_Message_GetStatusRequest::kind2_;
    
    msg = MessageFactory::CreateDCSMessage(msgHeader);
    DCS_Message_GetStatusRequest *msgGetStatusRequest = dynamic_cast<DCS_Message_GetStatusRequest*>(msg);
    ASSERT_NE(msgGetStatusRequest, nullptr);
    delete msg;

    msgHeader.kind1_ = DCS_Message_GetStatusResponse::kind1_;
    msgHeader.kind2_ = DCS_Message_GetStatusResponse::kind2_;
    
    msg = MessageFactory::CreateDCSMessage(msgHeader);
    DCS_Message_GetStatusResponse *msgGetStatusResponse = dynamic_cast<DCS_Message_GetStatusResponse*>(msg);
    ASSERT_NE(msgGetStatusResponse, nullptr);
    delete msg;

    msgHeader.kind1_ = DCS_Message_SetRPLLocationRequest::kind1_;
    msgHeader.kind2_ = DCS_Message_SetRPLLocationRequest::kind2_;
    
    msg = MessageFactory::CreateDCSMessage(msgHeader);
    DCS_Message_SetRPLLocationRequest *msgSetRPLLocationRequest = dynamic_cast<DCS_Message_SetRPLLocationRequest*>(msg);
    ASSERT_NE(msgSetRPLLocationRequest, nullptr);
    delete msg;

    msgHeader.kind1_ = DCS_Message_SetRPLLocationResponse::kind1_;
    msgHeader.kind2_ = DCS_Message_SetRPLLocationResponse::kind2_;
    
    msg = MessageFactory::CreateDCSMessage(msgHeader);
    DCS_Message_SetRPLLocationResponse *msgSetRPLLocationResponse = dynamic_cast<DCS_Message_SetRPLLocationResponse*>(msg);
    ASSERT_NE(msgSetRPLLocationResponse, nullptr);
    delete msg;

    msgHeader.kind1_ = DCS_Message_UpdateTimelineRequest::kind1_;
    msgHeader.kind2_ = DCS_Message_UpdateTimelineRequest::kind2_;
    
    msg = MessageFactory::CreateDCSMessage(msgHeader);
    DCS_Message_UpdateTimelineRequest *msgUpdateTimelineRequest = dynamic_cast<DCS_Message_UpdateTimelineRequest*>(msg);
    ASSERT_NE(msgUpdateTimelineRequest, nullptr);
    delete msg;

    msgHeader.kind1_ = DCS_Message_UpdateTimelineResponse::kind1_;
    msgHeader.kind2_ = DCS_Message_UpdateTimelineResponse::kind2_;
    
    msg = MessageFactory::CreateDCSMessage(msgHeader);
    DCS_Message_UpdateTimelineResponse *msgUpdateTimelineResponse = dynamic_cast<DCS_Message_UpdateTimelineResponse*>(msg);
    ASSERT_NE(msgUpdateTimelineResponse, nullptr);
    delete msg;

    msgHeader.kind1_ = DCS_Message_SetOutputModeRequest::kind1_;
    msgHeader.kind2_ = DCS_Message_SetOutputModeRequest::kind2_;
    
    msg = MessageFactory::CreateDCSMessage(msgHeader);
    DCS_Message_SetOutputModeRequest *msgSetOutputModeRequest = dynamic_cast<DCS_Message_SetOutputModeRequest*>(msg);
    ASSERT_NE(msgSetOutputModeRequest, nullptr);
    delete msg;

    msgHeader.kind1_ = DCS_Message_SetOutputModeResponse::kind1_;
    msgHeader.kind2_ = DCS_Message_SetOutputModeResponse::kind2_;
    
    msg = MessageFactory::CreateDCSMessage(msgHeader);
    DCS_Message_SetOutputModeResponse *msgSetOutputModeResponse = dynamic_cast<DCS_Message_SetOutputModeResponse*>(msg);
    ASSERT_NE(msgSetOutputModeResponse, nullptr);
    delete msg;

    msgHeader.kind1_ = DCS_Message_GetLogEventListRequest::kind1_;
    msgHeader.kind2_ = DCS_Message_GetLogEventListRequest::kind2_;
    
    msg = MessageFactory::CreateDCSMessage(msgHeader);
    DCS_Message_GetLogEventListRequest *msgGetLogEventListRequest = dynamic_cast<DCS_Message_GetLogEventListRequest*>(msg);
    ASSERT_NE(msgGetLogEventListRequest, nullptr);
    delete msg;

    msgHeader.kind1_ = DCS_Message_GetLogEventListResponse::kind1_;
    msgHeader.kind2_ = DCS_Message_GetLogEventListResponse::kind2_;
    
    msg = MessageFactory::CreateDCSMessage(msgHeader);
    DCS_Message_GetLogEventListResponse *msgGetLogEventListResponse = dynamic_cast<DCS_Message_GetLogEventListResponse*>(msg);
    ASSERT_NE(msgGetLogEventListResponse, nullptr);
    delete msg;

    msgHeader.kind1_ = DCS_Message_GetLogEventRequest::kind1_;
    msgHeader.kind2_ = DCS_Message_GetLogEventRequest::kind2_;
    
    msg = MessageFactory::CreateDCSMessage(msgHeader);
    DCS_Message_GetLogEventRequest *msgGetLogEventRequest = dynamic_cast<DCS_Message_GetLogEventRequest*>(msg);
    ASSERT_NE(msgGetLogEventRequest, nullptr);
    delete msg;

    msgHeader.kind1_ = DCS_Message_GetLogEventResponse::kind1_;
    msgHeader.kind2_ = DCS_Message_GetLogEventResponse::kind2_;
    
    msg = MessageFactory::CreateDCSMessage(msgHeader);
    DCS_Message_GetLogEventResponse *msgGetLogEventResponse = dynamic_cast<DCS_Message_GetLogEventResponse*>(msg);
    ASSERT_NE(msgGetLogEventResponse, nullptr);
    delete msg;
}