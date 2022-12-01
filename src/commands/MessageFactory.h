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

#ifndef MESSAGEFACTORY_H
#define MESSAGEFACTORY_H

#include <assert.h>
#include <cstdio>
#include <cstdlib>
#include <string>

#include "boost/shared_ptr.hpp"

#include "DCS_Message.h"

namespace SMPTE_SYNC
{
    /**
     * @brief Implements a class factor for DCS_Messages
     *
     *
     */
    
    class MessageFactory
    {
    public:

        /**
         *
         * Creates a new DCS_Message of the appropriate type based on the MessageHeader
         * The kind1_ and kind2_ data specify what type of DCS_Message to create.
         *
         * @param iHeader is the MessageHeader of the DCS_Message to create
         * @return The newly created DCS_Message of a specific type
         *
         */
        static DCS_Message* CreateDCSMessage(const MessageHeader &iHeader);
    };

}  // namespace SMPTE_SYNC

#endif // MESSAGEFACTORY_H
