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

#include "FrameValidator.h"

#include <assert.h>
#include <string.h>

#if 1
#include "Logger.h"
#else
#include <iostream>
#define mda_logger_is_enabled true
#define SMPTE_SYNC_LOG if (!mda_logger_is_enabled) ; else std::cout
#endif

#define WORDS_PER_PACKET 44 

namespace SMPTE_SYNC
{
    FrameValidator::FrameValidator(int32_t iSampleRate
                                   , int32_t iBufferSize) :
          frame_(nullptr)
        , sampleRate_(iSampleRate)
        , bufferSize_(iBufferSize)
        , payloadDuration_(WORDS_PER_PACKET * 2)
        , payloadSizeInBytes_(3 * payloadDuration_)
        , currentFrameDuration_(2000)
        , currentFrameSizeInBytes_(3*currentFrameDuration_)
        , offsetIntoFrameInSamples_(0)
        , samplesUntilSyncMarker_(0)
        , lookingForSyncMarker_(true)
        , lookingForSyncMarker2sComplement_(true)
        , payloadLength_(0)
        , currentFrame_(0)
        , silence_(nullptr)
        , foundSilence_(false)
        , isFirstSilentSample_(false)
        , notifiedSilenceExceededThreeSeconds_(false)
        , numberOfSilentSamples_(0)
    {
        currentAudioBuffer_ = new uint8_t[3*bufferSize_];
        memset((char*)currentAudioBuffer_, 0, 3 * bufferSize_);

        frame_ = new uint8_t[currentFrameSizeInBytes_];
        memset((char*)frame_, 0, currentFrameSizeInBytes_);
        
        syncMarker_ = new uint8_t[3];

        uint32_t sync = 0x01AAF0;
        memcpy(syncMarker_, &sync, 3);
        
        syncMarkerTwosComplement_ = new uint8_t[3];
        
        uint32_t sync2s = (~sync) + 1;
        memcpy(syncMarkerTwosComplement_, &sync2s, 3);
        
        silence_ = new uint8_t[3*bufferSize_];
        memset((char*)silence_, 0, 3 * bufferSize_);
    }

    FrameValidator::~FrameValidator()
    {
        delete [] syncMarkerTwosComplement_;
        syncMarkerTwosComplement_ = nullptr;

        delete [] syncMarker_;
        syncMarker_ = nullptr;

        delete [] frame_;
        frame_ = nullptr;
        
        delete [] currentAudioBuffer_;
        currentAudioBuffer_ = nullptr;

        delete [] silence_;
        silence_ = nullptr;
    }

    void FrameValidator::AddSamples(const uint8_t *iBuffer)
    {
        memcpy(currentAudioBuffer_, iBuffer, 3*bufferSize_);
        this->ParseFrames();
    }

    bool FrameValidator::ValidateSyncMarkers(void)
    {
        // We only run this when we have a full frame of samples
        // The first 6 bytes should be the valid sync marker
        //
        if (this->CheckSyncMarker())
        {
            int currentSample = 2;
            int numSamples = payloadDuration_;
            
            while (currentSample < numSamples)
            {
                if (memcmp(syncMarker_, frame_ + (currentSample*3), 3) == 0)
                {
                    SMPTE_SYNC_LOG << "FrameValidator::ValidateSyncMarkers 1 false"
                    << " currentFrame_ = "
                    << currentFrame_;
                    
                    return false;
                }
                if (memcmp(syncMarkerTwosComplement_, frame_ + (currentSample*3), 3) == 0)
                {
                    SMPTE_SYNC_LOG << "FrameValidator::ValidateSyncMarkers 2 false"
                    << " currentFrame_ = "
                    << currentFrame_;
                    
                    return false;
                }
                currentSample++;
            }
            
            return true;
        }
        else
        {
            SMPTE_SYNC_LOG << "FrameValidator::ValidateSyncMarkers 3 false"
            << " currentFrame_ = "
            << currentFrame_;

            return false;
        }
        
        SMPTE_SYNC_LOG << "FrameValidator::ValidateSyncMarkers 4 false"
        << " currentFrame_ = "
        << currentFrame_;

        return false;
    }

    bool FrameValidator::ValidateLeadAndTailStructures(int32_t iSample)
    {
        // Do we have at least 2 samples of data to validate
        //
        if (iSample + 2 < currentFrameDuration_)
        {
            int32_t lead = 0;
            int32_t tail = 0;

            memcpy(&lead, frame_ + (iSample * 3), 3);
            memcpy(&tail, frame_ + ((iSample * 3) + 3), 3);

            int32_t lead2sComplement = (~lead) + 1;
            lead2sComplement = lead2sComplement & 0x00FFFFFF;
            
            if (lead2sComplement == tail)
                return true;
            else
            {
                SMPTE_SYNC_LOG << "FrameValidator::ValidateLeadAndTailStructures 1 false"
                << " iSample = "
                << iSample
                << " currentFrameDuration_ = "
                << currentFrameDuration_
                << " currentFrame_ = "
                << currentFrame_;

                return false;
            }
        }
        else
        {
            SMPTE_SYNC_LOG << "FrameValidator::ValidateLeadAndTailStructures 2 false"
            << " iSample = "
            << iSample
            << " currentFrameDuration_ = "
            << currentFrameDuration_
            << " currentFrame_ = "
            << currentFrame_;
            
            return false;
        }
        
        SMPTE_SYNC_LOG << "FrameValidator::ValidateLeadAndTailStructures 3 false"
        << " iSample = "
        << iSample
        << " currentFrameDuration_ = "
        << currentFrameDuration_
        << " currentFrame_ = "
        << currentFrame_;
        
        return false;
    }
    
    bool FrameValidator::CheckSyncMarker()
    {
        if (memcmp(syncMarker_, frame_, 3) == 0)
        {
            if (memcmp(syncMarkerTwosComplement_, frame_ + 3, 3) == 0)
            {
                return true;
            }
            else
            {
                SMPTE_SYNC_LOG << "FrameValidator::CheckSyncMarker 1 false"
                << " currentFrame_ = "
                << currentFrame_;

                return false;
            }
        }
        else
        {
            SMPTE_SYNC_LOG << "FrameValidator::CheckSyncMarker 2 false"
            << " currentFrame_ = "
            << currentFrame_;
            
            return false;
        }
        
        SMPTE_SYNC_LOG << "FrameValidator::CheckSyncMarker 3 false"
        << " currentFrame_ = "
        << currentFrame_;

        return false;
    }
    
    bool FrameValidator::ValidatePayload(void)
    {
        if (offsetIntoFrameInSamples_ == payloadDuration_)
        {
            if (this->ValidateSyncMarkers())
            {
                // Check the length field
                //
                if (this->ValidateLeadAndTailStructures(6))
                {
                    payloadLength_ = 0;
                    memcpy(&payloadLength_, frame_ + (2 * 3), 3);
                    
                    // Make sure the total length is less than
                    // the total frame duration minus the marker and length
                    //
                    // Currently not supporting extensions as they are reservered for future updates
                    //
                    if (payloadLength_ > (payloadDuration_ - 2*2*3))
                    {
                        SMPTE_SYNC_LOG << "FrameValidator::ValidatePayload length is greater than allowed size for version 1 of the specification. payloadLength_ = " << payloadLength_;
                        return false;
                    }
                    
                    // Check all of the lead/tail pairs in the payload
                    //
                    int currentSample = 2 + 2;
                    int numSamples = payloadLength_ * 2;
                    
                    while (currentSample < numSamples)
                    {
                        if (!this->ValidateLeadAndTailStructures(currentSample))
                        {
                            SMPTE_SYNC_LOG << "FrameValidator::ValidatePayload lead and tail structures do not match."
                            << " currentFrame_ = "
                            << currentFrame_;
                            return false;
                        }
                        
                        currentSample += 2;
                    }
                    
                    return true;
                }
            }
        }
        
        return false;
    }
    
    bool FrameValidator::ValidateFillSamples(void)
    {
        if (offsetIntoFrameInSamples_ == currentFrameDuration_)
        {
            // To get to the first sample of the payload
            // We need to skip the 2 samples of the sync marker
            // and the 2 samples of the length
            // We then skip the playload length * 2 since there
            // are 2 samples for each word in the payload.
            //
            int currentSample = 4 + (payloadLength_ * 2);
            int numSamples = currentFrameDuration_;
            
            uint8_t zeroSample[3];
            memset(zeroSample, 0, 3);
            
            while (currentSample < numSamples)
            {
                if (memcmp(zeroSample, frame_ + (currentSample*3), 3) != 0)
                {
                    uint8_t testSample[3];
                    memcpy(testSample, frame_ + (currentSample*3), 3);
                    
                    SMPTE_SYNC_LOG << "FrameValidator::ValidateFillSamples found a non-zero sample 0x"
                    << testSample[0] << " "
                    << testSample[1] << " "
                    << testSample[2];

                    return false;
                }
                currentSample++;
            }

            return true;
        }
        
        return false;
    }
    
    bool FrameValidator::CheckOverlappingFramesAndShuffle(void)
    {
        if (this->CheckSyncMarker())
        {
            // Start checking after the sync marker
            //
            int currentSample = 2;
            int numSamples = currentFrameDuration_;
            
            while (currentSample < numSamples)
            {
                if (memcmp(syncMarker_, frame_ + (currentSample*3), 3) == 0)
                {
                    // We found another sync marker!
                    // Shuffle the contents of the frame_ such that the
                    // position of the sync marker  is in the 0 position
                    // of the frame_
                    //
                    memmove(frame_, frame_ + (currentSample * 3), currentFrameSizeInBytes_ - (currentSample * 3));
                    
                    // Is this sync marker at the next to last sample of the frame?
                    // We assume the next sample is the 2s complement of the sync marker
                    // If it isn't, the next loop through the main frame processing loop
                    // will check during ValidateFrame
                    //
                    
                    // If the sync marker is at the very last sample of the frame,
                    // we need to move it to the first sample of the frame
                    // and then accumulate more samples before checking for the
                    // 2s complement sync marker
                    //
                    if (currentSample == (currentFrameDuration_ - 1))
                    {
                        lookingForSyncMarker2sComplement_ = true;
                    }
                    
                    // Fix up the offset and memset any empty samples to 0
                    //
                    offsetIntoFrameInSamples_ = currentFrameDuration_ - currentSample;
                    memset(frame_ + (offsetIntoFrameInSamples_*3), 0, currentFrameSizeInBytes_ - (offsetIntoFrameInSamples_*3));

                    return true;
                }
                currentSample++;
            }
        }
        
        return false;
    }
    
    void FrameValidator::ResetFrame(void)
    {
        //SMPTE_SYNC_LOG << "FrameValidator::ResetFrame";

        syncPacket_.Reset();
        
        // Reset the frame and start looking for the next frame
        //
        memset(frame_, 0, currentFrameSizeInBytes_);
        lookingForSyncMarker_ = true;
        lookingForSyncMarker2sComplement_ = true;
        offsetIntoFrameInSamples_ = 0;
        samplesUntilSyncMarker_ = 0;
        this->ClearSilenceFlags();
    }

    void FrameValidator::ClearSilenceFlags(void)
    {
        //SMPTE_SYNC_LOG << "FrameValidator::ClearSilenceFlags";

        foundSilence_ = false;
        isFirstSilentSample_ = false;
        notifiedSilenceExceededThreeSeconds_ = false;
        numberOfSilentSamples_ = 0;
    }

    void FrameValidator::EncounteredSilence(int32_t iNumberSilentSamples)
    {
        SMPTE_SYNC_LOG << "FrameValidator::EncounteredSilence";
    }

    void FrameValidator::HandlePayload(void)
    {
        syncPacket_.marker_ = SYNCMARKER;
        syncPacket_.length_ = static_cast<uint16_t>(payloadLength_);
        syncPacket_.extensionLength_ = syncPacket_.length_ - BASE_PAYLOAD_LENGTH;

        uint8_t *framePtr = frame_;
        
        // Skip marker and length
        framePtr += 6 + 6;

        ReadUInt16(&framePtr, syncPacket_.flags_);
        ReadUInt32(&framePtr, syncPacket_.timelineEditUnitIndex_);
        ReadUInt32(&framePtr, syncPacket_.playoutID_);
        ReadUInt16(&framePtr, syncPacket_.editUnitDuration_);
        ReadUInt32(&framePtr, syncPacket_.sampleDurationEnum_);
        ReadUInt32(&framePtr, syncPacket_.sampleDurationDenom_);
        ReadInt32(&framePtr, syncPacket_.primaryPictureOutputOffset_);
        ReadUInt32(&framePtr, syncPacket_.primaryPictureScreenOffset_);
        ReadUInt32(&framePtr, syncPacket_.primaryPictureTrackFileEditUnitIndex_);
        ReadUUID(&framePtr, syncPacket_.primaryPictureTrackFileUUID_);
        ReadUInt32(&framePtr, syncPacket_.primarySoundTrackFileEditUnitIndex_);
        ReadUUID(&framePtr, syncPacket_.primarySoundTrackFileUUID_);
        ReadUUID(&framePtr, syncPacket_.compositionPlaylistUUID_);
        
        if (syncPacket_.extensionLength_ > 0)
        {
            syncPacket_.extension_ = nullptr;
            SMPTE_SYNC_LOG << "FrameValidator::HandlePayload"
            << " syncPacket_.extensionLength_ = "
            << syncPacket_.extensionLength_;
        }
        else
        {
            syncPacket_.extension_ = nullptr;
        }
        
        //this->PrintSyncPacket();
        
        // If the new frame's duration is different than our
        // currently allocated duration, we need to update
        // and reallocate the frame data
        //
        if (syncPacket_.editUnitDuration_ != currentFrameDuration_)
        {
            // The frame duration needs to be a reasonable size
            //
            if (syncPacket_.editUnitDuration_ == 0)
            {
                SMPTE_SYNC_LOG << "ERROR - syncPacket_.editUnitDuration_ = " << syncPacket_.editUnitDuration_;
                this->ResetFrame();
                return;
            }
            
            // Allocate a new frame
            //
            uint8_t *newFrame = new uint8_t[3 * syncPacket_.editUnitDuration_];
            
            if (newFrame == nullptr)
            {
                SMPTE_SYNC_LOG << "ERROR - newFrame == nulllptr";
                this->ResetFrame();
                return;
            }
            
            // Copy data from the original frame which is the data we just read
            //
            memcpy(newFrame, frame_, payloadSizeInBytes_);

            // Delete the old frame
            //
            delete [] frame_;
            
            // Save the pointer to the new frame
            //
            frame_ = newFrame;
            
            // Update the sizes for the new frame
            //
            currentFrameDuration_ = syncPacket_.editUnitDuration_;
            currentFrameSizeInBytes_ = 3 * currentFrameDuration_;
        }
    }

    void FrameValidator::PrintSyncPacket(void)
    {
        SMPTE_SYNC_LOG << "FrameValidator::PrintSyncPacket";
        SMPTE_SYNC_LOG << "editUnitDuration_ = " << syncPacket_.editUnitDuration_;
    }

    void FrameValidator::HandleFrame(void)
    {
    }

    const syncPacket& FrameValidator::GetSyncPacket(void)
    {
        return syncPacket_;
    }

    int32_t FrameValidator::ProcessSyncMarker(int32_t iCurrentSample)
    {
        if (lookingForSyncMarker_ || lookingForSyncMarker2sComplement_)
        {
            samplesUntilSyncMarker_++;

            if (lookingForSyncMarker_ && memcmp(syncMarker_, currentAudioBuffer_ + (iCurrentSample * 3), 3) == 0)
            {
                memcpy(frame_, currentAudioBuffer_ + (iCurrentSample * 3), 3);
                lookingForSyncMarker_ = false;
                iCurrentSample++;
                offsetIntoFrameInSamples_ = 1;
                this->ClearSilenceFlags();
                
                // Do we have enough samples for the tail structure of the sync marker?
                //
                if (iCurrentSample < bufferSize_)
                {
                    if (memcmp(syncMarkerTwosComplement_, currentAudioBuffer_ + (iCurrentSample * 3), 3) == 0)
                    {
                        // Copy the remaining sample buffer into our frame buffer
                        //
                        int32_t samplesToCopy = bufferSize_ - iCurrentSample;
                        if (samplesToCopy >= payloadDuration_)
                        {
                            // We already have 1 sample in our frame
                            // i.e the lead sync marker sample
                            //
                            samplesToCopy = payloadDuration_ - 1;
                        }
                        
                        memcpy(frame_ + (offsetIntoFrameInSamples_ * 3), currentAudioBuffer_ + (iCurrentSample * 3), samplesToCopy * 3);
                        offsetIntoFrameInSamples_ += samplesToCopy;
                        assert(offsetIntoFrameInSamples_ <= payloadDuration_);
                        lookingForSyncMarker2sComplement_ = false;

                        // Determine the number of extra samples between end of
                        // one frame and beginning of the next frame
                        // The sync marker and it's 2s complement take up 2 samples
                        //
                        if (samplesUntilSyncMarker_ > 1)
                        {
                            SMPTE_SYNC_LOG << "FrameValidator::ProcessSyncMarker Extra samples between frame and sync marker 1 samplesUntilSyncMarker_ = " << samplesUntilSyncMarker_ + 1;
                        }

                        iCurrentSample += samplesToCopy;
                    }
                    else
                    {
                        // We found a sync marker but not a 2s complement!
                        //
                        SMPTE_SYNC_LOG << "FrameValidator::ProcessSyncMarker We found a sync marker but not a 2s complement! ResetFrame 1\n";
                        this->ResetFrame();
                    }
                }
                else
                {
                    // The sync marker and it's 2s complement have been split across
                    // 2 audio buffers.
                    //
                    // We need to look in the next buffer to find the sync marker 2s complement
                    // as the first sample
                    //
                    SMPTE_SYNC_LOG << "FrameValidator::ProcessSyncMarker lead and tail structures of sync marker are split across audio buffers. 1\n";
                }
            }
            else
            if (!lookingForSyncMarker_
                && lookingForSyncMarker2sComplement_
                && memcmp(syncMarkerTwosComplement_, currentAudioBuffer_ + (iCurrentSample * 3), 3) == 0)
            {
                // Copy the remaining sample buffer into our frame buffer
                //
                int32_t samplesToCopy = bufferSize_ - iCurrentSample;
                if (samplesToCopy >= payloadDuration_)
                {
                    // We already have 1 sample in our frame
                    // i.e the lead sync marker sample
                    //
                    samplesToCopy = payloadDuration_ - 1;
                }

                memcpy(frame_ + (offsetIntoFrameInSamples_ * 3), currentAudioBuffer_ + (iCurrentSample * 3), samplesToCopy * 3);
                offsetIntoFrameInSamples_ += samplesToCopy;
                assert(offsetIntoFrameInSamples_ <= payloadDuration_);
                lookingForSyncMarker2sComplement_ = false;
                this->ClearSilenceFlags();
                
                // Determine the number of extra samples between end of
                // one frame and beginning of the next frame
                // The sync marker and it's 2s complement take up 2 samples
                //
                if (samplesUntilSyncMarker_ > 1)
                {
                    SMPTE_SYNC_LOG << "FrameValidator::ProcessSyncMarker Extra samples between frame and sync marker 2 samplesUntilSyncMarker_ = " << samplesUntilSyncMarker_ + 1;
                }

                iCurrentSample += samplesToCopy;
            }
            else
            if (!lookingForSyncMarker_ && lookingForSyncMarker2sComplement_)
            {
                // We found a sync marker but not a 2s complement!
                //
                SMPTE_SYNC_LOG << "FrameValidator::ProcessSyncMarker We found a sync marker but not a 2s complement! ResetFrame\n";
                this->ResetFrame();
            }
            else
            {
                // Test the remaining sample buffer for silence
                //
                int32_t samplesToTest = bufferSize_ - iCurrentSample;
                if (samplesToTest >= payloadDuration_)
                {
                    samplesToTest = payloadDuration_;
                }

                // Silence is considered as a buffer with all 0s that is not part of a valid frame
                // A typical frame has samples of data at the beginning followed by fill samples of 0s
                // A typical frame size is 2000 (48KHz at 24fps).
                // If there are 0s samples that are outside of the frame (2000),
                // that is considered silence
                //
                if (memcmp(silence_, currentAudioBuffer_ + (iCurrentSample * 3), samplesToTest * 3) == 0)
                {
                    if (!isFirstSilentSample_ && !foundSilence_)
                        isFirstSilentSample_ = true;
                    else
                        isFirstSilentSample_ = false;
                    
                    foundSilence_ = true;
                    numberOfSilentSamples_ += samplesToTest;
                    
                    // We did not find the sync marker.
                    // Advance through the audio sample buffer.
                    //
                    iCurrentSample += samplesToTest;
                }
                else
                {
                    // We did not find the sync marker.
                    // Advance through the audio sample buffer.
                    //
                    iCurrentSample++;

                    this->ClearSilenceFlags();
                }

                const int32_t threeSeconds = sampleRate_ * 3;
                
                // Notify when we find silence the first time
                // Notify when we have found greater than 3 seconds of silence
                //
                if ((foundSilence_ && isFirstSilentSample_)
                    || ((numberOfSilentSamples_ > threeSeconds) && !notifiedSilenceExceededThreeSeconds_))
                {
                    if (numberOfSilentSamples_ > threeSeconds)
                        notifiedSilenceExceededThreeSeconds_ = true;
                    
                    this->EncounteredSilence(numberOfSilentSamples_);
                }
            }
        }

        return iCurrentSample;
    }
    
    void FrameValidator::ParseFrames(void)
    {
        int currentSample = 0;
        while (currentSample < bufferSize_)
        {
            if (lookingForSyncMarker_ || lookingForSyncMarker2sComplement_)
            {
                currentSample = this->ProcessSyncMarker(currentSample);
            }
            else
            {
                int32_t samplesAvailable = bufferSize_ - currentSample;
                int32_t samplesNeeded = currentFrameDuration_ - offsetIntoFrameInSamples_;

                // If we have NOT collected enough samples for our payload
                // only copy over those samples
                //
                if (offsetIntoFrameInSamples_ < payloadDuration_)
                {
                    samplesNeeded = payloadDuration_ - offsetIntoFrameInSamples_;
                }
                
                // Copy the samples available.
                // But first make sure we have space to hold them.
                //
                int32_t samplesToCopy = samplesAvailable;
                
                // If we don't have space to store the samples in this frame
                // only copy the ones we have space for
                //
                if (samplesAvailable > samplesNeeded)
                {
                    samplesToCopy = samplesNeeded;
                }

                memcpy(frame_ + (offsetIntoFrameInSamples_ * 3), currentAudioBuffer_ + (currentSample * 3), samplesToCopy * 3);
                offsetIntoFrameInSamples_ += samplesToCopy;
                currentSample += samplesToCopy;
            }
            
            if (offsetIntoFrameInSamples_ == payloadDuration_)
            {
                if (this->ValidatePayload())
                {
                    this->HandlePayload();
                }
                else
                {
                    this->ResetFrame();
                }
            }
            else
            if (offsetIntoFrameInSamples_ == currentFrameDuration_)
            {
                if (this->ValidateFillSamples())
                {
                    this->HandleFrame();
                    this->ResetFrame();
                }
                else
                {
                    // If frame vaildation failed
                    // Do the extra step to see if we can find overlapping frames
                    // and shuffle the data if possible
                    //
                    if (this->CheckOverlappingFramesAndShuffle())
                    {
                        // We found an overlapping frame!
                        // Keep going collecting up samples and looking for the
                        // next full frame
                        //
                    }
                    else
                    {
                        // We did not find an overlapping frame
                        //
                        this->ResetFrame();
                    }
                }
            }
        }
    }
    
}  // namespace SMPTE_SYNC
