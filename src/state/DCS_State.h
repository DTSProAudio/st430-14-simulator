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

#ifndef DCS_STATE_H
#define DCS_STATE_H

#include "ObservableState.h"

#include <string>

#include "boost/atomic.hpp"

#include "DataTypes.h"

namespace SMPTE_SYNC
{
    
    /**
     * @brief DCS_State class provides a concrete implementation defining the states of the DCS both on the client and server. Threadsafe.
     *
     * Implements the base state machine for the client and server implementing the 
     * SMPTE ST 430-10:2010 D-Cinema Operations — Auxiliary Content Synchronization Protocol
     *
     */

    class DCS_State : public ObservableState
    {
    public:
        
        /**
         * @enum EState
         *
         * @brief Defines the valid states for a DCS_State object.
         *
         * The states of both the DCS client and server are defined by the EState enum.
         * Provides support for managing behavior of the DCS layer as defined by 
         * SMPTE ST 430-10:2010 D-Cinema Operations — Auxiliary Content Synchronization Protocol
         *
         */
        
        typedef enum EState {
            eState_Disconnected,            /**< Identifies the disconnected state */
            eState_Connected,               /**< Identifies the connected state */
        } EState;
        
        /// Constructor
        DCS_State();
        
        /// Destructor
        virtual ~DCS_State(void);
        
        /// Helper method for converting from an EState enum to a string for displaying to the user
        static std::string EStateToString(EState iState);
        
        /// Sets the current EState of the DCS_State object. State transition validation performed. If state transition is not valid, returns false.
        virtual bool SetState(EState iState);

        /// Returns the current EState of the DCS_State object
        virtual EState GetState(void);
        
    protected:

        /// Defines and validates all legal state transitions.
        bool ValidateStateTransition(EState iState);
        
    private:
        /// Stores the EState of the DCS_State object.
        boost::atomic<EState> state_;
    };
    
}  // namespace SMPTE_SYNC

#endif // DCS_STATE_H
