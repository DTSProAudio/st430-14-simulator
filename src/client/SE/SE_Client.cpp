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

#include "SE_Client.h"

#include <iostream>
#include <fstream>
#include <string>

#include "Logger.h"
#include "AuxData.h"
#include "sync.h"
#include "Utils.h"
#include "FrameValidator.h"

namespace SMPTE_SYNC
{
    SE_Client::SE_Client(int32_t iSampleRate
                         , int32_t iCallbackBufferSize) :
          currentFrameDuration_(2000)
        , sampleRate_(iSampleRate)
        , callbackBufferSize_(iCallbackBufferSize)
        , currentFrame_(0)
        , FrameValidator(iSampleRate, iCallbackBufferSize)
    {
        callbackBufferTimeInMS_ = (callbackBufferSize_ / (float)sampleRate_) * 1000;
        
        converter_ = new UTILS::ConverterInt24Float32(sampleRate_);
        
        currentAudioBufferInt24_ = new uint8_t[3*callbackBufferSize_];
        memset((char*)currentAudioBufferInt24_, 0, 3*callbackBufferSize_);

        // 5 frames is probably too aggressive as that allows for 5 frames of audio
        //
        unusedSampleQueue_ = new SampleQueue(currentFrameDuration_*5);
        sampleQueue_ = new SampleQueue(currentFrameDuration_*5);

        bool isLockFree = unusedSampleQueue_->is_lock_free();
        SMPTE_SYNC_LOG << "SE_Client::SE_Client unusedSampleQueue_ isLockFree = " << (isLockFree ? "true" : "false");

        isLockFree = sampleQueue_->is_lock_free();
        SMPTE_SYNC_LOG << "SE_Client::SE_Client sampleQueue_ isLockFree = " << (isLockFree ? "true" : "false");

        float *sampleBuffer = nullptr;
        for (int32_t i = 0; i < currentFrameDuration_*5; i++)
        {
            sampleBuffer = new float[callbackBufferSize_];
            memset(sampleBuffer, 0x0, callbackBufferSize_ * sizeof(float));
            if (!unusedSampleQueue_->push(sampleBuffer))
            {
                SMPTE_SYNC_LOG << "SE_Client::SE_Client failed to add buffer";
            }
        }
        
        keepParsing_ = true;
        frameParsingThread_ = boost::thread(&SE_Client::ParseFrames, this);

#ifdef COPY_BUFFERS_CLIENT
        copyBufferNumMinutes_ = 1;
        copyBufferSamplesAvailable_ = sampleRate_ * 60 * copyBufferNumMinutes_;
        copyBufferSamplesAvailable_ = sampleRate_ * 10 * 1;

        copyBuffer_ = new float[copyBufferSamplesAvailable_];
        copyBufferCopyPostion_ = copyBuffer_;
        memset(copyBuffer_, 0x0, copyBufferSamplesAvailable_ * sizeof(float));
#endif
    }

    SE_Client::~SE_Client()
    {
        // Before deleting the SE_Client, it's audioDeviceIOCallback
        // should be removed from the callback system.
        //

        keepParsing_ = false;

        frameParsingThread_.join();

        float *frame = nullptr;
        while (unusedSampleQueue_->pop(frame))
        {
            delete [] frame;
            frame = nullptr;
        }

        delete unusedSampleQueue_;
        unusedSampleQueue_ = nullptr;
        
        frame = nullptr;
        while (sampleQueue_->pop(frame))
        {
            delete [] frame;
            frame = nullptr;
        }

        delete sampleQueue_;
        sampleQueue_ = nullptr;
        
        delete [] currentAudioBufferInt24_;
        currentAudioBufferInt24_ = nullptr;

        delete converter_;
        converter_ = nullptr;

#ifdef COPY_BUFFERS_CLIENT
        delete [] copyBuffer_;
        copyBuffer_ = nullptr;
#endif
    }

    void SE_Client::SetValidatorCallback(SyncPacketValidatorCallback iCallback)
    {
        validator_ = iCallback;
    }

    void SE_Client::audioDeviceIOCallback(  const float** inputChannelData,
                                            int numInputChannels,
                                            float** /*outputChannelData*/,
                                            int /*numOutputChannels*/,
                                            int numSamples)
    {
        if (numInputChannels != 1)
        {
            return;
        }
        
        if (callbackBufferSize_ < numSamples)
        {
            SMPTE_SYNC_LOG << "SE_Client::audioDeviceIOCallback callbackBufferSize_ = " << callbackBufferSize_ << " numSamples = " << numSamples;
            return;
        }

        float *sampleBuffer = nullptr;
        if (unusedSampleQueue_->pop(sampleBuffer))
        {
            // Only supporting a single channel of audio
            // Thus the 0 for inputChannelData
            //
            memcpy(sampleBuffer, inputChannelData[0], numSamples * sizeof(float));

#ifdef COPY_BUFFERS_CLIENT
            if (copyBuffer_ && copyBufferSamplesAvailable_ >= callbackBufferSize_)
            {
                memcpy(copyBufferCopyPostion_, inputChannelData[0], numSamples * sizeof(float));
                copyBufferCopyPostion_ += numSamples;
                copyBufferSamplesAvailable_ -= numSamples;
            }
            else
            if (copyBufferSamplesAvailable_ < numSamples)
            {
                copyBufferSamplesAvailable_ = sampleRate_ * 60 * copyBufferNumMinutes_;
                copyBufferSamplesAvailable_ = sampleRate_ * 10 * 1;
                
                static int32_t fileCount = 0;
                std::string full_path = "clientCopyBuffer" + std::to_string(fileCount) + ".dat";
                fileCount++;
                std::ofstream dataFile(full_path, std::ofstream::out |std::ofstream::binary);
                if (dataFile.good())
                {
                    uint8_t *convertedCopyBuffer = new uint8_t[copyBufferSamplesAvailable_ * 3];
                    converter_->ConvertFloatToInt24(copyBuffer_, convertedCopyBuffer, copyBufferSamplesAvailable_);
                    
                    dataFile.write((char*)convertedCopyBuffer, copyBufferSamplesAvailable_ * 3);
                    dataFile.close();
                    
                    delete [] convertedCopyBuffer;
                }
                
                // Reset
                //
                copyBufferCopyPostion_ = copyBuffer_;
                memset(copyBuffer_, 0x0, copyBufferSamplesAvailable_ * sizeof(float));
            }
#endif

            if (!sampleQueue_->push(sampleBuffer))
            {
                SMPTE_SYNC_LOG << "SE_Client::audioDeviceIOCallback failed to push on sampleQueue_\n";
            }
        }
        else
        {
            SMPTE_SYNC_LOG << "SE_Client::audioDeviceIOCallback failed to pop from unusedSampleQueue_\n";
        }
    }

    int32_t SE_Client::GetCurrentFrame(void)
    {
        return currentFrame_;
    }
    
    SE_State::EState FlagsToState(uint16_t iFlags)
    {
        SE_State::EState state = SE_State::eState_NoData;
        
        if (iFlags == 0x0)
            state = SE_State::eState_Stopped;

        if (iFlags == 0x1)
            state = SE_State::eState_Paused;

        if (iFlags == 0x2)
            state = SE_State::eState_Playing;

        return state;
    }
    
    void SE_Client::HandleFrame(void)
    {
        SE_State::EState currentState = FlagsToState(this->GetSyncPacket().flags_);
        if (this->GetState() != currentState)
        {
            this->SetState(currentState);
        }
        
        currentFrame_ = this->GetSyncPacket().timelineEditUnitIndex_;
        
        if (validator_)
            validator_(this->GetSyncPacket());
    }
    
    void SE_Client::ResetFrame(void)
    {
        FrameValidator::ResetFrame();
    }

    void SE_Client::EncounteredSilence(int32_t iNumberSilentSamples)
    {
        //SMPTE_SYNC_LOG << "SE_Client::EncounteredSilence";

        const int32_t threeSeconds = sampleRate_ * 3;
        if (iNumberSilentSamples > threeSeconds)
        {
            this->SetState(eState_NoData);
        }
    }

    void SE_Client::ParseFrames(void)
    {
        while (keepParsing_)
        {
            float *sampleBuffer = nullptr;

            {
                if (!sampleQueue_->pop(sampleBuffer))
                {
                    //SMPTE_SYNC_LOG << "SE_Client::ParseFrames failed to pop from sampleQueue_\n";
                    boost::this_thread::sleep(boost::posix_time::milliseconds(callbackBufferTimeInMS_*2));
                }
                
                if (!keepParsing_)
                {
                    break;
                }
            }

            if (sampleBuffer != nullptr)
            {
                converter_->ConvertFloatToInt24(sampleBuffer, currentAudioBufferInt24_, callbackBufferSize_);
                
                // Recycle the buffer
                //
                memset(sampleBuffer, 0, callbackBufferSize_ * sizeof(float));
                
                {
                    if (!unusedSampleQueue_->push(sampleBuffer))
                    {
                        SMPTE_SYNC_LOG << "SE_Client::ParseFrames failed to push onto sampleQueue_\n";
                        delete [] sampleBuffer;
                    }
                    sampleBuffer = nullptr;
                }

                this->AddSamples(currentAudioBufferInt24_);
            }
        }
    }
    
}  // namespace SMPTE_SYNC
