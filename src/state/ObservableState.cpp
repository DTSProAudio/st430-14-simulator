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

#include "ObservableState.h"

#include <assert.h>
#include <vector>

#include "Logger.h"

namespace SMPTE_SYNC
{

    ObservableState::ObservableState()
    {
        
    }
    
    ObservableState::~ObservableState()
    {
        
    }

    void ObservableState::AddObserver(StateObserver *iCallback)
    {
        boost::mutex::scoped_lock scoped_lock(observersListMutex_);
        
        // Check to see if we already have this in our list
        //
        for (std::vector<StateObserver*>::iterator iter = observers_.begin(); iter != observers_.end(); iter++)
        {
            if (*iter == iCallback)
                return;
        }
        
        observers_.push_back(iCallback);
    }
    
    void ObservableState::RemoveObserver(StateObserver *iCallback)
    {
        boost::mutex::scoped_lock scoped_lock(observersListMutex_);
        
        for (std::vector<StateObserver*>::iterator iter = observers_.begin(); iter != observers_.end(); iter++)
        {
            if (*iter == iCallback)
            {
                observers_.erase(iter);
                return;
            }
        }
    }

    void ObservableState::NotifyStateChange(void)
    {
        boost::mutex::scoped_lock scoped_lock(observersListMutex_);
        
        for (std::vector<StateObserver*>::iterator iter = observers_.begin(); iter != observers_.end(); iter++)
        {
            (*iter)->NotifyStateChange();
        }
    }

}  // namespace SMPTE_SYNC
