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

#include "Client_Validator.h"

#include <iostream>
#include <fstream>

#include "Logger.h"
#include "AuxData.h"
#include "AuxDataMgr.h"
#include "Utils.h"

namespace SMPTE_SYNC
{
    Client_Validator::Client_Validator(AuxDataMgr *iAuxDataMgr
                                       , int32_t iSampleRate) :
          auxDataMgr_(iAuxDataMgr)
        , baseSampleRate_(iSampleRate)
        , auxData_(nullptr)
        , isValid_(false)
        , timelineEditUnitIndex_(-1)
    {
    }

    Client_Validator::~Client_Validator()
    {
    }

    bool Client_Validator::IsValid()
    {
        return isValid_;
    }
    
    void Client_Validator::Validate(const syncPacket &iSyncPacket)
    {
        //SMPTE_SYNC_LOG << "Client_Validator::Validate\n";

        // Algorith:
        //
        // Paused State
        //
        // Stopped State
        //
        // Play State
        //
        // 1) Fetch the next data item on the queue
        // 2) If this data item matches the syncPacket
        //      Success!
        // 3) If this data item does not match the syncPacket
        //      If it comes ealier in the timeline,
        //          log the error and keep fetching
        //          if you get to the end of the queue and did not find
        //          the data item, log the error
        //
        //      If it comes later in the timeline
        //          log the error
        //          hold onto the data item to use for a future syncPacket
        //

        do
        {
            if (auxData_ == nullptr)
                auxData_ = auxDataMgr_->GetNextDataItem();
            
            if (auxData_ != nullptr)
            {
                if (this->Test(iSyncPacket, auxData_))
                {
                    delete auxData_;
                    auxData_ = nullptr;
                    isValid_ = true;

                    //SMPTE_SYNC_LOG << "Valid iSyncPacket = " << iSyncPacket.timelineEditUnitIndex_;
                    
                    break;
                }
                else
                if (auxData_->editUnitIndex_ < iSyncPacket.timelineEditUnitIndex_)
                {
                    SMPTE_SYNC_LOG << "Dropping earlier AuxDataItem"
                    << " auxData_->editUnitIndex_ = " << auxData_->editUnitIndex_
                    << " iSyncPacket.timelineEditUnitIndex_ = " << iSyncPacket.timelineEditUnitIndex_;

                    isValid_ = false;

                    delete auxData_;
                    auxData_ = auxDataMgr_->GetNextDataItem();
                }
                else
                if (auxData_->editUnitIndex_ > iSyncPacket.timelineEditUnitIndex_)
                {
                    SMPTE_SYNC_LOG_LEVEL(trace) << "Found an aux data item that is ahead of the current sync sample time. Hold item until we get to the current sync sample time. "
                    << " auxData_->editUnitIndex_ = " << auxData_->editUnitIndex_
                    << " iSyncPacket.timelineEditUnitIndex_ = " << iSyncPacket.timelineEditUnitIndex_;
                    
                    isValid_ = false;

                    break;
                }
            }
            else
            {
                if (iSyncPacket.timelineEditUnitIndex_ != timelineEditUnitIndex_)
                {
                    SMPTE_SYNC_LOG << "No AuxDataItems to test. Not testing syncPacket timelineEditUnitIndex_ = " << timelineEditUnitIndex_ << " iSyncPacket.timelineEditUnitIndex_ = " << iSyncPacket.timelineEditUnitIndex_;
                }
                
                isValid_ = false;

                break;
            }
        } while (auxData_ != nullptr);
        
        // Update the saved edit unit index
        //
        timelineEditUnitIndex_ = iSyncPacket.timelineEditUnitIndex_;
    }

    bool Client_Validator::Test(const syncPacket &iSyncPacket , AuxDataBlock *iAuxDataBlock)
    {
        bool success = true;
        
        if (iSyncPacket.timelineEditUnitIndex_ != iAuxDataBlock->editUnitIndex_)
        {
            return false;
        }
        
        return success;
    }

}  // namespace SMPTE_SYNC
