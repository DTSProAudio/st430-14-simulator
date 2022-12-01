#error obsolete

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

#ifndef AUXDATAQUEUE_H_INCLUDED
#define AUXDATAQUEUE_H_INCLUDED

#include <unistd.h>
#include <stdint.h>
#include <iostream>
#include <vector>

#include "boost/thread/thread.hpp"

#include "Logger.h"

using namespace std;

namespace SMPTE_SYNC {

    template <class T>
    class AuxDataQueue
    {
    public:
        AuxDataQueue(int32_t iSize)
        {
            size_ = iSize;
            queue_ = new vector<T>;
            queue_->reserve(size_);
        }
        
        ~AuxDataQueue()
        {
            SMPTE_SYNC_LOG << "~AuxDataQueue size = " << queue_->size() << "\n";

            T *cmd = nullptr;
            
            while (this->try_dequeue(*cmd))
            {
                delete cmd;
                cmd = nullptr;
            }

            delete queue_;
        }

        size_t size(void)
        {
            boost::mutex::scoped_lock scoped_lock(mutex_);
            
            size_t queueSize = queue_->size();
            
            return queueSize;
        }

        bool empty(void)
        {
            boost::mutex::scoped_lock scoped_lock(mutex_);
            
            size_t queueSize = queue_->size();

            return queueSize == 0;
        }

        bool try_dequeue(T &oItem)
        {
            bool success = false;
            T tmp = nullptr;
            
            boost::mutex::scoped_lock scoped_lock(mutex_);
            
            if (queue_->size())
            {
                success = true;
                tmp = queue_->back();
                oItem = tmp;
                queue_->pop_back();
            }

            return success;
        }

        bool try_enqueue(const T &iItem)
        {
            bool success = false;
            
            boost::mutex::scoped_lock scoped_lock(mutex_);
            
            if (queue_->size() < size_)
            {
                success = true;
                queue_->insert(queue_->begin(), iItem);
            }
            
            return success;
        }

    private:
        vector<T>       *queue_;
        int32_t         size_;
        boost::mutex    mutex_;
    };

} // namespace MDA_COMM

#endif  // AUXDATAQUEUE_H_INCLUDED
