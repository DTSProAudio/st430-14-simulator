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

#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

#include "sync.h"

namespace SMPTE_SYNC
{
    bool syncPacket::WriteSyncPacket(uint8_t *buffer, uint32_t *count)
    {
        uint8_t lcount;
        *count = 0;
        
        if (!WriteUInt16(marker_, buffer, &lcount, true))
        {
            return false;
        }
        
        *count += lcount;
        buffer += lcount;
        
        length_ = 42 + extensionLength_;
        
        if (!WriteUInt16(length_, buffer, &lcount, false))
        {
            return false;
        }
        
        *count += lcount;
        buffer += lcount;
        
        if (!WriteUInt16(flags_, buffer, &lcount, false))
        {
            return false;
        }
        
        *count += lcount;
        buffer += lcount;
        
        if (!WriteUInt32(timelineEditUnitIndex_, buffer, &lcount, false))
        {
            return false;
        }
        
        *count += lcount;
        buffer += lcount;
        
        if (!WriteUInt32(playoutID_, buffer, &lcount, false))
        {
            return false;
        }
        
        *count += lcount;
        buffer += lcount;
        
        if (!WriteUInt16(editUnitDuration_, buffer, &lcount, false))
        {
            return false;
        }
        
        *count += lcount;
        buffer += lcount;
        
        if (!WriteUInt32(sampleDurationEnum_, buffer, &lcount, false))
        {
            return false;
        }
        
        *count += lcount;
        buffer += lcount;
        
        if (!WriteUInt32(sampleDurationDenom_, buffer, &lcount, false))
        {
            return false;
        }
        
        *count += lcount;
        buffer += lcount;
        
        if (!WriteInt32(primaryPictureOutputOffset_, buffer, &lcount, false))
        {
            return false;
        }
        
        *count += lcount;
        buffer += lcount;
        
        if (!WriteUInt32(primaryPictureScreenOffset_, buffer, &lcount, false))
        {
            return false;
        }
        
        *count += lcount;
        buffer += lcount;
        
        if (!WriteUInt32(primaryPictureTrackFileEditUnitIndex_, buffer, &lcount, false))
        {
            return false;
        }
        
        *count += lcount;
        buffer += lcount;
        
        if (!WriteUUID(primaryPictureTrackFileUUID_, buffer, &lcount, false))
        {
            return false;
        }
        
        *count += lcount;
        buffer += lcount;
        
        if (!WriteUInt32(primarySoundTrackFileEditUnitIndex_, buffer, &lcount, false))
        {
            return false;
        }
        
        *count += lcount;
        buffer += lcount;
        
        if (!WriteUUID(primarySoundTrackFileUUID_, buffer, &lcount, false))
        {
            return false;
        }
        
        *count += lcount;
        buffer += lcount;
        
        if (!WriteUUID(compositionPlaylistUUID_, buffer, &lcount, false))
        {
            return false;
        }
        
        *count += lcount;
        buffer += lcount;
        
        for (int i = 0; i < extensionLength_; i++)
        {
            if (!WriteUInt16(extension_[i], buffer, &lcount, false))
            {
                return false;
            }
            
            *count += lcount;
            buffer += lcount;
        }
        
        return true;
    }

    void syncPacket::SetLength(uint16_t length)
    {
        length_ = length;
    }

    void syncPacket::SetStatus(uint8_t status)
    {
        if (status < NSTATES)
        {
            flags_ = status;
        }
    }

    void syncPacket::SetTimelineEditUnitIndex(uint32_t timeIndex)
    {
        timelineEditUnitIndex_ = timeIndex;
    }

    void syncPacket::SetPlayoutID(uint32_t playoutID)
    {
        playoutID_ = playoutID;
    }

    void syncPacket::SetEditUnitDuration(uint16_t iDuration)
    {
        editUnitDuration_ = iDuration;
    }

    void syncPacket::SetSampleDuration(uint32_t enumerator, uint32_t denominator)
    {
        sampleDurationEnum_ = enumerator;
        sampleDurationDenom_ = denominator;
    }

    void syncPacket::SetPrimaryPictureOutputOffset(int32_t iOffset)
    {
        primaryPictureOutputOffset_ = iOffset;
    }

    void syncPacket::SetPrimaryPictureScreenOffset(uint32_t iOffset)
    {
        primaryPictureScreenOffset_ = iOffset;
    }

    void syncPacket::SetPrimaryPictureTrackFileEditUnitIndex(uint32_t iIndex)
    {
        primaryPictureTrackFileEditUnitIndex_ = iIndex;
    }

    bool syncPacket::SetPrimaryPictureTrackFileUUID(UUID uuid)
    {
        return Copy(uuid, primaryPictureTrackFileUUID_);
    }

    bool syncPacket::SetPrimaryPictureTrackFileUUID(char *uuid)
    {
        if (!Str2UUID(uuid,primaryPictureTrackFileUUID_))
        {
            return false;
        }

        return true;
    }

    void syncPacket::SetPrimarySoundTrackFileEditUnitIndex(uint32_t iIndex)
    {
        primarySoundTrackFileEditUnitIndex_ = iIndex;
    }

    bool syncPacket::SetPrimarySoundTrackFileUUID(UUID uuid)
    {
        return Copy(uuid, primarySoundTrackFileUUID_);
    }

    bool syncPacket::SetPrimarySoundTrackFileUUID(char *uuid)
    {
        if (!Str2UUID(uuid,primarySoundTrackFileUUID_))
        {
            return false;
        }
        
        return true;
    }

    bool syncPacket::SetCompositionPlaylistUUID(UUID uuid)
    {
        return Copy(uuid, compositionPlaylistUUID_);
    }

    bool syncPacket::SetCompositionPlaylistUUID(char *uuid)
    {
        if (!Str2UUID(uuid,compositionPlaylistUUID_))
        {
            return false;
        }
        
        return true;
    }

    bool syncPacket::SetExtension(uint16_t *extension, uint16_t extensionLength)
    {
        if (extension_ != nullptr)
        {
            return false;
        }

        extension_ = new uint16_t[extensionLength];
        
        if (extension_ == nullptr)
        {
            return false;
        }
        
        extensionLength_ = extensionLength;
        
        for (int i = 0; i < extensionLength; i++)
        {
            extension_[i] = extension[i];
        }
        
        return true;
    }

    // Section 5.2 of [ADSSTP]

    bool WriteUInt16(uint16_t value, uint8_t *buffer, uint8_t *count, bool first)
    {
        uint32_t lval, tval;
        
        lval = (uint32_t) value;
        
        if (first)
        {
            lval |= 0x010000;
        }
        
        tval = ((~lval) + 1) & 0x00FFFFFF;
        
        buffer[0] = (uint8_t) (lval & 0xFF);
        buffer[3] = (uint8_t) (tval & 0xFF);
        
        buffer[1] = (uint8_t) ((lval >> 8) & 0xFF);
        buffer[4] = (uint8_t) ((tval >> 8) & 0xFF);

        buffer[2] = (uint8_t) ((lval >> 16) & 0xFF);
        buffer[5] = (uint8_t) ((tval >> 16) & 0xFF);
        
        *count = 6;
        
        return true;
    }

    // Section 5.2 of [ADSSTP]

    bool WriteUInt32(uint32_t value, uint8_t *buffer, uint8_t *count, bool first)
    {
        uint8_t   lcount;
        
        if (!WriteUInt16((uint16_t)((value >> 16) & 0xFFFF), buffer, &lcount,first))
        {
            return false;
        }
        
        *count = lcount;
        
        if (!WriteUInt16((uint16_t)(value & 0xFFFF), buffer+lcount, &lcount,false))
        {
            return false;
        }
        
        *count += lcount;
        
        return true;
    }

    bool WriteInt32(int32_t value, uint8_t *buffer, uint8_t *count, bool first)
    {
        return WriteUInt32((uint32_t) value, buffer,count,first);
    }

    bool WriteUUID(UUID uuid, uint8_t *buffer, uint8_t *count, bool first)
    {
        uint16_t val;
        uint8_t  lcount;
        
        *count = 0;

        for (int i = 0; i < 16; i += 2)
        {
            val = (uuid[i+1] | (uuid[i] << 8)) & 0xFFFF;
            
            if (!WriteUInt16(val, buffer, &lcount, false))
            {
                return false;
            }
            
            *count += lcount;
            buffer += lcount;
        }
        
        return true;
    }

    // NOTE on reading values from a byte stream containing a valid syncPacket
    // See section 5.3.1.1 Structure from the SMPTE ST 430-14:2015
    // D-Cinema Operations – Digital Sync Signal and Aux Data Transfer Protocol document
    //
    // The code is reading a 16 bit value from a 24-bit fixed pointed sample
    // It is also skipping the next 16 bit value (the 2s complement) from the 24-bit fixed point sample
    //

    void ReadUInt16(uint8_t **iBuf, uint16_t &oVal)
    {
        memcpy(&oVal, *iBuf, sizeof(uint16_t));
        
        // See above NOTE
        *iBuf += (3 + 3);
    }

    void ReadUInt32(uint8_t **iBuf, uint32_t &oVal)
    {
        uint16_t upper;
        uint16_t lower;
        
        memcpy(&upper, *iBuf, sizeof(uint16_t));

        // See above NOTE
        *iBuf += (3 + 3);
        
        memcpy(&lower, *iBuf, sizeof(uint16_t));

        // See above NOTE
        *iBuf += (3 + 3);
        
        oVal = (upper << 16) | lower;
    }

    void ReadInt32(uint8_t **iBuf, int32_t &oVal)
    {
        uint32_t tmp;
        ReadUInt32(iBuf, tmp);
        oVal = tmp;
    }

    void ReadUUID(uint8_t **iBuf, UUID &oVal)
    {
        memcpy(&(oVal[0]), *iBuf, sizeof(uint16_t));

        // See above NOTE
        *iBuf += (3 + 3);
        
        memcpy(&(oVal[2]), *iBuf, sizeof(uint16_t));

        // See above NOTE
        *iBuf += (3 + 3);
        
        memcpy(&(oVal[4]), *iBuf, sizeof(uint16_t));
        
        // See above NOTE
        *iBuf += (3 + 3);

        memcpy(&(oVal[6]), *iBuf, sizeof(uint16_t));

        // See above NOTE
        *iBuf += (3 + 3);

        memcpy(&(oVal[8]), *iBuf, sizeof(uint16_t));

        // See above NOTE
        *iBuf += (3 + 3);

        memcpy(&(oVal[10]), *iBuf, sizeof(uint16_t));

        // See above NOTE
        *iBuf += (3 + 3);

        memcpy(&(oVal[12]), *iBuf, sizeof(uint16_t));

        // See above NOTE
        *iBuf += (3 + 3);

        memcpy(&(oVal[14]), *iBuf, sizeof(uint16_t));

        // See above NOTE
        *iBuf += (3 + 3);
    }

}  // namespace SMPTE_SYNC