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

#ifndef UUID_H
#define UUID_H

#include <stdint.h>
#include <string>

namespace SMPTE_SYNC
{
    /// TODO: Implement a full C++ class to support UUID
    
    /**
     *
     * @brief buffer of uint8_t to hold a UUID
     *
     */
    typedef uint8_t   UUID[16];

    /**
     *
     * @brief Initializes a UUID to all 0s
     *
     */
    void Initialize(UUID iUUID);

    /**
     *
     * @brief Converts a nullptr terminated char buff to a UUID
     *
     * @param str is nullptr terminated char buffer to be converted
     * @param uuid is UUID represented by the string
     * @return true if conversion was successful, false otherwise
     *
     */
    bool Str2UUID(const char *str, UUID oUUID);

    /**
     *
     * @brief Converts a std::string to a UUID
     *
     * @param iUUIDStr is string representation of UUID
     * @param oUUID is UUID represented by the string
     * @return true if conversion was successful, false otherwise
     *
     */
    bool StringToUUID(const std::string &iUUIDStr, UUID oUUID);
    
    /**
     *
     * @brief Compares to UUID objects
     *
     * @param iUUID1 is first UUID to compare
     * @param iUUID2 is second UUID to compare
     * @return true if the UUID objects are equal, false otherwise
     *
     */
    bool IsEqual(const UUID iUUID1, const UUID iUUID2);

    /**
     *
     * @brief Copies a UUID to another UUID
     *
     * @param iSource is the UUID to copy
     * @param oDestination is the destination UUID to copy to
     * @return true if the UUID objects are equal, false otherwise
     *
     */
    bool Copy(const UUID iSource, UUID oDestination);

}  // namespace SMPTE_SYNC

#endif // UUID_H
