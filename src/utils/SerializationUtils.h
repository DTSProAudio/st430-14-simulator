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

#ifndef SERIALIZATIONUTILS_H_INCLUDED
#define SERIALIZATIONUTILS_H_INCLUDED

#include <stdint.h>
#include <string>
#include <vector>
#include "DataTypes.h"

using namespace std;

namespace SMPTE_SYNC {

    /**
     *
     * Writes or serializes a uint8_t into a byte stream
     *
     * @param oBuf is the buffer being written. Note that this pointer is moved as it is written
     * @param iVal is uint8_t being written to the buffer.
     *
     */
    void Write(uint8_t **iBuf, uint8_t iVal);

    /**
     *
     * Reads or deserializes a byte stream into uint8_t
     *
     * @param iBuf is the buffer being read. Note that this pointer is moved as it is read
     * @param oVal is uint8_t being read to the buffer.
     *
     */
    void Read(uint8_t **iBuf, uint8_t &oVal);

    /**
     *
     * Writes or serializes a int8_t into a byte stream
     *
     * @param oBuf is the buffer being written. Note that this pointer is moved as it is written
     * @param iVal is int8_t being written to the buffer.
     *
     */
    void Write(uint8_t **iBuf, int8_t iVal);

    /**
     *
     * Reads or deserializes a byte stream into int8_t
     *
     * @param iBuf is the buffer being read. Note that this pointer is moved as it is read
     * @param oVal is int8_t being read to the buffer.
     *
     */
    void Read(uint8_t **iBuf, int8_t &oVal);

    /**
     *
     * Writes or serializes a uint32_t into a byte stream
     *
     * @param oBuf is the buffer being written. Note that this pointer is moved as it is written
     * @param iVal is uint32_t being written to the buffer.
     *
     */
    void Write(uint8_t **iBuf, uint32_t iVal);

    /**
     *
     * Reads or deserializes a byte stream into uint32_t
     *
     * @param iBuf is the buffer being read. Note that this pointer is moved as it is read
     * @param oVal is uint32_t being read to the buffer.
     *
     */
    void Read(uint8_t **iBuf, uint32_t &oVal);

    /**
     *
     * Writes or serializes a int32_t into a byte stream
     *
     * @param oBuf is the buffer being written. Note that this pointer is moved as it is written
     * @param iVal is int32_t being written to the buffer.
     *
     */
    void Write(uint8_t **iBuf, int32_t iVal);

    /**
     *
     * Reads or deserializes a byte stream into int32_t
     *
     * @param iBuf is the buffer being read. Note that this pointer is moved as it is read
     * @param oVal is int32_t being read to the buffer.
     *
     */
    void Read(uint8_t **iBuf, int32_t &oVal);

    /**
     *
     * Writes or serializes a uint64_t into a byte stream
     *
     * @param oBuf is the buffer being written. Note that this pointer is moved as it is written
     * @param iVal is uint64_t being written to the buffer.
     *
     */
    void Write(uint8_t **iBuf, uint64_t iVal);

    /**
     *
     * Reads or deserializes a byte stream into uint64_t
     *
     * @param iBuf is the buffer being read. Note that this pointer is moved as it is read
     * @param oVal is uint64_t being read to the buffer.
     *
     */
    void Read(uint8_t **iBuf, uint64_t &oVal);

    /**
     *
     * Writes or serializes a int64_t into a byte stream
     *
     * @param oBuf is the buffer being written. Note that this pointer is moved as it is written
     * @param iVal is int64_t being written to the buffer.
     *
     */
    void Write(uint8_t **iBuf, int64_t iVal);

    /**
     *
     * Reads or deserializes a byte stream into int64_t
     *
     * @param iBuf is the buffer being read. Note that this pointer is moved as it is read
     * @param oVal is int64_t being read to the buffer.
     *
     */
    void Read(uint8_t **iBuf, int64_t &oVal);

    /**
     *
     * Writes or serializes a int64_t into a byte stream
     *
     * @param oBuf is the buffer being written. Note that this pointer is moved as it is written
     * @param iVal is int64_t being written to the buffer.
     *
     */
    void Write(uint8_t **iBuf, bool iVal);

    /**
     *
     * Reads or deserializes a byte stream into bool
     *
     * @param iBuf is the buffer being read. Note that this pointer is moved as it is read
     * @param oVal is bool being read to the buffer.
     *
     */
    void Read(uint8_t **iBuf, bool &oVal);

    /**
     *
     * Writes or serializes a std::string into a byte stream
     *
     * @param oBuf is the buffer being written. Note that this pointer is moved as it is written
     * @param iVal is std::string being written to the buffer.
     *
     */
    void Write(uint8_t **iBuf, const std::string &iVal);

    /**
     *
     * Reads or deserializes a byte stream into std::string
     *
     * @param iBuf is the buffer being read. Note that this pointer is moved as it is read
     * @param uint32_t is length of the string being read
     * @param oVal is string being read from the buffer.
     *
     */
    void Read(uint8_t **iBuf, uint32_t iLength, std::string &oVal);

    /**
     *
     * Writes or serializes a buffer of uint8_t into a byte stream
     *
     * @param oBuf is the buffer being written. Note that this pointer is moved as it is written
     * @param iVal is buffer of uint8_t being written to the buffer.
     * @param iSize is the number of bytes written to the buffer.
     *
     */
    void WriteBuf(uint8_t **iBuf, uint8_t *iVal, int64_t iSize);

    /**
     *
     * Reads or deserializes a byte stream into buffer of uint8_t
     *
     * @param iBuf is the buffer being read. Note that this pointer is moved as it is read
     * @param oVal is buffer of uint8_t being read from the buffer.
     * @param iSize is the number of bytes written to the buffer.
     *
     */
    void ReadBuf(uint8_t **iBuf, uint8_t *oVal, int64_t iSize);

    /**
     *
     * Writes or serializes a BER4 iVal into a byte stream
     *
     * @param oBuf is the buffer being written. Note that this pointer is moved as it is written
     * @param iVal is int32_t being written to the buffer.
     *
     */
    void WriteBER4(uint8_t **iBuf, int32_t iVal);

    /**
     *
     * Reads or deserializes a byte stream into a BER4 int32_t
     *
     * @param iBuf is the buffer being read. Note that this pointer is moved as it is read
     * @param oVal is int32_t being read to the buffer.
     *
     */
    void ReadBER4(uint8_t **iBuf, int32_t &oVal);

    /**
     *
     * Writes or serializes a BER5 iVal into a byte stream
     *
     * @param oBuf is the buffer being written. Note that this pointer is moved as it is written
     * @param iVal is int32_t being written to the buffer.
     *
     */
    void WriteBER5(uint8_t **iBuf, int32_t iVal);
    
    /**
     *
     * Reads or deserializes a byte stream into a BER5 int32_t
     *
     * @param iBuf is the buffer being read. Note that this pointer is moved as it is read
     * @param oVal is int32_t being read to the buffer.
     *
     */
    void ReadBER5(uint8_t **iBuf, int32_t &oVal);

} // namespace SMPTE_SYNC

#endif  // SERIALIZATIONUTILS_H_INCLUDED
