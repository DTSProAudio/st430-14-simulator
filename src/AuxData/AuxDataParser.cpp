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

#include "AuxDataParser.h"

#include <fstream>
#include <sstream>
#include <iostream>
#include <string>

#include "Logger.h"

#ifdef USE_ASDCP
#include "AS_DCP.h"
#include "KLV.h"
#endif

namespace SMPTE_SYNC
{
    AuxDataParser::AuxDataParser(int32_t iStartFrame
                                 , int32_t iEndFrame
                                 , const std::string &iAuxDataFilePath) :
          path_(iAuxDataFilePath)
        , startFrame_(iStartFrame)
        , endFrame_(iEndFrame)
    {
        SMPTE_SYNC_LOG << "AuxDataParser::AuxDataParser";
        SMPTE_SYNC_LOG << "iAuxDataFilePath = " << iAuxDataFilePath;
    }
    
    AuxDataParser::~AuxDataParser()
    {
        SMPTE_SYNC_LOG << "AuxDataParser::~AuxDataParser";

    }

    int32_t AuxDataParser::GetStartFrame(void)
    {
        return startFrame_;
    }
    
    int32_t AuxDataParser::GetEndFrame(void)
    {
        return endFrame_;
    }

    bool AuxDataParser::Open(void)
    {
        bool success = true;

#ifdef USE_ASDCP
        if (!ASDCP_SUCCESS(r.OpenRead(path_)))
            return false;
            
        if (!ASDCP_SUCCESS(f.OpenRead(path_)))
            return false;
#endif

        return success;
    }

    bool AuxDataParser::Close(void)
    {
        bool closedFileReader = true;
        bool closedMXFReader = true;
        
#ifdef USE_ASDCP
        if (!ASDCP_SUCCESS(r.Close()))
            closedFileReader = false;
        
        if (!ASDCP_SUCCESS(f.Close()))
            closedMXFReader = false;
#endif
        
        return closedFileReader && closedMXFReader;
    }

    bool AuxDataParser::GetDataItem(int32_t iItemNumber
                                    , uint8_t **oDataItem
                                    , uint32_t &oDataItemSize)
    {
        bool success = true;

        // The MXF file indexes frames from 0
        // Since the MXF is placed on a timeline based on startFrame_ and endFrame_
        // We need to offset to the index used by the MXF file
        //
        iItemNumber -= startFrame_;

        if (iItemNumber < 0)
            return false;
        
#ifdef USE_ASDCP
        Kumu::fpos_t fileOffset;
        
        i8_t temporalOffset;
        i8_t keyFrameOffset;

        if (ASDCP_SUCCESS(r.LocateFrame(iItemNumber, fileOffset, temporalOffset, keyFrameOffset)))
        {
            /* seek to the edit unit offset */
            f.Seek(fileOffset);
            
            /* temporary buffer to store the K and L of the KLV */
            
            const ui32_t READ_BUF_SZ = 32;
            byte_t readBuf[READ_BUF_SZ];
            
            /* read the K and L */
            
            ui32_t readSz;
            
            ASDCP::Result_t result = f.Read(readBuf, READ_BUF_SZ, &readSz);
            
            if (ASDCP_FAILURE(result))
            {
                SMPTE_SYNC_LOG << "AuxDataParser::GetDataItem Failed to read iItemNumber = " << iItemNumber;
                return false;
            }
            
            /* did we read enough for at least one K and L */
            
            if (readSz < (ASDCP::SMPTE_UL_LENGTH + 1))
            {
                SMPTE_SYNC_LOG << "AuxDataParser::GetDataItem Failed to read EOF before K and L iItemNumber = " << iItemNumber;
                return false;
            }
            
            /* confirm the K looks like a SMPTE UL */
            
            if (memcmp(readBuf, ASDCP::SMPTE_UL_START, 4) != 0)
            {
                SMPTE_SYNC_LOG << "AuxDataParser::GetDataItem Failed to read K is not a SMPTE UL iItemNumber = " << iItemNumber;
                return false;
            }
            
            /* read the length of V */
            
            ui64_t valueLength;
            
            if (!Kumu::read_BER(readBuf + ASDCP::SMPTE_UL_LENGTH, &valueLength))
            {
                SMPTE_SYNC_LOG << "AuxDataParser::GetDataItem Failed to read Bad BER length iItemNumber = " << iItemNumber;
                return false;
            }
            
            /* total KLV length */
            
            ui32_t klvLength = static_cast<ui32_t>(ASDCP::SMPTE_UL_LENGTH + Kumu::BER_length(readBuf + ASDCP::SMPTE_UL_LENGTH) + valueLength);
            
            uint8_t *klvBuffer = new uint8_t[klvLength];
            
            f.Seek(fileOffset);
            
            result = f.Read(klvBuffer, klvLength, &readSz);

            if (klvLength != readSz)
            {
                SMPTE_SYNC_LOG << "AuxDataParser::GetDataItem Failed to read klvLength != readSz iItemNumber = "
                << iItemNumber
                << " klvLength = " << klvLength
                << " readSz = " << readSz;
                
                return false;
            }

            if (ASDCP_FAILURE(result))
            {
                SMPTE_SYNC_LOG << "AuxDataParser::GetDataItem Failed to read iItemNumber = " << iItemNumber;
                return false;
            }
            
            *oDataItem = klvBuffer;
            oDataItemSize = klvLength;
        }
        else
            success = false;
#endif 
        
        return success;
    }

}  // namespace SMPTE_SYNC
