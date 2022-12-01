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

#ifndef CLIENT_VALIDATOR_H
#define CLIENT_VALIDATOR_H

#include <stdint.h>
#include <iostream>
#include <istream>
#include <ostream>
#include <string>
#include <queue>

#include "boost/atomic.hpp"
#include "boost/function.hpp"

#include "sync.h"
#include "DataTypes.h"

namespace SMPTE_SYNC
{
    class AuxDataMgr;
    class AuxDataBlock;

    /**
     * @brief Callback function for the client to validate the sync signal.
     * Installed in the SE_Client.
     *
     */
    typedef boost::function<void(const syncPacket &iSyncPacket)> SyncPacketValidatorCallback;

    /**
     * @brief Client_Validator class implements validation of the Aux Data items against the Sync Signal.
     * Additional validation cases can be added to Client_Validator::Validate
     *
     */
    class Client_Validator
    {
    public:

        /**
         * Constructor
         *
         * @param iAuxDataMgr \link AuxDataMgr \endlink is source of recieved Aux Data Items.
         * @param iSampleRate sample rate of the AES/EBU signal
         *
         */

        Client_Validator(AuxDataMgr *iAuxDataMgr
                         , int32_t iSampleRate);

        /// Destructor
        virtual ~Client_Validator();

        /**
         *
         * Returns true/false if the Aux Data item and sync signal are in a valid state.

         * @return true/false if the Aux Data item and sync signal are in a valid state.
         *
         */
        bool IsValid(void);
        
        /**
         *
         * Provides validation checks on the syncPacket object.
         * Takes the syncPacket object and looks through the queue of aux data items to 
         * find one that is the same as the syncPacket. If the syncPacket has a greater
         * edit unit index (timeline timestamp), it stops dequeueing aux data items 
         * until a syncSignal with a larger edit unit index arrives.
         *
         * @param iSyncPacket syncPacket is the sync signal data to be validated
         *
         */
        void Validate(const syncPacket &iSyncPacket);

    private:
        
        /**
         *
         * Implements specific test criteria for comparing the syncSignal and AuxDataBlock information.
         *
         * @param iSyncPacket syncPacket is the syncSignal to compare
         * @param iAuxDataBlock AuxDataBlock is the AuxDataBlock to compare
         * @return true/false if two signals are considered equal
         *
         */
        bool Test(const syncPacket &iSyncPacket, AuxDataBlock *iAuxDataBlock);

        /// Pointer to the currently installed AuxDataMgr
        AuxDataMgr      *auxDataMgr_;

        /// Sample rate of the sync signal (AES/EBU signal)
        const int32_t   baseSampleRate_;

        /// A pointer to the last dequeued AuxDataBlock. This is the AuxDataBlock that is being tested with the syncSignal
        AuxDataBlock    *auxData_;

        /**
         *
         * The last recieved that is not a duplicate of the previous one. 
         * In the case of paused or stopped state, the server is generating the same frame over and over.
         *
         */
        ///
        
        uint32_t    timelineEditUnitIndex_;

        /// Is the Client_Validator in a valid state. That is did the last syncSignal and AuxDataBlock comparison pass.
        boost::atomic<bool> isValid_;
    };

}  // namespace SMPTE_SYNC

#endif // CLIENT_VALIDATOR_H
