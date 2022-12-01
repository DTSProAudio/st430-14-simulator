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

#ifndef SE_STATE_H
#define SE_STATE_H

#include "ObservableState.h"

#include <string>

#include "boost/atomic.hpp"

#include "DataTypes.h"

namespace SMPTE_SYNC
{
    
    /**
     *
     * @brief SE_State class implements the client or server state machine for the SE or Sync Emitter layer.
     *
     */

    class SE_State : public ObservableState
    {
    public:

        /**
         *
         * @enum EState
         *
         * @brief Defines the valid states for a SE_State object.
         *
         * The states of client or server are defined by the EState enum.
         *
         */
        
        typedef enum EState {
            eState_NoData,                  /**< Identifies the no data state where there is no show loaded in the timeline and silence is generated for the AES/EBU signal */
            eState_Stopped,                 /**< Identifies the stopped state where the SE sync signal is in a stopped state */
            eState_Paused,                  /**< Identifies the stopped state where the SE sync signal is in a paused state */
            eState_WaitingToPlay,           /**< Identifies the stopped state where the SE sync signal is in a waiting to play state. This is when playback has been initiated but the sign signal is paused or stopped. This case be a delay of multiple seconds or more. */
            eState_Playing                  /**< Identifies the stopped state where the SE sync signal is in a playing state */
        } EState;
        
        /// Constructor
        SE_State();
        
        /// Destructor
        ~SE_State(void);
        
        /// Helper method for converting from an EState enum to a string for displaying to the user
        static std::string EStateToString(EState iState);
        
        /// Sets the current EState of the SE_State object. State transition validation performed. If state transition is not valid, returns false.
        bool SetState(EState iState);

        /// Returns the current EState of the SE_State object
        EState GetState(void);
        
    protected:

        /// Defines and validates all legal state transitions.
        bool ValidateStateTransition(EState iState);

    private:

        /// Stores the EState of the SE_State object.
        boost::atomic<EState> state_;
    };
    
}  // namespace SMPTE_SYNC

#endif // SE_STATE_H
