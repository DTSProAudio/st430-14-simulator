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

#include "AuxDataMgr.h"

#include <assert.h>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>

#include "Logger.h"

namespace SMPTE_SYNC
{
    AuxDataMgr::AuxDataMgr()
    {
        SMPTE_SYNC_LOG << "AuxDataMgr::AuxDataMgr\n";
        auxDataQueue_ = new DataQueue(0);
    }
    
    AuxDataMgr::~AuxDataMgr()
    {
        SMPTE_SYNC_LOG << "AuxDataMgr::~AuxDataMgr\n";

        AuxDataBlock *auxData = nullptr;
        while (auxDataQueue_->pop(auxData))
        {
            delete auxData;
            auxData = nullptr;
        }
        
        delete auxDataQueue_;
    }

    AuxDataBlock* AuxDataMgr::GetNextDataItem(void)
    {
        AuxDataBlock* item = nullptr;

        auxDataQueue_->pop(item);

        return item;
    }

    bool AuxDataMgr::AddDataItem(AuxDataBlock* iItem)
    {
        bool success = true;
        
        success = auxDataQueue_->push(iItem);
        
        return success;
    }

}  // namespace SMPTE_SYNC
