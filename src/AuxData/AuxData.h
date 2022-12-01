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

#ifndef AUXDATA_H
#define AUXDATA_H

#include <assert.h>
#include <stdint.h>
#include <cstdio>
#include <cstdlib>
#include <string.h>
#include <string>
#include <algorithm>

namespace SMPTE_SYNC
{
    /**
     * @brief PackKey struct implements data sent via HTTP as part of the AuxDataBlockTransferHeader and AuxDataBlock data structures
     * Defined in the SMPTE ST 430-10:2010 D-Cinema Operations — Auxiliary Content Synchronization Protocol
     *
     * @struct PackKey
     *
     */
    
    typedef struct PackKey
    {
        /// Constructor
        PackKey()
        {
            memset(data_, 0x0, 16);
        }
        
        /**
         *
         * Returns the total size in bytes of the PackKey. The size is fixed at 16 bytes.
         *
         * @return int32_t of 16 bytes.
         *
         */
        int32_t GetSizeInBytes(void) const
        {
            return 16;
        }
        
        /// 16 bytes Array of characters representing the PackKey data structure.
        int8_t data_[16];
    } PackKey;

    /**
     * @brief UL struct implements data sent via HTTP as part of the AuxDataBlock data structure
     * Defined in the SMPTE ST 430-10:2010 D-Cinema Operations — Auxiliary Content Synchronization Protocol
     *
     * @struct UL
     *
     */

    typedef struct UL
    {
        /// Constructor
        UL()
        {
            memset(data_, 0x0, 16);
        }

        /**
         *
         * Returns the total size in bytes of the UL. The size is fixed at 16 bytes.
         *
         * @return int32_t of 16 bytes.
         *
         */
        int32_t GetSizeInBytes(void) const
        {
            return 16;
        }

        
        /**
         *
         * Utility for converting a char to an int for conversion to a hex string
         *
         */
        int char2int(char input)
        {
            if(input >= '0' && input <= '9')
                return input - '0';
            if(input >= 'A' && input <= 'F')
                return input - 'A' + 10;
            if(input >= 'a' && input <= 'f')
                return input - 'a' + 10;
            
            return 0;
        }
        
        /**
         *
         * Utility for converting a null terminated string of "hex" values to a binary array
         *
         */
        void HexStrToBinaryArray(const char* src, char* target)
        {
            while(*src && src[1])
            {
                *(target++) = char2int(*src)*16 + char2int(src[1]);
                src += 2;
            }
        }
        
        /**
         *
         * Returns the total size in bytes of the UL. The size is fixed at 16 bytes.
         *
         * @return int32_t of 16 bytes.
         *
         */
        bool SetFromString(const std::string& iULString)
        {
            bool success = false;

            if (iULString.length() == 32)
            {
                success = true;
                HexStrToBinaryArray(iULString.c_str(), (char*)data_);
            }
            else
            if (iULString.length() == 48)
            {
                success = true;
                std::string ULOnly = iULString;
                ULOnly = ULOnly.substr(std::string("urn:smpte:ul:").length(), ULOnly.length());
                ULOnly.erase(std::remove(ULOnly.begin(), ULOnly.end(), '.'), ULOnly.end());
                
                HexStrToBinaryArray(ULOnly.c_str(), (char*)data_);
            }
            
            return success;
        }

        /// 16 bytes Array of characters representing the UL data structure.
        int8_t data_[16];
    } UL;

    /**
     * @brief AuxDataBlockTransferHeader class implements data sent via HTTP as part of the aux data item transfer protocol.
     * Defined in the SMPTE ST 430-10:2010 D-Cinema Operations — Auxiliary Content Synchronization Protocol
     * The header is sent with head HTTP request. 
     * The editUnitRangeStartIndex_ and editUnitRangeCount_ represent the contents of the payload of the HTTP response.
     *
     */

    class AuxDataBlockTransferHeader
    {
    public:

        /// Constructor
        AuxDataBlockTransferHeader();

        /// Destructor
        ~AuxDataBlockTransferHeader();
        
        /**
         *
         * Returns the total size in bytes of the AuxDataBlockTransferHeader.
         *
         * @return int32_t of a fixed size of the header.
         *
         */
        int32_t GetSizeInBytes(void) const;
        
        /**
         *
         * Reads or deserializes a byte stream into a AuxDataBlockTransferHeader C++ object
         *
         * @param iBuffer is the buffer being read. Note that this pointer is moved as it is read
         * @return true/false if the buffer has been properly read
         *
         */
        bool read(uint8_t **iBuffer);

        /**
         *
         * Writes or serializes a AuxDataBlockTransferHeader C++ object into a byte stream
         *
         * @param iBuffer is the buffer being written. Note that this pointer is moved as it is written
         * @return true/false if the buffer has been properly written
         *
         */
        bool write(uint8_t **oBuffer);
        
        /// Stores the PackKey data of the object
        PackKey         packKey_;
        
        /// Stores the length in bytes of the header
        int32_t         length_;
        
        /// Represents the starting edit unit range of the payload of the HTTP response
        uint32_t        editUnitRangeStartIndex_;

        /// Represents the starting number of edit units returned in the payload of the HTTP response
        uint32_t        editUnitRangeCount_;
    };

    /**
     * @brief AuxDataBlock class implements data sent via HTTP as part of the aux data item transfer protocol.
     * Defined in the SMPTE ST 430-10:2010 D-Cinema Operations — Auxiliary Content Synchronization Protocol
     * The AuxDataBlock is the payload is sent with head HTTP request.
     *
     */

    class AuxDataBlock
    {
    public:

        /// Constructor
        AuxDataBlock();

        /// Copy Constructor
        AuxDataBlock(const AuxDataBlock& other);
        
        /// Destructor
        ~AuxDataBlock();
        
        /**
         *
         * Returns the total size in bytes of the AuxDataBlock.
         *
         * @return int32_t of a variable sized payload
         *
         */
        int32_t GetSizeInBytes(void) const;
        
        /**
         *
         * Reads or deserializes a byte stream into a AuxDataBlock C++ object
         *
         * @param iBuffer is the buffer being read. Note that this pointer is moved as it is read
         * @return true/false if the buffer has been properly read
         *
         */
        bool read(uint8_t **iBuffer);

        /**
         *
         * Writes or serializes a AuxDataBlock C++ object into a byte stream
         *
         * @param iBuffer is the buffer being written. Note that this pointer is moved as it is written
         * @return true/false if the buffer has been properly written
         *
         */
        bool write(uint8_t **oBuffer);
        
        /// Stores the PackKey data of the object.
        PackKey         packKey_;

        /// Stores the length in bytes of the payload. This is a BER5 encoded value.
        int32_t         length_;

        /// Stores Index of the timeline Edit Unit
        uint32_t        editUnitIndex_;
        
        /// Stores Edit rate of the timeline Edit Unit
        int32_t         editUnitRateNumerator_;
        
        /// Stores Edit rate of the timeline Edit Unit
        int32_t         editUnitRateDenominator_;
        
        /// Stores Data Essence Coding UL of the source Aux Data Track File
        UL              sourceDataEssenceCodingUL_;

        /// Stores Length in bytes of the Source Data Item element
        uint64_t        sourceDataItemLength_;

        /// Stores Data Item of the source Aux Data Track File
        uint8_t         *sourceDataItem_;

        /// Stores Length in bytes of the Cryptographic Context Set
        uint64_t        sourceCryptographicContextLength_;

        /// Stores Cryptographic Context Set, if any, associated with the Elements of the Data Item contained in Source Data Item element
        uint8_t         *sourceCryptographicContext_;
    };

}  // namespace SMPTE_SYNC

#endif // AUXDATA_H
