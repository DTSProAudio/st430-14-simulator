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

/**
 * Utilitities.
 *
 * @file Utils.h
 */

#ifndef __SMPTE_SYNC__UTILS__
#define __SMPTE_SYNC__UTILS__

#include <iostream>
#include <sstream>
#include <iomanip>      // std::setfill, std::setw
#include <stdint.h>

#ifdef MAC_VERSION
#include <CoreAudio/CoreAudioTypes.h>
#include <AudioToolbox/AudioConverter.h>
#endif

#include "DataTypes.h"

namespace SMPTE_SYNC
{
    
    namespace UTILS
    {
        /**
         *
         * @brief ConverterInt24Float32 class implements conversion routines to convert a 24-bit fixed point buffer to a 32-bit floating point buffer
         *
         */
        class ConverterInt24Float32
        {
        public:

            /**
             * Constructor
             *
             * @param iSampleRate is the sample rate of the buffer that needs to be converted
             *
             */
            ConverterInt24Float32(int32_t iSampleRate);
            
            /// Destructor
            ~ConverterInt24Float32();
            
            /**
             * Int24 to Float32 conversion
             *
             * @param iBuf is the buffer of 24-bit fixed point data
             * @param oBuf is is the buffer of 32-bit floatint point data
             * @param capacity number of samples to convert
             *
             */
            void ConvertInt24ToFloat(const uint8_t *iBuf, float *oBuf, size_t capacity);

            /**
             * Float32 to Int24 conversion
             *
             * @param iBuf is is the buffer of 32-bit floatint point data
             * @param oBuf is the buffer of 24-bit fixed point data
             * @param capacity number of samples to convert
             *
             */
            void ConvertFloatToInt24(const float *iBuf, uint8_t *oBuf, size_t capacity);
            
            /// TODO: Move to unit test
            /// Tests the conversion routines
            static void TestConversions(void);

        private:
            
#ifdef MAC_VERSION
            /// Apple API for converting from 24 to 32
            AudioConverterRef               converter24To32_;

            /// Apple API for converting from 32 to 24
            AudioConverterRef               converter32To24_;
            
            /// Apple API for describing the 32 bit data
            AudioStreamBasicDescription     float32Description_;

            /// Apple API for describing the 24 bit data
            AudioStreamBasicDescription     int24Description_;
#endif            
            
            /// Number of bytes per float == 4
            const int32_t bytesPerSampleFloat_;

            /// Number of bytes per 24-bit fixed point == 3
            const int32_t bytesPerSampleInt24_;

            /// Sample rate being converted
            const int32_t sampleRate_;

        };
    }  // namespace UTILS
    
}  // namespace SMPTE_SYNC

#endif /* defined(__SMPTE_SYNC__UTILS__) */

