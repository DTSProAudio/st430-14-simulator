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
//  SyncSignal_Test.cpp
//
//

#include "SyncSignal_Test.h"
#include "gtest/gtest.h"

#include "boost/random.hpp"
#include "boost/generator_iterator.hpp"
#include "boost/date_time/posix_time/posix_time.hpp"
#include "boost/date_time/posix_time/posix_time_types.hpp"

#include "FrameValidator.h"
#include "Logger.h"

using namespace SMPTE_SYNC;
using namespace std;

typedef struct syncInput {
    
    uint32_t  numFrames;
    uint32_t  state;
    uint32_t  frameRate;
    uint32_t  sampleRate;
    uint32_t  timeIndex;
    uint32_t  pictureIndex;
    uint32_t  soundIndex;
    uint32_t  playoutID;
    int32_t  outputOffset;
    uint32_t  screenOffset;
    std::string   pictureUUID;
    std::string   soundUUID;
    std::string   playlistUUID;
    
} syncInput;

bool nextFrame(syncPacket *ss, uint8_t *buffer, uint32_t *count);

bool initInputs(syncInput *si) {
    
    si->numFrames = 1;
    si->state = 0;
    si->frameRate = 24;
    si->sampleRate = 48000;
    si->timeIndex = 0;
    si->pictureIndex = 0;
    si->soundIndex = 0;
    si->playoutID = 0x12345678;
    si->outputOffset = 0;
    si->screenOffset = 0;
    si->pictureUUID = "00000000000000000000000000000000";
    si->soundUUID = "00000000000000000000000000000000";
    si->playlistUUID = "00000000000000000000000000000000";
    return true;
}

bool nextFrame(syncPacket *ss, uint8_t *buffer, uint32_t *count) {
    
    uint8_t lcount;
    *count = 0;
    
    ss->timelineEditUnitIndex_++;
    ss->primaryPictureTrackFileEditUnitIndex_++;
    ss->primarySoundTrackFileEditUnitIndex_++;
    
    if (!WriteUInt32(ss->timelineEditUnitIndex_, buffer + 3*6, &lcount, false)) {
        
        return false;
        
    };
    
    *count += lcount;
    
    if (!WriteUInt32(ss->primaryPictureTrackFileEditUnitIndex_, buffer + 16*6, &lcount, false)) {
        
        return false;
        
    };
    
    *count += lcount;
    
    if (!WriteUInt32(ss->primarySoundTrackFileEditUnitIndex_, buffer + 26*6, &lcount, false)) {
        
        return false;
        
    };
    
    *count += lcount;
    
    return true;
    
}

class CustomFrameValidator : public FrameValidator
{
public:
    CustomFrameValidator(int32_t iSampleRate
                         , int32_t iCallbackBufferSize) :
    FrameValidator(iSampleRate, iCallbackBufferSize)
    , sampleRate_(iSampleRate)
    , callbackBufferSize_(iCallbackBufferSize)
    {
        
    }
    
    virtual ~CustomFrameValidator()
    {
        
    }
    
    virtual void EncounteredSilence(int32_t iNumberSilentSamples)
    {
        SMPTE_SYNC_LOG << "EncounteredSilence";
        
        const int32_t threeSeconds = sampleRate_ * 3;
        if (iNumberSilentSamples > threeSeconds)
        {
            SMPTE_SYNC_LOG << "Silence greater than 3 seconds!";
        }
    }
    
    virtual void HandleFrame(void)
    {
        std::string stateStr = "unknown";
        
        if (this->GetSyncPacket().flags_ == STOPPED)
            stateStr = "stopped";
        if (this->GetSyncPacket().flags_ == PAUSED)
            stateStr = "paused";
        if (this->GetSyncPacket().flags_ == PLAYING)
            stateStr = "playing";
        
        if (this->GetSyncPacket().extensionLength_ != 0)
        {
            SMPTE_SYNC_LOG << "extensionLength_ = " << this->GetSyncPacket().extensionLength_;
        }
        
        SMPTE_SYNC_LOG << "timelineEditUnitIndex_ = " << this->GetSyncPacket().timelineEditUnitIndex_ << " state = " << stateStr;
        
        ASSERT_EQ(currentlyReadFrame_, this->GetSyncPacket().timelineEditUnitIndex_);
        ASSERT_EQ(currentlyReadFrame_, this->GetSyncPacket().primaryPictureTrackFileEditUnitIndex_);
        ASSERT_EQ(currentlyReadFrame_, this->GetSyncPacket().primarySoundTrackFileEditUnitIndex_);
        currentlyReadFrame_++;
    }
    
    void ExecuteTest(void)
    {
        int32_t sampleRate = sampleRate_;
        int32_t validatorBufferSize = callbackBufferSize_;
        int32_t validatorBufferSizeInBytes = validatorBufferSize * 3;
        
        syncInput si;
        
        initInputs(&si);
        
        si.sampleRate = sampleRate;
        si.numFrames = 1000;
        
        uint32_t frameDuration = si.sampleRate / si.frameRate;
        
        uint32_t frameSize = 3*frameDuration;
        
        uint8_t *frame = nullptr;
        
        frame = new uint8_t[frameSize];
        
        ASSERT_NE(frame, nullptr);
        
        memset(frame, 0, frameSize);
        
        uint32_t count = 0;
        
        // Setup the frame we are going to start generating on
        //
        startFrame_ = 0;

        // The first currently read frame should be the start frame
        //
        currentlyReadFrame_ = startFrame_;
        
        syncPacket  sSample;
        
        sSample.SetStatus((uint8_t) si.state);
        sSample.SetTimelineEditUnitIndex(si.timeIndex);
        sSample.SetPlayoutID(si.playoutID);
        sSample.SetEditUnitDuration((uint16_t) frameDuration);
        sSample.SetSampleDuration(1, si.sampleRate);
        sSample.SetPrimaryPictureOutputOffset(si.outputOffset);
        sSample.SetPrimaryPictureScreenOffset(si.screenOffset);
        sSample.SetPrimaryPictureTrackFileEditUnitIndex(si.pictureIndex);

        sSample.timelineEditUnitIndex_ = startFrame_;
        sSample.primaryPictureTrackFileEditUnitIndex_ = startFrame_;
        sSample.primarySoundTrackFileEditUnitIndex_ = startFrame_;

        UUID uuid;
        
        ASSERT_EQ(StringToUUID(si.pictureUUID, uuid), true);
        ASSERT_EQ(sSample.SetPrimaryPictureTrackFileUUID(uuid), true);
        sSample.SetPrimarySoundTrackFileEditUnitIndex(si.soundIndex);
        ASSERT_EQ(StringToUUID(si.soundUUID, uuid), true);
        ASSERT_EQ(sSample.SetPrimarySoundTrackFileUUID(uuid), true);
        ASSERT_EQ(StringToUUID(si.playlistUUID, uuid), true);
        ASSERT_EQ(sSample.SetCompositionPlaylistUUID(uuid), true);
        ASSERT_EQ(sSample.WriteSyncPacket(frame, &count), true);
        
        // Allocate some memory to hold the buffers of audio data
        // in the size that the FrameValidator expects
        //
        uint8_t *validatorBuffer = nullptr;
        validatorBuffer = new uint8_t[validatorBufferSizeInBytes];
        memset(validatorBuffer, 0, validatorBufferSizeInBytes);
        
        int32_t destinationOffset = 0;
        int32_t sourceOffset = 0;
        int32_t copySize = validatorBufferSizeInBytes;
        
        for (int i = 0; i < si.numFrames; i++)
        {
            while (true)
            {
                // How many bytes do we need to copy from the frame buffer into the validatorBuffer
                //
                
                // Check the bytes available in the current frame first
                //
                if ((frameSize - sourceOffset) >= validatorBufferSizeInBytes)
                    copySize = validatorBufferSizeInBytes;
                else
                    copySize = frameSize - sourceOffset;
                
                // Then check the bytes available in the validatorBuffer
                //
                if ((validatorBufferSizeInBytes - destinationOffset) < copySize)
                    copySize = validatorBufferSizeInBytes - destinationOffset;
                
                //SMPTE_SYNC_LOG << "copySize = " << copySize << " destinationOffset = " << destinationOffset << " sourceOffset = " << sourceOffset;
                
                memcpy(validatorBuffer + destinationOffset, frame + sourceOffset, copySize);
                
                destinationOffset += copySize;
                sourceOffset += copySize;
                
                ASSERT_LE(destinationOffset, validatorBufferSizeInBytes);
                ASSERT_LE(sourceOffset, frameSize);
                
                if (destinationOffset == validatorBufferSizeInBytes)
                {
                    destinationOffset = 0;
                    this->AddSamples(validatorBuffer);
                    memset(validatorBuffer, 0, validatorBufferSize);
                }
                
                if (sourceOffset == frameSize)
                {
                    sourceOffset = 0;
                    break;
                }
            }

            ASSERT_EQ(nextFrame(&sSample, frame, &count), true);
        }
        
        delete [] frame;
        delete [] validatorBuffer;
    }
    
private:
    uint32_t    startFrame_;
    uint32_t    currentlyReadFrame_;
    
    int32_t     sampleRate_;
    int32_t     callbackBufferSize_;
};

TEST(SyncSignal_Test, SyncSignal_Test_Case1)
{
    Init_Logger();
    
    int32_t sampleRate = 48000;
    int32_t validatorBufferSize = 512;

    boost::mt19937 randGen(static_cast<std::uint32_t>(std::time(0)));
    boost::uniform_int<> uInt8Dist(16, 16384);
    boost::variate_generator<boost::mt19937&, boost::uniform_int<>> GetRand(randGen, uInt8Dist);

    for (int32_t testCase = 0; testCase < 10; testCase++)
    {
        validatorBufferSize = GetRand();
        CustomFrameValidator validator(sampleRate, validatorBufferSize);
        validator.ExecuteTest();
    }

    sampleRate = 96000;
    
    for (int32_t testCase = 0; testCase < 10; testCase++)
    {
        validatorBufferSize = GetRand();
        CustomFrameValidator validator(sampleRate, validatorBufferSize);
        validator.ExecuteTest();
    }
}

TEST(SyncSignal_Test, SyncSignal_Test_Case2)
{
    const wchar_t*  wideStr1 = L"abcdefghijklmnopqrstuvwxyz0123456789";
    const wchar_t*  wideStr2 = L"bbcdefghijklmnopqrstuvwxyz0123456788";
    const char*     narrowStr1 = "abcdefghijklmnopqrstuvwxyz0123456789";
    const char*     narrowStr2 = "bbcdefghijklmnopqrstuvwxyz0123456788";
    
    boost::posix_time::ptime startTime;
    boost::posix_time::ptime endTime;
    
    startTime = boost::posix_time::microsec_clock::local_time();
    
    for (int i = 0; i < 100000000; i++)
        if (wcscmp(wideStr1, wideStr2) != 0)
        {
            int i = 0;
            i++;
        }
    
    endTime = boost::posix_time::microsec_clock::local_time();
    
    std::cout << "wcscmp = " << (endTime - startTime).total_microseconds() << "\n";
    
    startTime = boost::posix_time::microsec_clock::local_time();
    
    for (int i = 0; i < 100000000; i++)
        if (strcmp(narrowStr1, narrowStr2) != 0)
        {
            int i = 0;
            i++;
        }
    
    endTime = boost::posix_time::microsec_clock::local_time();
    
    std::cout << "strcmp = " << (endTime - startTime).total_microseconds() << "\n";
    
    startTime = boost::posix_time::microsec_clock::local_time();
    
    int64_t t1 = 1;
    int64_t t2 = 2;
    for (int i = 0; i < 100000000; i++)
        if (t1 != t2)
        {
            int i = 0;
            i++;
        }
    
    endTime = boost::posix_time::microsec_clock::local_time();
    
    std::cout << "int64_t = " << (endTime - startTime).total_microseconds() << "\n";
}