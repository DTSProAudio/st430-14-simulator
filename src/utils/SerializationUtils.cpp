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

#include "SerializationUtils.h"

#include <string>

namespace SMPTE_SYNC {

    void Write(uint8_t **iBuf, uint8_t iVal)
    {
        memcpy(*iBuf, &iVal, sizeof(uint8_t));
        *iBuf += sizeof(uint8_t);
    }
    
    void Read(uint8_t **iBuf, uint8_t &oVal)
    {
        memcpy(&oVal, *iBuf, sizeof(uint8_t));
        *iBuf += sizeof(uint8_t);
    }

    void Write(uint8_t **iBuf, int8_t iVal)
    {
        memcpy(*iBuf, &iVal, sizeof(int8_t));
        *iBuf += sizeof(int8_t);
    }
    
    void Read(uint8_t **iBuf, int8_t &oVal)
    {
        memcpy(&oVal, *iBuf, sizeof(int8_t));
        *iBuf += sizeof(int8_t);
    }
    
    void Write(uint8_t **iBuf, uint32_t iVal)
    {
        uint8_t tmp = 0x0;
        
        tmp = (iVal & 0xFF000000) >> 24;

        memcpy(*iBuf, &tmp, sizeof(uint8_t));
        *iBuf += sizeof(uint8_t);
        
        tmp = (iVal & 0x00FF0000) >> 16;
        memcpy(*iBuf, &tmp, sizeof(uint8_t));
        *iBuf += sizeof(uint8_t);
        
        tmp = (iVal & 0x0000FF00) >> 8;
        memcpy(*iBuf, &tmp, sizeof(uint8_t));
        *iBuf += sizeof(uint8_t);
        
        tmp = (iVal & 0x00000000FF);
        memcpy(*iBuf, &tmp, sizeof(uint8_t));
        *iBuf += sizeof(uint8_t);
    }
    
    void Read(uint8_t **iBuf, uint32_t &oVal)
    {
        uint8_t tmp;
        
        memcpy(&tmp, *iBuf, sizeof(uint8_t));
        *iBuf += sizeof(uint8_t);
        
        oVal = tmp;
        oVal = oVal << 8;
        
        memcpy(&tmp, *iBuf, sizeof(uint8_t));
        *iBuf += sizeof(uint8_t);
        
        oVal |= tmp;
        oVal = oVal << 8;
        
        memcpy(&tmp, *iBuf, sizeof(uint8_t));
        *iBuf += sizeof(uint8_t);
        
        oVal |= tmp;
        oVal = oVal << 8;
        
        memcpy(&tmp, *iBuf, sizeof(uint8_t));
        *iBuf += sizeof(uint8_t);
        
        oVal |= tmp;
    }

    void Write(uint8_t **iBuf, int32_t iVal)
    {
        uint8_t tmp = 0x0;
        
        tmp = (iVal & 0xFF000000) >> 24;
        
        memcpy(*iBuf, &tmp, sizeof(uint8_t));
        *iBuf += sizeof(uint8_t);
        
        tmp = (iVal & 0x00FF0000) >> 16;
        memcpy(*iBuf, &tmp, sizeof(uint8_t));
        *iBuf += sizeof(uint8_t);
        
        tmp = (iVal & 0x0000FF00) >> 8;
        memcpy(*iBuf, &tmp, sizeof(uint8_t));
        *iBuf += sizeof(uint8_t);
        
        tmp = (iVal & 0x00000000FF);
        memcpy(*iBuf, &tmp, sizeof(uint8_t));
        *iBuf += sizeof(uint8_t);
    }
    
    void Read(uint8_t **iBuf, int32_t &oVal)
    {
        uint8_t tmp;
        
        memcpy(&tmp, *iBuf, sizeof(uint8_t));
        *iBuf += sizeof(uint8_t);
        
        oVal = tmp;
        oVal = oVal << 8;
        
        memcpy(&tmp, *iBuf, sizeof(uint8_t));
        *iBuf += sizeof(uint8_t);
        
        oVal |= tmp;
        oVal = oVal << 8;
        
        memcpy(&tmp, *iBuf, sizeof(uint8_t));
        *iBuf += sizeof(uint8_t);
        
        oVal |= tmp;
        oVal = oVal << 8;
        
        memcpy(&tmp, *iBuf, sizeof(uint8_t));
        *iBuf += sizeof(uint8_t);
        
        oVal |= tmp;
    }

    void Write(uint8_t **iBuf, uint64_t iVal)
    {
        uint8_t tmp = 0x0;
        
        tmp = (iVal & 0xFF00000000000000) >> 56;
        
        memcpy(*iBuf, &tmp, sizeof(uint8_t));
        *iBuf += sizeof(uint8_t);
        
        tmp = (iVal & 0x00FF000000000000) >> 48;
        memcpy(*iBuf, &tmp, sizeof(uint8_t));
        *iBuf += sizeof(uint8_t);
        
        tmp = (iVal & 0x0000FF0000000000) >> 40;
        memcpy(*iBuf, &tmp, sizeof(uint8_t));
        *iBuf += sizeof(uint8_t);
        
        tmp = (iVal & 0x000000FF00000000) >> 32;
        memcpy(*iBuf, &tmp, sizeof(uint8_t));
        *iBuf += sizeof(uint8_t);

        tmp = (iVal & 0x00000000FF000000) >> 24;
        memcpy(*iBuf, &tmp, sizeof(uint8_t));
        *iBuf += sizeof(uint8_t);
        
        tmp = (iVal & 0x0000000000FF0000) >> 16;
        memcpy(*iBuf, &tmp, sizeof(uint8_t));
        *iBuf += sizeof(uint8_t);
        
        tmp = (iVal & 0x000000000000FF00) >> 8;
        memcpy(*iBuf, &tmp, sizeof(uint8_t));
        *iBuf += sizeof(uint8_t);
        
        tmp = (iVal & 0x00000000000000FF);
        memcpy(*iBuf, &tmp, sizeof(uint8_t));
        *iBuf += sizeof(uint8_t);
    }
    
    void Read(uint8_t **iBuf, uint64_t &oVal)
    {
        uint8_t tmp;
        
        memcpy(&tmp, *iBuf, sizeof(uint8_t));
        *iBuf += sizeof(uint8_t);
        
        oVal = tmp;
        oVal = oVal << 8;
        
        memcpy(&tmp, *iBuf, sizeof(uint8_t));
        *iBuf += sizeof(uint8_t);
        
        oVal |= tmp;
        oVal = oVal << 8;
        
        memcpy(&tmp, *iBuf, sizeof(uint8_t));
        *iBuf += sizeof(uint8_t);
        
        oVal |= tmp;
        oVal = oVal << 8;
        
        memcpy(&tmp, *iBuf, sizeof(uint8_t));
        *iBuf += sizeof(uint8_t);
        
        oVal |= tmp;
        memcpy(&tmp, *iBuf, sizeof(uint8_t));
        *iBuf += sizeof(uint8_t);
        
        oVal = tmp;
        oVal = oVal << 8;
        
        memcpy(&tmp, *iBuf, sizeof(uint8_t));
        *iBuf += sizeof(uint8_t);
        
        oVal |= tmp;
        oVal = oVal << 8;
        
        memcpy(&tmp, *iBuf, sizeof(uint8_t));
        *iBuf += sizeof(uint8_t);
        
        oVal |= tmp;
        oVal = oVal << 8;
        
        memcpy(&tmp, *iBuf, sizeof(uint8_t));
        *iBuf += sizeof(uint8_t);
        
        oVal |= tmp;
    }
    
    void Write(uint8_t **iBuf, int64_t iVal)
    {
        uint8_t tmp = 0x0;
        
        tmp = (iVal & 0xFF00000000000000) >> 56;
        
        memcpy(*iBuf, &tmp, sizeof(uint8_t));
        *iBuf += sizeof(uint8_t);
        
        tmp = (iVal & 0x00FF000000000000) >> 48;
        memcpy(*iBuf, &tmp, sizeof(uint8_t));
        *iBuf += sizeof(uint8_t);
        
        tmp = (iVal & 0x0000FF0000000000) >> 40;
        memcpy(*iBuf, &tmp, sizeof(uint8_t));
        *iBuf += sizeof(uint8_t);
        
        tmp = (iVal & 0x000000FF00000000) >> 32;
        memcpy(*iBuf, &tmp, sizeof(uint8_t));
        *iBuf += sizeof(uint8_t);
        
        tmp = (iVal & 0x00000000FF000000) >> 24;
        memcpy(*iBuf, &tmp, sizeof(uint8_t));
        *iBuf += sizeof(uint8_t);
        
        tmp = (iVal & 0x0000000000FF0000) >> 16;
        memcpy(*iBuf, &tmp, sizeof(uint8_t));
        *iBuf += sizeof(uint8_t);
        
        tmp = (iVal & 0x000000000000FF00) >> 8;
        memcpy(*iBuf, &tmp, sizeof(uint8_t));
        *iBuf += sizeof(uint8_t);
        
        tmp = (iVal & 0x00000000000000FF);
        memcpy(*iBuf, &tmp, sizeof(uint8_t));
        *iBuf += sizeof(uint8_t);
    }
    
    void Read(uint8_t **iBuf, int64_t &oVal)
    {
        uint8_t tmp;
        
        memcpy(&tmp, *iBuf, sizeof(uint8_t));
        *iBuf += sizeof(uint8_t);
        
        oVal = tmp;
        oVal = oVal << 8;
        
        memcpy(&tmp, *iBuf, sizeof(uint8_t));
        *iBuf += sizeof(uint8_t);
        
        oVal |= tmp;
        oVal = oVal << 8;
        
        memcpy(&tmp, *iBuf, sizeof(uint8_t));
        *iBuf += sizeof(uint8_t);
        
        oVal |= tmp;
        oVal = oVal << 8;
        
        memcpy(&tmp, *iBuf, sizeof(uint8_t));
        *iBuf += sizeof(uint8_t);
        
        oVal |= tmp;
        memcpy(&tmp, *iBuf, sizeof(uint8_t));
        *iBuf += sizeof(uint8_t);
        
        oVal = tmp;
        oVal = oVal << 8;
        
        memcpy(&tmp, *iBuf, sizeof(uint8_t));
        *iBuf += sizeof(uint8_t);
        
        oVal |= tmp;
        oVal = oVal << 8;
        
        memcpy(&tmp, *iBuf, sizeof(uint8_t));
        *iBuf += sizeof(uint8_t);
        
        oVal |= tmp;
        oVal = oVal << 8;
        
        memcpy(&tmp, *iBuf, sizeof(uint8_t));
        *iBuf += sizeof(uint8_t);
        
        oVal |= tmp;
    }

    void Write(uint8_t **iBuf, bool iVal)
    {
        uint8_t boolVal = 0;
        if (iVal)
            boolVal = 1;

        memcpy(*iBuf, &boolVal, sizeof(uint8_t));
        *iBuf += sizeof(uint8_t);
    }

    void Read(uint8_t **iBuf, bool &oVal)
    {
        uint8_t boolVal = 0;
        memcpy(&boolVal, *iBuf, sizeof(uint8_t));
        *iBuf += sizeof(uint8_t);
        
        if (boolVal)
            oVal = true;
    }

    void Write(uint8_t **iBuf, const std::string &iVal)
    {
        if (iVal.length() == 0)
            return;
        
        memcpy(*iBuf, iVal.c_str(), iVal.length());
        *iBuf += iVal.length();
    }
    
    void Read(uint8_t **iBuf, uint32_t iLength, std::string &oVal)
    {
        if (iLength == 0)
            return;
        
        // Create a temp buffer of length plus one for the null termination
        //
        uint8_t *strBuf = new uint8_t[iLength + 1];
        
        if (strBuf)
        {
            memset(strBuf, 0, iLength + 1);
            memcpy(strBuf, *iBuf, iLength);
            *iBuf += iLength;
            
            std::string tmpStr((char *)strBuf);
            oVal = tmpStr;
        }
        delete [] strBuf;
    }
    
    void WriteBuf(uint8_t **iBuf, uint8_t *iVal, int64_t iSize)
    {
        memcpy(*iBuf, iVal, iSize);
        *iBuf += iSize;
    }

    void ReadBuf(uint8_t **iBuf, uint8_t *oVal, int64_t iSize)
    {
        memcpy(oVal, *iBuf, iSize);
        *iBuf += iSize;
    }

    void WriteBER4(uint8_t **iBuf, int32_t iVal)
    {
        uint8_t tmp = 0x83;
        
        // First byte is BER 0x83
        //
        memcpy(*iBuf, &tmp, sizeof(uint8_t));
        *iBuf += sizeof(uint8_t);
        
        tmp = (iVal & 0x00FF0000) >> 16;
        memcpy(*iBuf, &tmp, sizeof(uint8_t));
        *iBuf += sizeof(uint8_t);
        
        tmp = (iVal & 0x0000FF00) >> 8;
        memcpy(*iBuf, &tmp, sizeof(uint8_t));
        *iBuf += sizeof(uint8_t);

        tmp = (iVal & 0x00000000FF);
        memcpy(*iBuf, &tmp, sizeof(uint8_t));
        *iBuf += sizeof(uint8_t);
    }
    
    void ReadBER4(uint8_t **iBuf, int32_t &oVal)
    {
        uint8_t tmp;
        
        // First byte is BER 0x83
        //
        memcpy(&tmp, *iBuf, sizeof(uint8_t));
        *iBuf += sizeof(uint8_t);

        // Don't use the 0x83 hex values
        
        memcpy(&tmp, *iBuf, sizeof(uint8_t));
        *iBuf += sizeof(uint8_t);

        oVal = tmp;
        oVal = oVal << 8;
        
        memcpy(&tmp, *iBuf, sizeof(uint8_t));
        *iBuf += sizeof(uint8_t);

        oVal |= tmp;
        oVal = oVal << 8;

        memcpy(&tmp, *iBuf, sizeof(uint8_t));
        *iBuf += sizeof(uint8_t);

        oVal |= tmp;
    }

    void WriteBER5(uint8_t **iBuf, int32_t iVal)
    {
        uint8_t tmp = 0x84;
        
        // First byte is BER 0x84
        //
        memcpy(*iBuf, &tmp, sizeof(uint8_t));
        *iBuf += sizeof(uint8_t);
        
        tmp = (iVal & 0xFF000000) >> 24;
        memcpy(*iBuf, &tmp, sizeof(uint8_t));
        *iBuf += sizeof(uint8_t);

        tmp = (iVal & 0x00FF0000) >> 16;
        memcpy(*iBuf, &tmp, sizeof(uint8_t));
        *iBuf += sizeof(uint8_t);
        
        tmp = (iVal & 0x0000FF00) >> 8;
        memcpy(*iBuf, &tmp, sizeof(uint8_t));
        *iBuf += sizeof(uint8_t);
        
        tmp = (iVal & 0x00000000FF);
        memcpy(*iBuf, &tmp, sizeof(uint8_t));
        *iBuf += sizeof(uint8_t);
    }
    
    void ReadBER5(uint8_t **iBuf, int32_t &oVal)
    {
        uint8_t tmp;
        
        // First byte is BER 0x84
        //
        memcpy(&tmp, *iBuf, sizeof(uint8_t));
        *iBuf += sizeof(uint8_t);
        
        // Don't use the 0x84 hex values
        
        memcpy(&tmp, *iBuf, sizeof(uint8_t));
        *iBuf += sizeof(uint8_t);
        
        oVal = tmp;
        oVal = oVal << 8;

        memcpy(&tmp, *iBuf, sizeof(uint8_t));
        *iBuf += sizeof(uint8_t);
        
        oVal |= tmp;
        oVal = oVal << 8;
        
        memcpy(&tmp, *iBuf, sizeof(uint8_t));
        *iBuf += sizeof(uint8_t);
        
        oVal |= tmp;
        oVal = oVal << 8;
        
        memcpy(&tmp, *iBuf, sizeof(uint8_t));
        *iBuf += sizeof(uint8_t);
        
        oVal |= tmp;
    }

} // namespace SMPTE_SYNC
