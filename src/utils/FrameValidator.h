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

#ifndef FRAMEVALIDATOR_H
#define FRAMEVALIDATOR_H

#include <string>

#include "sync.h"

namespace SMPTE_SYNC
{
    /**
     * @brief FrameValidator class implements parsing a 24-bit byte stream that contains syncPacket as specified
     * SMPTE ST 430-10:2010 D-Cinema Operations — Auxiliary Content Synchronization Protocol
     * This class is used as a base class for parsing the data it is derived from for specific implementations and handlers
     */
    class FrameValidator
    {
    public:

        /**
         * Constructor
         *
         * @param iSampleRate is the sample rate of the AES/EBU audio signal and DCP audio media
         * @param iBufferSize is size of a buffer of data being added to the FrameValidator for processing
         *
         */
        FrameValidator(int32_t iSampleRate
                       , int32_t iBufferSize);
        
        /// Destructor
        virtual ~FrameValidator();

        
        /**
         * Clients add data to the FrameValidator by calling AddSamples. 
         * The data is copied into a local buffer and ParseFrames is called. 
         * Adding data may a nondeterministic amount of time and allocate memory.
         *
         * @param iBuffer is buffer of 24-bit fixed point audio data. It is the size of bufferSize_
         *
         */
        virtual void AddSamples(const uint8_t *iBuffer);

        /**
         * 
         * Provides a base empty implementation for handling a frame. Derived classes should implement their specialized implementations.
         *
         */
        virtual void HandleFrame(void);
        
        /**
         *
         * Returns the last syncPacket that has been parsed
         *
         */
        virtual const syncPacket& GetSyncPacket(void);

        /**
         *
         * Resets the FrameValidator and any data that is has parsed.
         *
         */
        virtual void ResetFrame(void);
        
        /**
         *
         * Provides a base empty implementation for handling and notification of when silence has been encountered. 
         * Derived classes should implement their specialized implementations.
         *
         * @param iNumberSilentSamples is number of samples of samples since the end of the last valid syncPacket
         *
         */
        virtual void EncounteredSilence(int32_t iNumberSilentSamples);

    private:
        
        /**
         *
         * The core parser for processing audio data and populating a syncPacket
         *
         * Algorithm
         *
         * As soon as the lead sample struture for the sync marker is found
         * accumulate a frame's worth of samples.
         * Once you have a frames's worth of samples with the lead sample structure
         * of the sync marker as the first 3 bytes of the buffer,
         * then check to see if we have the tail sample structure for the sync marker.
         *
         * If we find a valid lead and tail sync marker as the first 2 samples (6 bytes),
         * then we validate there are no other sync markers (lead & tail) in the buffer.
         *
         * Once we have validated the buffered frame has the right sync markers,
         * then validate the length, followed by the payload (determined by the lenght),
         * followed by fill samples (padding, 0s) for the remaining frame.
         *
         * Once all of these have been validated, the frame is considered valid.
         *
         * If the frame fails to be valid there are several cases:
         *
         * Case 1: Multiple sync markers (could be caused by dropped buffers)
         * Solution 1: Skip to the last valid sync marker in the frame
         *             and accumulate more samples to fill the buffer.
         *
         * Case 2: Invalid payload - wrong length, mismatching lead vs. tail strutures
         * Solution 2: Skip the frame and look for new sync marker.
         *
         * Case 3: Invalid fill samples - not all 0s in the fill samples
         * Solution 3: Skip the frame and look for new sync marker.
         *
         * There are several cases we need to support for finding the sync marker
         *
         * Case 1: Sync marker is somwhere in the audio buffer.
         *         This could be at the first sample of an audio buffer or later in audio buffer.
         *
         * Case 2: Sync marker is at the very last sample of the audio buffer.
         *         In this case, the lead sample of the sync marker is in one audio buffer
         *         and the tail sample of the sync marker is the first sample in the next audio buffer..
         *
         */
        void ParseFrames(void);

        /**
         *
         * This is a specialized portion of the ParseFrames algorithm that checks each sample for the exisitence of a 
         * sync marker as defined by the syncPacket.
         * It returns the sample of the start of the sync marker. 
         * If a sync marker and it's 2s complement cannot be found it resets the frame.
         * When it encounters a valid sync marker, it copies the remaining data from the currentAudioBuffer_ into the frame_
         *
         */
        int32_t ProcessSyncMarker(int32_t iCurrentSample);

        /**
         *
         * Once a frames work of audio data has been accumulated and validated, HandlePayload is called to covert the 
         * audio data in a buffer to the syncPacket_ syncPacket object.
         *
         */
        void HandlePayload(void);

        /**
         *
         * Checks the frame_ to make sure the sync marker is correct. 
         * Additionally checks that there are not multiple sync markers in a frame_
         *
         * @return true/false if the sync marker is valid
         *
         */
        bool ValidateSyncMarkers(void);

        /**
         *
         * Checks that the payload of the syncPacket is validate.
         * Executes once a full payload has been accumulated
         *
         * @return true/false if the payload is valid
         *
         */
        bool ValidatePayload(void);

        /**
         *
         * Checks that the fill samples are all silence
         * If a non-zero (silent) value is found, this fails and returns false
         *
         * @return true/false if the fill samples are valid
         *
         */
        bool ValidateFillSamples(void);

        /**
         *
         * This function checks to see if there are overlapping syncPackets in the frame_
         * It looks for multiple sync markers in the frame_ buffer
         * If multiple sync markers are found, the memory is shuffled such that the last found
         * sync marker and all trailing data is moved to the beginning of the frame_
         *
         * There is a case where there could be a dropped audio buffer
         * which could cause a partial frame followed by a sync marker
         * for a new frame.
         *
         * We need to catch this case. When we do, we will move the
         * sync marker and associated buffered data to the beginning
         * of the frame_ buffer.
         *
         * Once we have moved this data to the front of the frame_ buffer
         * this will set up the frame_ buffer to continue to be populated
         * with the next audio buffer frame data.
         *
         * @return Return true if we found an otherlapping frame, Return false if we did not find an overlapping frame (should reset the frame)
         *
         */
        bool CheckOverlappingFramesAndShuffle(void);

        /**
         *
         * Checks each lead and tail sample has the proper 2s complement as the next sample
         *
         * @param iSample is the sample in the frame_ to start validating on
         *
         * @return true/false if the all samples have the proper 2s complement structure
         *
         */
        bool ValidateLeadAndTailStructures(int32_t iSample);

        /**
         *
         * Checks that the frame_ contains a valid sync marker as the first sample and the proper 2s complement as the second sample
         *
         * @return true/false if the there is a valid sync marker
         *
         */
        bool CheckSyncMarker();

        /**
         *
         * Clears any flags used for tracking silence between syncPacket
         *
         */
        void ClearSilenceFlags(void);

        /**
         *
         * Prints debugging information for the current syncPacket_
         *
         */
        void PrintSyncPacket(void);
        
        /// The current/last syncPacket packet that has been read from the AES/EBU signal
        syncPacket      syncPacket_;

        /// Sample rate of the AES/EBU (audio IO callback) and DCP audio media
        const int32_t   sampleRate_;
        
        /// The buffer size of the audio IO callback
        const int32_t   bufferSize_;

        /**
         *
         * A buffer of 24-bit fixed point audio data used to hold a frames worth of data
         * This is pre-allocated with the currentFrameSizeInBytes_, but if the frame size changes
         * it is deallocated and created again with new size as read in the syncPacket
         *
         */
        uint8_t         *frame_;
        
        /// The current audio buffer that has been added through AddSamples this is processed into a frame_
        uint8_t         *currentAudioBuffer_;

        /// The current frame count as determined by the syncPacket::timelineEditUnitIndex_
        int32_t         currentFrame_;

        /// The length of the payload for the syncPacket being read
        int32_t         payloadLength_;

        /// Number of samples in a packet. 44 samples per payload is being used. This is true until the extension is defined and used
        const uint32_t  payloadDuration_;

        /// Number of bytes in a packet this is 3 * payloadDuration_
        const uint32_t  payloadSizeInBytes_;
        
        /**
         *
         * Number of samples in a frame including fill samples. This is variable due to the editUnitDuration_.
         * This typically defaults to 2000 samples which is 48Khz at 24 frames
         *
         */
        uint32_t        currentFrameDuration_;
        
        /// currentFrameDuration_ in bytes
        uint32_t        currentFrameSizeInBytes_;
        
        /// The offset into frame_ used to determine how many samples have been consumed
        uint32_t        offsetIntoFrameInSamples_;
        
        /// The number of samples since the last sync maker. Used to deterine if there were extra samples between sync markers
        uint32_t        samplesUntilSyncMarker_;
        
        /// A preallocated buffer representing the sync marker. Used for comparison with bytes in the byte buffer
        uint8_t         *syncMarker_;

        /// A preallocated buffer representing the 2s complement sync marker. Used for comparison with bytes in the byte buffer
        uint8_t         *syncMarkerTwosComplement_;

        /// This flag is set track if the byte buffer needs to be searched for a sync marker. This is set to true in ResetFrame
        bool            lookingForSyncMarker_;

        /// This flag is set track if the byte buffer needs to be searched for a sync marker. This is set to true in ResetFrame
        bool            lookingForSyncMarker2sComplement_;

        /// A preallocated buffer representing 24-bit fixed point sample of silence. That a buffer of all 0s
        uint8_t         *silence_;
        
        /// Set when silence has been found
        bool            foundSilence_;

        /// Set when the first sample of silence has been found
        bool            isFirstSilentSample_;
        
        /// Set when at least 3 seconds of silence is found. This is used to determine when to call EncounteredSilence
        bool            notifiedSilenceExceededThreeSeconds_;
        
        /// Tracks the number of samples of silence to notify EncounteredSilence about
        int32_t         numberOfSilentSamples_;
    };

}  // namespace SMPTE_SYNC

#endif // FRAMEVALIDATOR_H
