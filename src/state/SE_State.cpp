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

#include "SE_State.h"

#include <assert.h>
#include <string>

#include "Logger.h"

namespace SMPTE_SYNC
{
    
    std::string SE_State::EStateToString(EState iState)
    {
        //SMPTE_SYNC_LOG << "EStateToString\n";
        
        std::string strVal = "UNKNOWN STATE";
        
        switch (iState) {
            case eState_NoData:
                strVal = "eState_NoData";
                break;

            case eState_Stopped:
                strVal = "eState_Stopped";
                break;
                
            case eState_Paused:
                strVal = "eState_Paused";
                break;
                
            case eState_WaitingToPlay:
                strVal = "eState_WaitingToPlay";
                break;

            case eState_Playing:
                strVal = "eState_Playing";
                break;
                
            default:
                assert(!"eStateToString - UNKNOWN STATE");
                break;
        }
        
        return strVal;
    }
    
    SE_State::SE_State() : state_(eState_NoData)
    {
        bool isLockFree = state_.is_lock_free();
        SMPTE_SYNC_LOG << "SE_State isLockFree = " << (isLockFree ? "true" : "false");
    }
    
    SE_State::~SE_State(void)
    {
        
    }
    
    bool SE_State::ValidateStateTransition(EState iState)
    {
        //SMPTE_SYNC_LOG << "SE_State::ValidateStateTransition\n";
        //SMPTE_SYNC_LOG << "iState = " << EStateToString(iState);
        //SMPTE_SYNC_LOG << "state_ = " << EStateToString(state_);
        
        bool valid = true;
        
        return valid;
    }
    
    bool SE_State::SetState(EState iState)
    {
        EState currentState = eState_NoData;
        currentState = state_;
        
        if (currentState != iState)
        {
            SMPTE_SYNC_LOG << "SE_State::SetState";
            SMPTE_SYNC_LOG << "iState = " << EStateToString(iState);
            
            this->ValidateStateTransition(iState);
            
            state_ = iState;

            this->NotifyStateChange();
        }
        
        return true;
    }
    
    SE_State::EState SE_State::GetState(void)
    {
        //SMPTE_SYNC_LOG << "SE_State::GetState\n";
        //SMPTE_SYNC_LOG << "state_ = " << EStateToString(state_);

        return state_;
    }
    
}  // namespace SMPTE_SYNC
