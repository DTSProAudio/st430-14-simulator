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

#include "AuxData.h"

#include <assert.h>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>

#include "Logger.h"
#include "tinyxml.h"
#include "SerializationUtils.h"

namespace SMPTE_SYNC
{
 
    /**
     *
     * Writes or serializes a PackKey C++ object into a byte stream
     *
     * @param oBuf is the buffer being written. Note that this pointer is moved as it is written
     * @param iVal is PackKey being written to the buffer.
     *
     */
    void Write(uint8_t **oBuf, const PackKey &iVal)
    {
        memcpy(*oBuf, &iVal, iVal.GetSizeInBytes());
        *oBuf += iVal.GetSizeInBytes();
    }
    
    /**
     *
     * Reads or deserializes a byte stream into PackKey C++ object
     *
     * @param iBuf is the buffer being read. Note that this pointer is moved as it is read
     * @param oVal is PackKey being read to the buffer.
     *
     */
    void Read(uint8_t **iBuf, PackKey &oVal)
    {
        memcpy(&oVal, *iBuf, oVal.GetSizeInBytes());
        *iBuf += oVal.GetSizeInBytes();
    }
    
    /**
     *
     * Writes or serializes a UL C++ object into a byte stream
     *
     * @param oBuf is the buffer being written. Note that this pointer is moved as it is written
     * @param iVal is UL being written to the buffer.
     *
     */
    void Write(uint8_t **oBuf, const UL &iVal)
    {
        memcpy(*oBuf, &iVal, iVal.GetSizeInBytes());
        *oBuf += iVal.GetSizeInBytes();
    }
    
    /**
     *
     * Reads or deserializes a byte stream into UL C++ object
     *
     * @param iBuf is the buffer being read. Note that this pointer is moved as it is read
     * @param oVal is UL being read to the buffer.
     *
     */
    void Read(uint8_t **iBuf, UL &oVal)
    {
        memcpy(&oVal, *iBuf, oVal.GetSizeInBytes());
        *iBuf += oVal.GetSizeInBytes();
    }

    AuxDataBlockTransferHeader::AuxDataBlockTransferHeader()
        : length_(0)
        , editUnitRangeStartIndex_(0)
        , editUnitRangeCount_(0)
    {
        //SMPTE_SYNC_LOG << "AuxDataBlockTransferHeader::AuxDataBlockTransferHeader\n";
        length_ += sizeof(editUnitRangeStartIndex_);
        length_ += sizeof(editUnitRangeCount_);

        packKey_.data_[0] = 0x06;
        packKey_.data_[1] = 0x0E;
        packKey_.data_[2] = 0x2B;
        packKey_.data_[3] = 0x34;
        packKey_.data_[4] = 0x02;
        packKey_.data_[5] = 0x7F;
        packKey_.data_[6] = 0x01;
        packKey_.data_[7] = 0x01;
        packKey_.data_[8] = 0x0C;
        packKey_.data_[9] = 0x03;
        packKey_.data_[10] = 0x01;
        packKey_.data_[11] = 0x01;
        packKey_.data_[12] = 0x00;
        packKey_.data_[13] = 0x00;
        packKey_.data_[14] = 0x00;
        packKey_.data_[15] = 0x00;
    }
    
    AuxDataBlockTransferHeader::~AuxDataBlockTransferHeader()
    {
        //SMPTE_SYNC_LOG << "AuxDataBlockTransferHeader::~AuxDataBlockTransferHeader\n";
    }
    
    int32_t AuxDataBlockTransferHeader::GetSizeInBytes(void) const
    {
        int32_t size = 0;
        
        size += packKey_.GetSizeInBytes();
        size += 5; // BER5
        
        size += sizeof(editUnitRangeStartIndex_);
        size += sizeof(editUnitRangeCount_);
        
        return size;
    }

    bool AuxDataBlockTransferHeader::read(uint8_t **iBuffer)
    {
        Read(iBuffer, packKey_);
        ReadBER5(iBuffer, length_);
        
        Read(iBuffer, editUnitRangeStartIndex_);
        Read(iBuffer, editUnitRangeCount_);
        
        return true;
    }
    
    bool AuxDataBlockTransferHeader::write(uint8_t **oBuffer)
    {
        Write(oBuffer, packKey_);
        WriteBER5(oBuffer, length_);
        
        Write(oBuffer, editUnitRangeStartIndex_);
        Write(oBuffer, editUnitRangeCount_);
        
        return true;
    }

    AuxDataBlock::AuxDataBlock() :
      packKey_()
    , length_(0)
    , editUnitIndex_(0)
    , editUnitRateNumerator_(0)
    , editUnitRateDenominator_(0)
    , sourceDataEssenceCodingUL_()
    , sourceDataItemLength_(0)
    , sourceDataItem_(nullptr)
    , sourceCryptographicContextLength_(0)
    , sourceCryptographicContext_(nullptr)
    {
        packKey_.data_[0] = 0x06;
        packKey_.data_[1] = 0x0E;
        packKey_.data_[2] = 0x2B;
        packKey_.data_[3] = 0x34;
        packKey_.data_[4] = 0x02;
        packKey_.data_[5] = 0x7F;
        packKey_.data_[6] = 0x01;
        packKey_.data_[7] = 0x01;
        packKey_.data_[8] = 0x0C;
        packKey_.data_[9] = 0x03;
        packKey_.data_[10] = 0x01;
        packKey_.data_[11] = 0x02;
        packKey_.data_[12] = 0x00;
        packKey_.data_[13] = 0x00;
        packKey_.data_[14] = 0x00;
        packKey_.data_[15] = 0x00;
    }
    
    AuxDataBlock::AuxDataBlock(const AuxDataBlock& other)
    {
        packKey_ = other.packKey_;
        length_ = other.length_;
        
        editUnitIndex_ = other.editUnitIndex_;
        editUnitRateNumerator_ = other.editUnitRateNumerator_;
        editUnitRateDenominator_ = other.editUnitRateDenominator_;
        sourceDataEssenceCodingUL_ = other.sourceDataEssenceCodingUL_;

        sourceDataItemLength_ = other.sourceDataItemLength_;
        sourceDataItem_ = new uint8_t[sourceDataItemLength_];
        memcpy(sourceDataItem_, other.sourceDataItem_ ,sourceDataItemLength_);
        
        sourceCryptographicContextLength_ = other.sourceCryptographicContextLength_;
        sourceCryptographicContext_ = new uint8_t[sourceCryptographicContextLength_];
        memcpy(sourceCryptographicContext_, other.sourceCryptographicContext_ ,sourceCryptographicContextLength_);
    }

    AuxDataBlock::~AuxDataBlock()
    {
        delete [] sourceCryptographicContext_;
        delete [] sourceDataItem_;
    }

    bool AuxDataBlock::read(uint8_t **iBuffer)
    {
        Read(iBuffer, packKey_);
        ReadBER5(iBuffer, length_);

        Read(iBuffer, editUnitIndex_);

        Read(iBuffer, editUnitRateNumerator_);
        Read(iBuffer, editUnitRateDenominator_);

        Read(iBuffer, sourceDataEssenceCodingUL_);

        Read(iBuffer, sourceDataItemLength_);
        if (sourceDataItemLength_ > 0)
        {
            sourceDataItem_ = new uint8_t[sourceDataItemLength_];
            ReadBuf(iBuffer, sourceDataItem_, sourceDataItemLength_);
        }
        
        Read(iBuffer, sourceCryptographicContextLength_);
        if (sourceCryptographicContextLength_ > 0)
        {
            sourceCryptographicContext_ = new uint8_t[sourceCryptographicContextLength_];
            ReadBuf(iBuffer, sourceCryptographicContext_, sourceCryptographicContextLength_);
        }
        
        return true;
    }
    
    bool AuxDataBlock::write(uint8_t **oBuffer)
    {
        // Make sure the length is up to date
        //
        // The length_ does not include the packKey_ and length_
        // as in the specificiation
        //
        length_ = this->GetSizeInBytes();
        length_ -= packKey_.GetSizeInBytes();

        // 5 bytes for BER5
        length_ -= 5;
        
        Write(oBuffer, packKey_);
        WriteBER5(oBuffer, length_);
        
        Write(oBuffer, editUnitIndex_);

        Write(oBuffer, editUnitRateNumerator_);
        Write(oBuffer, editUnitRateDenominator_);
        
        Write(oBuffer, sourceDataEssenceCodingUL_);
        
        Write(oBuffer, sourceDataItemLength_);
        if (sourceDataItemLength_ > 0)
        {
            assert(sourceDataItem_ != nullptr);
            WriteBuf(oBuffer, sourceDataItem_, sourceDataItemLength_);
        }
        
        Write(oBuffer, sourceCryptographicContextLength_);
        if (sourceCryptographicContextLength_ > 0)
        {
            assert(sourceCryptographicContext_ != nullptr);
            WriteBuf(oBuffer, sourceCryptographicContext_, sourceCryptographicContextLength_);
        }
        
        return true;
    }

    int32_t AuxDataBlock::GetSizeInBytes(void) const
    {
        int32_t size = 0;
        
        size += packKey_.GetSizeInBytes();

        // 5 bytes for BER5
        size += 5;

        size += sizeof(editUnitIndex_);

        size += sizeof(editUnitRateNumerator_);
        size += sizeof(editUnitRateDenominator_);

        size += sourceDataEssenceCodingUL_.GetSizeInBytes();

        size += sizeof(sourceDataItemLength_);
        size += sourceDataItemLength_;

        size += sizeof(sourceCryptographicContextLength_);
        size += sourceCryptographicContextLength_;

        return size;
    }

}  // namespace SMPTE_SYNC
