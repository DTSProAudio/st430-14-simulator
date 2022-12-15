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

#ifndef AUXDATAMGR_H
#define AUXDATAMGR_H

#include <assert.h>
#include <cstdio>
#include <cstdlib>
#include <string>

#include "boost/lockfree/queue.hpp"

#include "DataTypes.h"
#include "AuxData.h"

namespace SMPTE_SYNC
{
    /**
     * @brief AuxDataMgr class implements a threadsafe queue for storing in coming AuxDataBlock pointers.
     * The AuxDataMgr is used on the client side or processor (rather than server side) to store data until
     * the sync signal is received and can be used to validate the AuxDataBlock and the syncSignal objects.
     *
     */
    class AuxDataMgr
    {
    public:

        /// Constructor
        AuxDataMgr();

        /// Destructor
        ~AuxDataMgr();

        /**
         *
         * Returns the a pointer to the AuxDataBlock that was just dequeued.
         * The caller is expected to delete the AuxDataBlock.
         *
         * @return AuxDataBlock pointer
         *
         */
        AuxDataBlock* GetNextDataItem(void);
        
        /**
         *
         * Enqueues a pointer to the AuxDataBlock that was just received from the HTTP call requesting data.
         *
         * @param iItem is a AuxDataBlock pointer to an object received and created by the SS_Client
         * @return true/false if the AuxDataBlock has been enqueued. This should not fail under normal circumstances.
         *
         */
        bool AddDataItem(AuxDataBlock* iItem);
        
    private:
        typedef boost::lockfree::queue<AuxDataBlock*, boost::lockfree::fixed_sized<false>> DataQueue;

        DataQueue *auxDataQueue_;
    };

}  // namespace SMPTE_SYNC

#endif // AUXDATAMGR_H
