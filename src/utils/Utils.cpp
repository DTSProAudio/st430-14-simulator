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

#include "Utils.h"
#include <assert.h>
#include "Logger.h"
#include <math.h>       /* floor */
#include <iostream>
#include <fstream>

#ifdef MAC_VERSION
#include <CoreAudio/CoreAudioTypes.h>
#include <AudioToolbox/AudioConverter.h>
#endif

namespace SMPTE_SYNC
{

namespace UTILS
{
#ifndef MAC_VERSION
    static const double mult_2147483648_ = 2147483648.0;
    static const double div_2147483648_ = 1.0 / 2147483648.0;
    
    static void Float32_To_Int24(void *destinationBuffer,
                                 const void *sourceBuffer,
                                 unsigned int count )
    {
        float *src = (float*)sourceBuffer;
        unsigned char *dest = (unsigned char*)destinationBuffer;
        uint32_t temp = 0;
        
        while( count-- )
        {
            double scaled = (double)(*src) * mult_2147483648_;
            temp = (uint32_t) scaled;
            
            dest[0] = (unsigned char)(temp >> 8);
            dest[1] = (unsigned char)(temp >> 16);
            dest[2] = (unsigned char)(temp >> 24);
            
            src += 1;
            dest += 3;
        }
    }

    static void Int24_To_Float32(void *destinationBuffer,
                                 const void *sourceBuffer,
                                 unsigned int count )
    {
        unsigned char *src = (unsigned char*)sourceBuffer;
        float *dest = (float*)destinationBuffer;
        int32_t temp = 0;
        
        while( count-- )
        {
            temp = (((uint32_t)src[0]) << 8);
            temp = temp | (((uint32_t)src[1]) << 16);
            temp = temp | (((uint32_t)src[2]) << 24);
            
            *dest = (float) ((double)temp * div_2147483648_);
            
            src += 3;
            dest += 1;
        }
    }
#endif
    
    ConverterInt24Float32::ConverterInt24Float32(int32_t iSampleRate) :
          bytesPerSampleFloat_(sizeof(float))
        , bytesPerSampleInt24_(3)
        , sampleRate_(iSampleRate)
    {
#ifdef MAC_VERSION
        float32Description_.mFormatID = kAudioFormatLinearPCM;
        float32Description_.mFormatFlags = kAudioFormatFlagIsFloat | kAudioFormatFlagIsPacked;
        float32Description_.mBitsPerChannel = 8 * bytesPerSampleFloat_;
        float32Description_.mFramesPerPacket = 1;
        float32Description_.mChannelsPerFrame = 1;
        float32Description_.mBytesPerPacket = bytesPerSampleFloat_ * float32Description_.mFramesPerPacket;
        float32Description_.mBytesPerFrame = bytesPerSampleFloat_ * float32Description_.mChannelsPerFrame;
        float32Description_.mSampleRate = iSampleRate;
        
        int24Description_.mFormatID = kAudioFormatLinearPCM;
        int24Description_.mFormatFlags = kLinearPCMFormatFlagIsSignedInteger | kAudioFormatFlagIsPacked;
        int24Description_.mBitsPerChannel = 8 * bytesPerSampleInt24_;
        int24Description_.mFramesPerPacket = 1;
        int24Description_.mChannelsPerFrame = 1;
        int24Description_.mBytesPerPacket = bytesPerSampleInt24_ * int24Description_.mFramesPerPacket;
        int24Description_.mBytesPerFrame = bytesPerSampleInt24_ * int24Description_.mChannelsPerFrame;
        int24Description_.mSampleRate = iSampleRate;

        OSStatus err;
        err = AudioConverterNew(&int24Description_, &float32Description_, &converter24To32_);
        if (err != noErr)
        {
            SMPTE_SYNC_LOG << "AudioConverterNew = " << err;
        }

        err = AudioConverterNew(&float32Description_, &int24Description_, &converter32To24_);
        if (err != noErr)
        {
            SMPTE_SYNC_LOG << "AudioConverterNew = " << err;
        }
#endif
	}
    
    ConverterInt24Float32::~ConverterInt24Float32()
    {
#ifdef MAC_VERSION

        OSStatus err;
        err = AudioConverterDispose(converter24To32_);
        if (err != noErr)
        {
            SMPTE_SYNC_LOG << "AudioConverterDispose = " << err;
        }

        err = AudioConverterDispose(converter32To24_);
        if (err != noErr)
        {
            SMPTE_SYNC_LOG << "AudioConverterDispose = " << err;
        }
#endif
	}

    void ConverterInt24Float32::ConvertInt24ToFloat(const uint8_t *iBuf, float *oBuf, size_t capacity)
    {
#ifdef MAC_VERSION

        OSStatus err;

        UInt32 inSize = static_cast<UInt32>(capacity * bytesPerSampleInt24_);
        UInt32 outSize = static_cast<UInt32>(capacity * sizeof(float));
        
        err = AudioConverterConvertBuffer(converter24To32_, inSize, iBuf, &outSize, oBuf);
        if (err != noErr)
        {
            SMPTE_SYNC_LOG << "AudioConverterConvertBuffer::ConvertInt24ToFloat = " << err;
        }
#else
        Int24_To_Float32(oBuf, iBuf, capacity);
#endif
	}
    
    void ConverterInt24Float32::ConvertFloatToInt24(const float *iBuf, uint8_t *oBuf, size_t capacity)
    {
#ifdef MAC_VERSION

        OSStatus err;
        
        UInt32 inSize = static_cast<UInt32>(capacity * sizeof(float));
        UInt32 outSize = static_cast<UInt32>(capacity * bytesPerSampleInt24_);
        
        err = AudioConverterConvertBuffer(converter32To24_, inSize, iBuf, &outSize, oBuf);
        if (err != noErr)
        {
            SMPTE_SYNC_LOG << "AudioConverterConvertBuffer::ConvertFloatToInt24 = " << err;
        }
#else
        Float32_To_Int24(oBuf, iBuf, capacity);
#endif
	}

    void ConverterInt24Float32::TestConversions(void)
    {
        ConverterInt24Float32 converter(48000);
        
        size_t capacity = 1024;
        size_t int24bufSize = capacity * 3;
        
        uint8_t *beginInt24Buf = new uint8_t[int24bufSize];
        uint8_t *endInt24Buf = new uint8_t[int24bufSize];
        float *floatBuf = new float[capacity];
#ifndef MAC_VERSION
        uint8_t *endInt24Buf1 = new uint8_t[int24bufSize];
        float *floatBuf1 = new float[capacity];
#endif
        
        int64_t counter = 0;
        int iterations = 0x007FFFFF / capacity;
        for (int j = 0; j < iterations; j++)
        {
            memset(beginInt24Buf, 0, int24bufSize);
            memset(endInt24Buf, 0, int24bufSize);
            memset(floatBuf, 0, capacity * sizeof(float));
            
            uint8_t *tmp = beginInt24Buf;
            for (int32_t i = static_cast<int32_t>(j * capacity); i < ((j * capacity) + capacity); i++)
            {
                *tmp++ = (i & 0xFF);
                *tmp++ = ((i & 0xFF00) >> 8);
                *tmp++ = ((i & 0xFF0000) >> 16);
                counter++;
            }
            
            converter.ConvertInt24ToFloat(beginInt24Buf, floatBuf, capacity);
            converter.ConvertFloatToInt24(floatBuf, endInt24Buf, capacity);
            
            int val = 0;
            val = memcmp(beginInt24Buf, endInt24Buf, int24bufSize);
            if (val != 0)
            {
                SMPTE_SYNC_LOG << "convert failed = " << val;
            }
            
#ifndef MAC_VERSION
            Int24_To_Float32(floatBuf1,
                             beginInt24Buf,
                             (unsigned int)capacity);
            Float32_To_Int24(endInt24Buf1,
                             floatBuf1,
                             (unsigned int)capacity);
            
            val = memcmp(floatBuf, floatBuf1, capacity*4);
            if (val != 0)
            {
                SMPTE_SYNC_LOG << "convert 1 failed = " << val;
            }
            
            val = memcmp(beginInt24Buf, endInt24Buf1, int24bufSize);
            if (val != 0)
            {
                SMPTE_SYNC_LOG << "convert 2 failed = " << val;
            }
#endif
        }
        
        delete [] beginInt24Buf;
        delete [] endInt24Buf;
        delete [] floatBuf;
        
#ifndef MAC_VERSION
        delete [] endInt24Buf1;
        delete [] floatBuf1;
#endif
    }

}  // namespace UTILS

}  // namespace SMPTE_SYNC


