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

#include "MessageFactory.h"

#include <assert.h>
#include <cstdio>
#include <cstdlib>
#include <string>
#include "Logger.h"

namespace SMPTE_SYNC
{
    DCS_Message* MessageFactory::CreateDCSMessage(const MessageHeader &iHeader)
    {
        DCS_Message* msg = nullptr;
        
        if (iHeader.kind1_ == DCS_Message_AnnounceRequest::kind1_ && iHeader.kind2_ == DCS_Message_AnnounceRequest::kind2_)
        {
            msg = new DCS_Message_AnnounceRequest();
        }
        else
        if (iHeader.kind1_ == DCS_Message_AnnounceResponse::kind1_ && iHeader.kind2_ == DCS_Message_AnnounceResponse::kind2_)
        {
            msg = new DCS_Message_AnnounceResponse();
        }
        else
        if (iHeader.kind1_ == DCS_Message_GetNewLeaseRequest::kind1_ && iHeader.kind2_ == DCS_Message_GetNewLeaseRequest::kind2_)
        {
            msg = new DCS_Message_GetNewLeaseRequest();
        }
        else
        if (iHeader.kind1_ == DCS_Message_GetNewLeaseResponse::kind1_ && iHeader.kind2_ == DCS_Message_GetNewLeaseResponse::kind2_)
        {
            msg = new DCS_Message_GetNewLeaseResponse();
        }
        else
        if (iHeader.kind1_ == DCS_Message_GetStatusRequest::kind1_ && iHeader.kind2_ == DCS_Message_GetStatusRequest::kind2_)
        {
            msg = new DCS_Message_GetStatusRequest();
        }
        else
        if (iHeader.kind1_ == DCS_Message_GetStatusResponse::kind1_ && iHeader.kind2_ == DCS_Message_GetStatusResponse::kind2_)
        {
            msg = new DCS_Message_GetStatusResponse();
        }
        else
        if (iHeader.kind1_ == DCS_Message_SetRPLLocationRequest::kind1_ && iHeader.kind2_ == DCS_Message_SetRPLLocationRequest::kind2_)
        {
            msg = new DCS_Message_SetRPLLocationRequest();
        }
        else
        if (iHeader.kind1_ == DCS_Message_SetRPLLocationResponse::kind1_ && iHeader.kind2_ == DCS_Message_SetRPLLocationResponse::kind2_)
        {
            msg = new DCS_Message_SetRPLLocationResponse();
        }
        else
        if (iHeader.kind1_ == DCS_Message_UpdateTimelineRequest::kind1_ && iHeader.kind2_ == DCS_Message_UpdateTimelineRequest::kind2_)
        {
            msg = new DCS_Message_UpdateTimelineRequest();
        }
        else
        if (iHeader.kind1_ == DCS_Message_UpdateTimelineResponse::kind1_ && iHeader.kind2_ == DCS_Message_UpdateTimelineResponse::kind2_)
        {
            msg = new DCS_Message_UpdateTimelineResponse();
        }
        else
        if (iHeader.kind1_ == DCS_Message_SetOutputModeRequest::kind1_ && iHeader.kind2_ == DCS_Message_SetOutputModeRequest::kind2_)
        {
            msg = new DCS_Message_SetOutputModeRequest();
        }
        else
        if (iHeader.kind1_ == DCS_Message_SetOutputModeResponse::kind1_ && iHeader.kind2_ == DCS_Message_SetOutputModeResponse::kind2_)
        {
            msg = new DCS_Message_SetOutputModeResponse();
        }
        else
        if (iHeader.kind1_ == DCS_Message_GetLogEventListRequest::kind1_ && iHeader.kind2_ == DCS_Message_GetLogEventListRequest::kind2_)
        {
            msg = new DCS_Message_GetLogEventListRequest();
        }
        else
        if (iHeader.kind1_ == DCS_Message_GetLogEventListResponse::kind1_ && iHeader.kind2_ == DCS_Message_GetLogEventListResponse::kind2_)
        {
            msg = new DCS_Message_GetLogEventListResponse();
        }
        else
        if (iHeader.kind1_ == DCS_Message_GetLogEventRequest::kind1_ && iHeader.kind2_ == DCS_Message_GetLogEventRequest::kind2_)
        {
            msg = new DCS_Message_GetLogEventRequest();
        }
        else
        if (iHeader.kind1_ == DCS_Message_GetLogEventResponse::kind1_ && iHeader.kind2_ == DCS_Message_GetLogEventResponse::kind2_)
        {
            msg = new DCS_Message_GetLogEventResponse();
        }

        if (msg)
            msg->ReadHeader(iHeader);
        else
        {
            SMPTE_SYNC_LOG << "CreateMessage failed to create command. Unknown command.\n";
        }
        
        return msg;
    }

}  // namespace SMPTE_SYNC
