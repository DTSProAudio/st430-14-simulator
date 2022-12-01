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

#include "DCS_State.h"

#include <assert.h>
#include <string>

#include "Logger.h"

namespace SMPTE_SYNC
{
    
    std::string DCS_State::EStateToString(EState iState)
    {
        //SMPTE_SYNC_LOG << "EStateToString";
        
        std::string strVal = "UNKNOWN STATE";
        
        switch (iState) {
            case eState_Disconnected:
                strVal = "eState_Disconnected";
                break;
                
            case eState_Connected:
                strVal = "eState_Connected";
                break;
                
            default:
                assert(!"eStateToString - UNKNOWN STATE");
                break;
        }
        
        return strVal;
    }
    
    DCS_State::DCS_State() : state_(eState_Disconnected)
    {
        
    }
    
    DCS_State::~DCS_State(void)
    {
        
    }
    
    bool DCS_State::ValidateStateTransition(EState iState)
    {
        SMPTE_SYNC_LOG << "DCS_State::ValidateStateTransition";
        SMPTE_SYNC_LOG << "iState = " << EStateToString(iState);
        SMPTE_SYNC_LOG << "state_ = " << EStateToString(state_);
        
        bool valid = true;
        
        return valid;
    }
    
    bool DCS_State::SetState(EState iState)
    {
        EState currentState = eState_Disconnected;
        currentState = state_;
        
        if (currentState != iState)
        {
            SMPTE_SYNC_LOG << "DCS_State::SetState";
            SMPTE_SYNC_LOG << "iState = " << EStateToString(iState);
            
            this->ValidateStateTransition(iState);
            
            state_ = iState;
            
            this->NotifyStateChange();
        }
        
        return true;
    }
    
    DCS_State::EState DCS_State::GetState(void)
    {
        //SMPTE_SYNC_LOG << "DCS_State::GetState";
        //SMPTE_SYNC_LOG << "state_ = " << EStateToString(state_);
        
        return state_;
    }
    
}  // namespace SMPTE_SYNC
