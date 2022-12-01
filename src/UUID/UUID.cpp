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


#include "UUID.h"

#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

namespace SMPTE_SYNC
{
    /*
     *  Function: GetHexChar
     *  Hex character to hex value
     */

    bool GetHexChar(uint8_t cin, uint8_t *cout, bool *count);
    bool GetHexChar(uint8_t cin, uint8_t *cout, bool *count)
    {
        cin = tolower(cin);
        
        *count = false;
        *cout = 0;
        
        if (('0' <= cin) && (cin <= '9'))
        {
            *cout = (uint8_t) (cin - '0');
            *count = true;
            
            return true;
        }
        
        if (('a' <= cin) && (cin <= 'f'))
        {
            *cout = (uint8_t) (cin - 'a' + 10);
            *count = true;
            
            return true;
        }
        
        if (cin == '-')
        {
            *count = false;
            return true;
        }
        
        return false;
    }

    void Initialize(UUID uuid)
    {
        memset(uuid, 0, 16);
    }

    /*
     *  Function: Str2UUID
     *  UUID string to UUID value
     */

    bool Str2UUID(const char *str, UUID uuid)
    {
        for (int i = 0; i < 16; i++)
        {
            uint8_t cm, cl;
            bool count;
            
            // Get first char
            if (!GetHexChar(*(str++),&cm,&count))
            {
                return false;
            }
            
            if (count)
            {
                // If first not '-'
                if (!GetHexChar(*(str++),&cl,&count))
                {
                    return false;
                }
                
                if (!count)
                {
                    // If second is '-'
                    return false;
                }
            }
            else
            {
                // If first is '-'
                if (!GetHexChar(*(str++),&cm,&count))
                {
                    return false;
                }
                
                // First is not '-'
                if (!count)
                {
                    // If second is '-'
                    return false;
                }

                // Second is not '-'
                if (!GetHexChar(*(str++),&cl,&count))
                {
                    return false;
                }
                
                if (!count)
                {
                    // If third is '-'
                    return false;
                }
            }
            
            uuid[i] = (uint8_t) ((cm << 4) | cl) & 0xFF;
        }
        
        return true;
    }
    
    bool StringToUUID(const std::string &iUUIDStr, UUID oUUID)
    {
        return Str2UUID(iUUIDStr.c_str(), oUUID);
    }
    
    bool IsEqual(const UUID iUUID1, const UUID iUUID2)
    {
        return strncmp((char*)iUUID1, (char*)iUUID2, 16) == 0;
    }
    
    bool Copy(const UUID iSource, UUID oDestination)
    {
        memcpy(oDestination, iSource, 16);
        
        return true;
    }
    

}  // namespace SMPTE_SYNC