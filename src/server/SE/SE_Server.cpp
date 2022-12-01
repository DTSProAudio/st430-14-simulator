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

#include "SE_Server.h"
#include <iostream>
#include <fstream>
#include <string>

#include "Logger.h"
#include "Utils.h"

namespace SMPTE_SYNC
{

    SE_Server::SE_Server(int32_t iSampleRate
                         , int32_t iCallbackBufferSize
                         , int32_t iMaxFrameDurationInSamples
                         , int32_t iProcessingWaitTime
                         ) :
          sampleRate_(iSampleRate)
        , callbackBufferSize_(iCallbackBufferSize)
        , playoutID_(0)
        , primaryPictureOutputOffset_(0)
        , primaryPictureScreenOffset_(0)
        , maxFrameDuration_(iMaxFrameDurationInSamples)
        , processingWaitTime_(iProcessingWaitTime)
        , currentFrameBuffer_(nullptr)
        , currentAudioBuffer_(nullptr)
        , currentFrameDuration_(maxFrameDuration_)
        , currentFrameSize_(3*currentFrameDuration_)
        , playStarTimeInSeconds_(0)
        , isProcessorReady_(false)
        , showLengthInFrames_(0)
        , currentFrame_(0)
        , offsetIntoFrame_(0)
        , offsetIntoCurrentAudioBuffer_(0)
    {
        converter_ = new UTILS::ConverterInt24Float32(sampleRate_);

        currentFrameBuffer_ = new uint8_t[currentFrameSize_];
        memset(currentFrameBuffer_, 0x0, currentFrameSize_);

        // The sample queues store pre-allocated buffers of float arrays
        // that are the size of an audio buffer for the audioDeviceIOCallback
        // The audio buffers are typically 32, 64, 128, 256, 512, etc in size
        //
        // If the audio callback buffer size is changed, the buffers need to be
        // properly reset/rebuilt (drain, delete, allocate, fill)
        //
        // We need to allocate enough buffers to hold enough "frames".
        //
        // What is enough frames? 1/4 of a second will hopefully give enough
        // time to process/create new frames.
        //
        // We need to compute how many frames are in 1/4 of a second.
        // This is based on the iMaxFrameDurationInSamples.
        // Additionally, we need to make sure we have enough samples to hold
        // at least 1/4 of a second worth of frames. This typically means that
        // we will need more buffers of audio than exactly 1/4 of a second
        //
        int32_t samplesInQuarterOfSecond = ((float)iSampleRate / 4) + 0.5;
        int32_t queueDepth = ((float)samplesInQuarterOfSecond / iCallbackBufferSize) + 0.5;
        
        unusedSampleQueue_ = new SampleQueue(queueDepth);
        sampleQueue_ = new SampleQueue(queueDepth);

        // We want our thread to wake up 2x faster than the queue depth to ensure it
        // is filled in a timely fashion
        //
        threadSleepTimeInMS_ = (callbackBufferSize_ / (float)sampleRate_) * 1000 * queueDepth;
        threadSleepTimeInMS_ = threadSleepTimeInMS_ / 2;

        bool isLockFree = unusedSampleQueue_->is_lock_free();
        SMPTE_SYNC_LOG << "SE_Server::SE_Server unusedSampleQueue_ isLockFree = " << (isLockFree ? "true" : "false");
        
        isLockFree = sampleQueue_->is_lock_free();
        SMPTE_SYNC_LOG << "SE_Server::SE_Server sampleQueue_ isLockFree = " << (isLockFree ? "true" : "false");
        
        float *frameBuffer = nullptr;
        for (int32_t i = 0; i < queueDepth; i++)
        {
            frameBuffer = new float[callbackBufferSize_];
            memset(frameBuffer, 0x0, callbackBufferSize_ * sizeof(float));
            
            bool success = unusedSampleQueue_->push(frameBuffer);
            assert(success);
        }
        
#ifdef COPY_BUFFERS_SERVER
        copyBufferNumMinutes_ = 1;
        copyBufferSamplesAvailable_ = sampleRate_ * 60 * copyBufferNumMinutes_;
        copyBufferSamplesAvailable_ = sampleRate_ * 10 * 1;
        //copyBufferSamplesAvailable_ = 10000;
        
        copyBuffer_ = new float[copyBufferSamplesAvailable_];
        copyBufferCopyPostion_ = copyBuffer_;
        memset(copyBuffer_, 0x0, copyBufferSamplesAvailable_ * sizeof(float));
#endif

        keepBuildingFrames_ = true;
        frameBuilderThread_ = boost::thread(&SE_Server::BuildFrames, this);
    }
    
    SE_Server::~SE_Server(void)
    {
        // Before deleting the SE_Server, it's audioDeviceIOCallback
        // should be removed from the callback system.
        //
        
        keepBuildingFrames_ = false;
        
        frameBuilderThread_.join();
        
        delete [] currentFrameBuffer_;
        delete [] currentAudioBuffer_;
        
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

        delete converter_;
        converter_ = nullptr;

#ifdef COPY_BUFFERS_SERVER
        delete [] copyBuffer_;
        copyBuffer_ = nullptr;
#endif
    }

    void SE_Server::Reset(void)
    {
        SMPTE_SYNC_LOG << "SE_Server::Reset";
        this->SetState(eState_NoData);

        boost::mutex::scoped_lock scoped_lock(needsResetMutex_);

        currentFrame_ = 0;
        playStarTimeInSeconds_ = 0;
        offsetIntoFrame_ = 0;
        syncSamp_.Reset();
        memset(currentFrameBuffer_, 0x0, currentFrameSize_);
    }

    bool SE_Server::Initialize(int32_t iSampleRate
                               , int32_t iMaxFrameDurationInSamples
                               , int32_t iShowLengthInFrames)
    {
        if (this->GetState() != eState_NoData)
            return false;

        sampleRate_ = iSampleRate;
        
        if (maxFrameDuration_ != iMaxFrameDurationInSamples)
        {
            maxFrameDuration_ = iMaxFrameDurationInSamples;

            delete [] currentFrameBuffer_;

            currentFrameBuffer_ = new uint8_t[maxFrameDuration_];
            memset(currentFrameBuffer_, 0x0, maxFrameDuration_);
        }
        
        showLengthInFrames_ = iShowLengthInFrames;
        this->Reset();
        
        this->Stop();
        
        return true;
    }
    
    void SE_Server::Play()
    {
        bool ready = true;
        {
            boost::mutex::scoped_lock scoped_lock(isProcessorReadyMutex_);
            ready = isProcessorReady_;
        }

        if (ready)
        {
            this->SetState(eState_Playing);
        }
        else
        {
            {
                boost::mutex::scoped_lock scoped_lock(isProcessorReadyMutex_);
                playStarTimeInSeconds_ = time(NULL);
            }
            
            this->SetState(eState_WaitingToPlay);
        }
    }
    
    void SE_Server::Pause()
    {
        this->SetState(eState_Paused);
    }
    
    void SE_Server::Stop()
    {
        this->SetState(eState_Stopped);
    }

    void SE_Server::SetFrame(int32_t iFrameNumber)
    {
        this->Pause();
        currentFrame_ = iFrameNumber;
    }
    
    // Get the frame that is currently being played out
    //
    int32_t SE_Server::GetCurrentFrame(void)
    {
        return currentFrame_;
    }
    
    void SE_Server::ReturnToStart()
    {
        this->SetFrame(0);
    }

    void SE_Server::audioDeviceIOCallback(const float** /*inputChannelData*/,
                                          int /*numInputChannels*/,
                                          float** outputChannelData,
                                          int numOutputChannels,
                                          int numSamples)
    {
#ifdef COPY_BUFFERS_SERVER
        static boost::posix_time::time_duration cummulativeCallbackTime;
        static boost::posix_time::time_duration maxCallbackTime;
        static int32_t numberOfCallbacks;
        bool printCallbackTimeStats = false;
        
        numberOfCallbacks++;
        boost::posix_time::ptime callbackStartTime = boost::posix_time::microsec_clock::local_time();
#endif

        for (int i = 0; i < numOutputChannels; ++i)
            memset(outputChannelData[i], 0, sizeof(float) * (size_t)numSamples);
        
        float *sampleBuffer = nullptr;
        if (sampleQueue_->pop(sampleBuffer))
        {
            memcpy(outputChannelData[0], sampleBuffer, numSamples * sizeof(float));

            bool success = unusedSampleQueue_->push(sampleBuffer);
            assert(success);

#ifdef COPY_BUFFERS_SERVER
            if (copyBuffer_ && copyBufferSamplesAvailable_ >= numSamples)
            {
                memcpy(copyBufferCopyPostion_, outputChannelData[0], numSamples * sizeof(float));
                copyBufferCopyPostion_ += numSamples;
                copyBufferSamplesAvailable_ -= numSamples;
            }
            else
            if (copyBufferSamplesAvailable_ < numSamples)
            {
                if (copyBufferSamplesAvailable_ > 0)
                {
                    memcpy(copyBufferCopyPostion_, outputChannelData[0], copyBufferSamplesAvailable_ * sizeof(float));
                }
                
                //copyBufferSamplesAvailable_ = sampleRate_ * 60 * copyBufferNumMinutes_;
                copyBufferSamplesAvailable_ = sampleRate_ * 10 * 1;
                //copyBufferSamplesAvailable_ = 10000;
        
                static int32_t fileCount = 0;
                std::string full_path = "serverCopyBuffer" + std::to_string(fileCount) + ".dat";
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
                
                // Copy any remaining samples
                //
                memcpy(copyBufferCopyPostion_, outputChannelData[0], numSamples * sizeof(float));
                copyBufferCopyPostion_ += numSamples;
                copyBufferSamplesAvailable_ -= numSamples;

                // Print stats
                boost::posix_time::time_duration averageTime = cummulativeCallbackTime / numberOfCallbacks;
                SMPTE_SYNC_LOG << "SE_Server maxCallbackTime = " << maxCallbackTime.total_microseconds();
                SMPTE_SYNC_LOG << "SE_Server averageTime = " << averageTime.total_microseconds();
                cummulativeCallbackTime = boost::posix_time::time_duration();
                maxCallbackTime = boost::posix_time::time_duration();
                printCallbackTimeStats = true;
            }
            
            if (printCallbackTimeStats)
            {
                // don't want to average time when we dump to a file
            }
            else
            {
                boost::posix_time::ptime callbackEndTime = boost::posix_time::microsec_clock::local_time();
                boost::posix_time::time_duration callbackTime = (callbackEndTime - callbackStartTime);
                cummulativeCallbackTime += callbackTime;
                if (callbackTime > maxCallbackTime)
                    maxCallbackTime = callbackTime;
            }
#endif
        }
        else
        {
            //SMPTE_SYNC_LOG << "SE_Server::audioDeviceIOCallback failed to pop from sampleQueue_\n";
        }
    }
    
    // Algorithm
    //
    // The BuildFrames function is runs on a separate thread.
    // This thread runs approximately once every video frame.
    // It computes 1 video frame of data and subdivides it into audio samples
    //
    // There are 3 cases for audio samples that need to be taken into consideration...
    // 1) We may not have en
    //
    void SE_Server::BuildFrames(void)
    {
        while (keepBuildingFrames_)
        {
            // Since I'm using 'continue' in this loop multiple times
            // I'm going to sleep my thread at the beginning of the loop
            // rather than at the end.
            //
            boost::this_thread::sleep(boost::posix_time::milliseconds(threadSleepTimeInMS_));
            EState state = this->GetState();
            
            if (state == eState_NoData)
            {
                // don't generate any frames to be played back
                //
                continue;
            }
            
            if (currentAudioBuffer_ != nullptr)
            {
                SMPTE_SYNC_LOG << "SE_Server::BuildsFrames - fatal error currentAudioBuffer_ should always be null at this point. Exiting thread.";
                assert(!"Error: currentAudioBuffer_ should always be null at this point.");
                
                keepBuildingFrames_ = false;
                delete currentAudioBuffer_;
                currentAudioBuffer_ = nullptr;
                offsetIntoCurrentAudioBuffer_ = 0;
                
                break;
            }
            
            bool needMoreSamplesToFillBuffer = false;
            uint8_t bytesPerSample = (3 * sizeof(uint8_t));
            
            // If we have some leftover samples
            // we must copy them into an audio buffer before
            // creating a new frame
            //
            bool needsToWaitForMoreAvailableBuffers = false;
            while((currentFrameDuration_ - offsetIntoFrame_) > 0)
            {
                if (unusedSampleQueue_->pop(currentAudioBuffer_))
                {
                    offsetIntoCurrentAudioBuffer_ = 0;
                    int32_t numberOfSamplesToCopy = std::min(callbackBufferSize_, currentFrameDuration_ - offsetIntoFrame_);

                    converter_->ConvertInt24ToFloat(currentFrameBuffer_ + (offsetIntoFrame_ * bytesPerSample),
                                                    currentAudioBuffer_,
                                                    numberOfSamplesToCopy);
                    
                    offsetIntoFrame_ += numberOfSamplesToCopy;
                    offsetIntoCurrentAudioBuffer_ += numberOfSamplesToCopy;
                    
                    if (offsetIntoCurrentAudioBuffer_ == callbackBufferSize_)
                    {
                        bool success = sampleQueue_->push(currentAudioBuffer_);
                        assert(success);
                        currentAudioBuffer_ = nullptr;
                        offsetIntoCurrentAudioBuffer_ = 0;
                    }
                    else
                    {
                        //SMPTE_SYNC_LOG << "SE_Server::BuildFrames Partially filled an audio buffer. Need another frame to finish filling the buffer. 1\n";
                        needMoreSamplesToFillBuffer = true;
                    }
                }
                else
                {
                    SMPTE_SYNC_LOG << "SE_Server::BuildFrames failed to pop unusedSampleQueue_ 1\n";
                    needsToWaitForMoreAvailableBuffers = true;

                    // Ensure that since the pop failed, we reset the buffer to null
                    // in case the Boost code set it to something other than null
                    // in the process of trying to pop and failing
                    //
                    currentAudioBuffer_ = nullptr;

                    break;
                }
            }

            if (needsToWaitForMoreAvailableBuffers)
            {
                // Jump back to the top of the BuildFrames function and sleep
                //
                continue;
            }
        
            syncSamp_.timelineEditUnitIndex_ = currentFrame_;
            
            if (state == eState_Paused)
            {
                syncSamp_.flags_ = 0x1;
            }
            else
            if (state == eState_Stopped)
            {
                syncSamp_.flags_ = 0x0;
            }
            else
            if (state == eState_WaitingToPlay)
            {
                // If we are waiting to play,
                // consider the state to be paused
                //
                syncSamp_.flags_ = 0x1;
                
                bool ready = true;
                {
                    boost::mutex::scoped_lock scoped_lock(isProcessorReadyMutex_);
                    ready = isProcessorReady_;
                }
                
                if (ready)
                {
                    this->SetState(eState_Playing);
                }
                else
                {
                    // Test waiting for playback
                    //
#if 0
                    time_t currentTime(NULL);
                    time_t startTime(NULL);
                    {
                        boost::mutex::scoped_lock scoped_lock(isProcessorReadyMutex_);
                        startTime = playStarTimeInSeconds_;
                    }
                    
                    time_t timeOut = startTime + processingWaitTime_;
                    if (currentTime > timeOut)
#endif
                    {
                        this->SetState(eState_Playing);

                        // SetState could fail,
                        // use GetState to determine the new state
                        //
                        state = this->GetState();
                    }
                }
            }
            
            // Start filling our sample buffers
            //
            bool createFrames = true;
            while (createFrames)
            {
                if (state == eState_Playing)
                {
                    syncSamp_.timelineEditUnitIndex_ = currentFrame_;
                    if (syncSamp_.timelineEditUnitIndex_ >= showLengthInFrames_)
                    {
                        this->Reset();
                        
                        // Break out of our fill buffer loop
                        // We will sleep and start again
                        //
                        break;
                    }
                    else
                    {
                        syncSamp_.flags_ = 0x2;
                    }
                    
                    // Now that we have computed our last frame
                    // increment the current frame
                    //
                    currentFrame_++;
                }
                
                if (frameDataCallback_)
                {
                    if (!frameDataCallback_(syncSamp_.timelineEditUnitIndex_, frameData_))
                    {
                        this->SetState(eState_NoData);
                        this->Reset();
                        
                        // Break out of our fill buffer loop
                        // We will sleep and start again
                        //
                        break;
                    }
                }
                
                if (this->SetupPacket())
                {
                    if (needMoreSamplesToFillBuffer)
                    {
                        int32_t numberOfSamplesToCopy = std::min(callbackBufferSize_ - offsetIntoCurrentAudioBuffer_, currentFrameDuration_ - offsetIntoFrame_);
                        
                        converter_->ConvertInt24ToFloat(currentFrameBuffer_ + (offsetIntoFrame_ * bytesPerSample),
                                                        currentAudioBuffer_ + offsetIntoCurrentAudioBuffer_,
                                                        numberOfSamplesToCopy);
                        
                        offsetIntoFrame_ += numberOfSamplesToCopy;
                        offsetIntoCurrentAudioBuffer_ += numberOfSamplesToCopy;
                        
                        // If we need more data, get a new frame
                        //
                        if (offsetIntoFrame_ > currentFrameDuration_)
                        {
                            assert(false);
                            SMPTE_SYNC_LOG << "SE_Server::BuildFrames Something wrong happened\n";
                        }
                        
                        if (offsetIntoCurrentAudioBuffer_ == callbackBufferSize_)
                        {
                            bool success = sampleQueue_->push(currentAudioBuffer_);
                            assert(success);
                            currentAudioBuffer_ = nullptr;
                            offsetIntoCurrentAudioBuffer_ = 0;
                            needMoreSamplesToFillBuffer = false;
                        }
                    }
                    
                    // Now that we have created the frame,
                    // we must subdivide it into audio buffers
                    //
                    while (offsetIntoFrame_ < currentFrameDuration_)
                    {
                        if (unusedSampleQueue_->pop(currentAudioBuffer_))
                        {
                            offsetIntoCurrentAudioBuffer_ = 0;
                            int32_t numberOfSamplesToCopy = std::min(callbackBufferSize_, currentFrameDuration_ - offsetIntoFrame_);
                            
                            converter_->ConvertInt24ToFloat(currentFrameBuffer_ + (offsetIntoFrame_ * bytesPerSample),
                                                            currentAudioBuffer_,
                                                            numberOfSamplesToCopy);

                            offsetIntoFrame_ += numberOfSamplesToCopy;
                            offsetIntoCurrentAudioBuffer_ += numberOfSamplesToCopy;
                            
                            if (offsetIntoCurrentAudioBuffer_ == callbackBufferSize_)
                            {
                                bool success = sampleQueue_->push(currentAudioBuffer_);
                                assert(success);
                                currentAudioBuffer_ = nullptr;
                                offsetIntoCurrentAudioBuffer_ = 0;
                            }
                            else
                            {
                                //SMPTE_SYNC_LOG << "SE_Server::BuildFrames Partially filled an audio buffer. Need another frame to finish filling the buffer.\n";
                                needMoreSamplesToFillBuffer = true;
                            }
                        }
                        else
                        {
                            //SMPTE_SYNC_LOG << "SE_Server::BuildFrames Failed to pop unusedSampleQueue_ 2\n";
                            createFrames = false;
                            
                            // Ensure that since the pop failed, we reset the buffer to null
                            // in case the Boost code set it to something other than null
                            // in the process of trying to pop and failing
                            //
                            currentAudioBuffer_ = nullptr;
                            break;
                        }
                    }
                }
                else
                {
                    SMPTE_SYNC_LOG << "SE_Server::BuildFrames SetupPacket failed.\n";
                    break;
                }
            }
        }
    }

    bool SE_Server::SetupPacket(void)
    {
        bool success = true;
        
        if (currentFrameDuration_ != frameData_.currentFrameDuration_)
        {
            int i = 0;
            i++;
        }
        
        currentFrameDuration_ = frameData_.currentFrameDuration_;
        currentFrameSize_ = 3 * currentFrameDuration_;

        memset(currentFrameBuffer_, 0x0, currentFrameSize_);

        // syncSamp_.flags set in BuildFrames
        //
        syncSamp_.SetStatus((uint8_t) syncSamp_.flags_);
        
        // syncSamp_.timelineEditUnitIndex set in BuildFrames
        //
        syncSamp_.SetTimelineEditUnitIndex(syncSamp_.timelineEditUnitIndex_);

        syncSamp_.SetPlayoutID(playoutID_);
        
        syncSamp_.SetEditUnitDuration((uint16_t) currentFrameDuration_);
        
        syncSamp_.SetSampleDuration(1, sampleRate_);
        
        syncSamp_.SetPrimaryPictureOutputOffset(primaryPictureOutputOffset_);
        
        syncSamp_.SetPrimaryPictureScreenOffset(primaryPictureScreenOffset_);
        
        syncSamp_.SetPrimaryPictureTrackFileEditUnitIndex(frameData_.primaryPictureTrackFileEditUnitIndex_);
        
        if (!syncSamp_.SetPrimaryPictureTrackFileUUID(frameData_.primaryPictureTrackFileUUID_))
            success = false;
        
        syncSamp_.SetPrimarySoundTrackFileEditUnitIndex(frameData_.primarySoundTrackFileEditUnitIndex_);
        
        if (!syncSamp_.SetPrimarySoundTrackFileUUID(frameData_.primarySoundTrackFileUUID_))
            success = false;
        
        if (!syncSamp_.SetCompositionPlaylistUUID(frameData_.compositionPlaylistUUID_))
            success = false;
        
        uint32_t count;

        offsetIntoFrame_ = 0;
        if (!syncSamp_.WriteSyncPacket(currentFrameBuffer_, &count))
            success = false;

        return success;
    }
    
    void SE_Server::SetProcessorIsReady(bool iReady)
    {
        boost::mutex::scoped_lock scoped_lock(isProcessorReadyMutex_);
        isProcessorReady_ = iReady;
    }

    void SE_Server::SetGetFrameDataCallback(GetFrameDataCallback iCallback)
    {
        frameDataCallback_ = iCallback;
    }

    void SE_Server::SetPlayoutID(uint32_t iID)
    {
        playoutID_ = iID;
    }
    
    bool SE_Server::SetPrimaryPictureOutputOffset(int32_t iOffset)
    {
        if (this->GetState() != eState_NoData)
            return false;

        primaryPictureOutputOffset_ = iOffset;
        
        return true;
    }
    
    bool SE_Server::SetPrimaryPictureScreenOffset(uint32_t iOffset)
    {
        if (this->GetState() != eState_NoData)
            return false;

        primaryPictureScreenOffset_ = iOffset;
        
        return true;
    }
    
}  // namespace SMPTE_SYNC
