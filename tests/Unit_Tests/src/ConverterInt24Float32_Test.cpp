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

//
//  ConverterInt24Float32_Test.cpp
//
//

#include "ConverterInt24Float32_Test.h"
#include "gtest/gtest.h"

#include "boost/random.hpp"

#include "Utils.h"
#include "Logger.h"

using namespace SMPTE_SYNC;
using namespace std;

TEST(ConverterInt24Float32_Test, ConverterInt24Float32_Test_Case1)
{
    return;
    UTILS::ConverterInt24Float32 converter(48000);
    
    size_t capacity = 1024;
    size_t int24bufSize = capacity * 3;
    
    uint8_t *beginInt24Buf = new uint8_t[int24bufSize];
    uint8_t *endInt24Buf = new uint8_t[int24bufSize];
    float *floatBuf = new float[capacity];
    
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
        
        //converter.ConvertInt24ToFloat(beginInt24Buf, floatBuf, capacity);
        //converter.ConvertFloatToInt24(floatBuf, endInt24Buf, capacity);
        
        int val = 0;
        //val = memcmp(beginInt24Buf, endInt24Buf, int24bufSize);
        //ASSERT_EQ(val, 0);
    }
    
    delete [] beginInt24Buf;
    delete [] endInt24Buf;
    delete [] floatBuf;
}

