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
//  SerializationUtils_Test.cpp
//
//

#include "SerializationUtils_Test.h"
#include "gtest/gtest.h"

#include "boost/random.hpp"

#include "MessageFactory.h"
#include "DCS_Message.h"

using namespace SMPTE_SYNC;
using namespace std;

void gen_random_string(char *s, const int len) {
    static const char alphanum[] =
    "0123456789"
    "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
    "abcdefghijklmnopqrstuvwxyz";
    
    for (int i = 0; i < len; ++i) {
        s[i] = alphanum[rand() % (sizeof(alphanum) - 1)];
    }
    
    s[len] = 0;
}

TEST(SerializationUtils_Test, SerializationUtils_Test_Case1)
{
    uint8_t *buf = new uint8_t[1024];
    uint8_t *tmp = buf;
    
    std::time_t now = std::time(0);
    boost::random::mt19937 gen{static_cast<std::uint32_t>(now)};

    for (uint8_t writeVal = 0; writeVal < UINT8_MAX; writeVal++)
    {
        uint8_t readVal = 0;
        
        tmp = buf;
        Write(&tmp, writeVal);

        tmp = buf;
        Read(&tmp, readVal);
        ASSERT_EQ(writeVal, readVal);
    }

    for (int8_t writeVal = 0; writeVal < INT8_MAX; writeVal++)
    {
        int8_t readVal = 0;

        tmp = buf;
        Write(&tmp, writeVal);
        
        tmp = buf;
        Read(&tmp, readVal);
        ASSERT_EQ(writeVal, readVal);
    }


    for (uint32_t i = 0; i < 1000; i++)
    {
        uint32_t writeVal = gen();
        uint32_t readVal = 0;
        
        tmp = buf;
        Write(&tmp, writeVal);
        
        tmp = buf;
        Read(&tmp, readVal);
        ASSERT_EQ(writeVal, readVal);
    }

    for (uint32_t i = 0; i < 1000; i++)
    {
        int32_t writeVal = gen();
        int32_t readVal = 0;
        
        tmp = buf;
        Write(&tmp, writeVal);
        
        tmp = buf;
        Read(&tmp, readVal);
        ASSERT_EQ(writeVal, readVal);
    }

    for (uint32_t i = 0; i < 1000; i++)
    {
        uint64_t writeVal = gen();
        uint64_t readVal = 0;
        
        tmp = buf;
        Write(&tmp, writeVal);
        
        tmp = buf;
        Read(&tmp, readVal);
        ASSERT_EQ(writeVal, readVal);
    }
    
    for (uint32_t i = 0; i < 1000; i++)
    {
        int64_t writeVal = gen();
        int64_t readVal = 0;
        
        tmp = buf;
        Write(&tmp, writeVal);
        
        tmp = buf;
        Read(&tmp, readVal);
        ASSERT_EQ(writeVal, readVal);
    }

    {
        bool writeVal = true;
        bool readVal = true;
        
        tmp = buf;
        Write(&tmp, writeVal);
        
        tmp = buf;
        Read(&tmp, readVal);
        ASSERT_EQ(writeVal, readVal);

        writeVal = false;
        readVal = false;
        
        tmp = buf;
        Write(&tmp, writeVal);
        
        tmp = buf;
        Read(&tmp, readVal);
        ASSERT_EQ(writeVal, readVal);
    }

    for (uint32_t i = 0; i < 1000; i++)
    {
        char* randomString = new char[25];
        gen_random_string(randomString, 24);
        
        std::string writeVal(randomString);
        std::string readVal = "";
        int32_t strLen = static_cast<int32_t>(writeVal.length());
        
        tmp = buf;
        Write(&tmp, writeVal);
        
        tmp = buf;
        Read(&tmp, strLen, readVal);
        ASSERT_EQ(writeVal, readVal);
        delete [] randomString;
    }
    
    for (uint32_t i = 0; i < 1000; i++)
    {
        int32_t bufSize = 25;
        uint8_t* randomStringWrite = new uint8_t[bufSize + 1];
        uint8_t* randomStringRead = new uint8_t[bufSize + 1];
        gen_random_string((char*)randomStringWrite, bufSize);
        
        tmp = buf;
        WriteBuf(&tmp, randomStringWrite, bufSize);
        
        tmp = buf;
        ReadBuf(&tmp, randomStringRead, bufSize);
        int isEqual = memcmp(randomStringWrite, randomStringRead, bufSize);
        ASSERT_EQ(isEqual, 0);
        delete [] randomStringWrite;
        delete [] randomStringRead;
    }

    for (int32_t i = 0; i < 10; i++)
    {
        int32_t writeVal = i;
        int32_t readVal = 0;
        
        tmp = buf;
        WriteBER4(&tmp, writeVal);
        
        tmp = buf;
        ReadBER4(&tmp, readVal);
        ASSERT_EQ(writeVal, readVal);
    }

    delete [] buf;
}

