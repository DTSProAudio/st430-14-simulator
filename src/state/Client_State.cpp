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

#include "Client_State.h"
#include <assert.h>
#include "Logger.h"

namespace SMPTE_SYNC
{
    
    std::string Client_State::EStateToString(EState iState)
    {
        //SMPTE_SYNC_LOG << "EStateToString\n";
        
        std::string strVal = "UNKNOWN STATE";
        
        switch (iState) {
            case eState_Wait:
                strVal = "eState_Wait";
                break;
                
            case eState_Connect:
                strVal = "eState_Connect";
                break;
                
            case eState_Buffer:
                strVal = "eState_Buffer";
                break;

            case eState_Play:
                strVal = "eState_Play";
                break;
                
            default:
                assert(!"eStateToString - UNKNOWN STATE");
                break;
        }
        
        return strVal;
    }
    
    Client_State::Client_State() : state_(eState_Wait)
    {
        
    }
    
    Client_State::~Client_State(void)
    {
        
    }
    
    bool Client_State::ValidateStateTransition(EState iState)
    {
        SMPTE_SYNC_LOG << "Client_State::ValidateStateTransition\n";
        SMPTE_SYNC_LOG << "iState = " << EStateToString(iState);
        SMPTE_SYNC_LOG << "state_ = " << EStateToString(state_);
        
        /// TODO: Double check the state transitions
        bool valid = false;
        if (state_ == eState_Wait)
        {
            if (iState == eState_Connect)
            {
                valid = true;
            }
        }
        else
        if (state_ == eState_Connect)
        {
            if (iState == eState_Wait)
            {
                valid = true;
            }
            else
            if (iState == eState_Buffer)
            {
                valid = true;
            }
        }
        else
        if (state_ == eState_Buffer)
        {
            if (iState == eState_Wait)
            {
                valid = true;
            }
            else
            if (iState == eState_Connect)
            {
                valid = true;
            }
            else
            if (iState == eState_Play)
            {
                valid = true;
            }
        }
        else
        if (state_ == eState_Play)
        {
            if (iState == eState_Wait)
            {
                valid = true;
            }
            else
            if (iState == eState_Buffer)
            {
                valid = true;
            }
        }
        
        return valid;
    }
    
    bool Client_State::SetState(EState iState)
    {
        EState currentState = eState_Wait;
        currentState = state_;
        
        if (currentState != iState)
        {
            SMPTE_SYNC_LOG << "Client_State::SetState\n";
            SMPTE_SYNC_LOG << "iState = " << EStateToString(iState);
            
            this->ValidateStateTransition(iState);

            state_ = iState;

            this->NotifyStateChange();
        }
        
        return true;
    }
    
    Client_State::EState Client_State::GetState(void)
    {
        //SMPTE_SYNC_LOG << "Client_State::GetState\n";
        //SMPTE_SYNC_LOG << "state_ = " << EStateToString(state_);

        return state_;
    }
    
}  // namespace SMPTE_SYNC
