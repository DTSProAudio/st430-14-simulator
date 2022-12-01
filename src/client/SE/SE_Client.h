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

#ifndef SE_CLIENT_H
#define SE_CLIENT_H

#include <stdint.h>
#include <iostream>
#include <istream>
#include <ostream>
#include <string>
#include <queue>

#include "boost/thread/thread.hpp"
#include "boost/lockfree/queue.hpp"
#include "boost/atomic.hpp"

#include "SE_State.h"
#include "Client_Validator.h"
#include "FrameValidator.h"

//#define COPY_BUFFERS_CLIENT

namespace SMPTE_SYNC
{
    namespace UTILS {
        class ConverterInt24Float32;
    }
    
    /**
     *
     * @brief SE_Client class implements the sync emitter client that reads a byte stream and decodes the syncPacket signal sent from the SE_Server.
     *
     * Implements for the client of a sync emitter server as defined by
     * SMPTE ST 430-10:2010 D-Cinema Operations — Auxiliary Content Synchronization Protocol
     *
     */

    class SE_Client : public SE_State, FrameValidator
    {
    public:

        /**
         *
         * Constructor
         *
         * @param iSampleRate is the sample rate of the AES/EBU audio callback layer and the sample rate of the DCP
         * @param iCallbackBufferSize is the buffer size of the audio callbacks. This is typically 128, 256, 512, 1024, etc.
         *
         */
        SE_Client(int32_t iSampleRate
                  , int32_t iCallbackBufferSize);

        /// Destructor
        virtual ~SE_Client();

        /**
         *
         * Implements the audio callback handler function for processing audio data. This callback is called from the
         * audio thread thus it is important to spend as little and consistent time in this call as possible.
         * No memory allocations are allowed. The audio buffers are copied onto the sampleQueue_ for processing in the
         * frameParsingThread_ thread.
         *
         * @param inputChannelData is a pointer to a pointer of the input audio data. This is an array of arrays of audio samples.
         * The arrays correspond to each input channel with the maximum defined as numInputChannels.
         * @param numInputChannels is the maximum number of input channels available during this call
         * @param outputChannelData is a pointer to a pointer of the output audio data. This is an array of arrays of audio samples.
         * The arrays correspond to each input channel with the maximum defined as numOutputChannels.
         * @param numOutputChannels is the maximum number of output channels available during this call
         * @param numSamples is the number of samples contained in an audio buffer. This should correspond to the callbackBufferSize_ of the system
         *
         */
        void audioDeviceIOCallback(const float** inputChannelData,
                                   int numInputChannels,
                                   float** outputChannelData,
                                   int numOutputChannels,
                                   int numSamples);

        /**
         *
         * Sets the callback for the syncPacket validator.
         *
         * @param iCallback is the callback function for the validator function
         *
         */
        void SetValidatorCallback(SyncPacketValidatorCallback iCallback);

        /**
         *
         * Sets the callback for the syncPacket validator.
         *
         * @return The frame number that has been last decoded from the syncPacket.
         *
         */
        int32_t GetCurrentFrame(void);

        /**
         *
         * Implements and overrides the FrameValidator::HandleFrame function.
         * This sets the state of the SE_Client based on the current syncPacket.
         * It updates the currentFrame_ and calls the validator_ callback if installed.
         *
         */
        virtual void HandleFrame(void);

        /// TODO: Remove this override as it provides no specialization.
        /**
         *
         * Calls the base class FrameValidator::ResetFrame
         *
         */
        virtual void ResetFrame(void);

        /**
         *
         * Called by FrameValidator::EncounteredSilence when one or more samples of silence is detected.
         * Provides specialization by tracking when more than 3 seconds of silence has occurred and sets the state to eState_NoData
         *
         * @param iNumberSilentSamples is number of silent samples encountered before calling this function.
         *
         */
        virtual void EncounteredSilence(int32_t iNumberSilentSamples);

    private:

        /**
         *
         * Runs on the frameParsingThread_ to parse data from the syncPacket signal.
         * This thread runs approximately every other callbackBufferSize_. 
         * Audio data is converted from 32 bit floating point to 24 bit fixed point.
         * Once the samples are converted to 24 bit fixed point the audio is added to the 
         * FrameValidator::AddSamples for processing.
         *
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
         *         and the tail sample of the sync marker is the first sample in the next audio buffer.
         *
         */
        void ParseFrames(void);
        
        /// A buffer of 24 bit audio data of callbackBufferSize_ samples (3 bytes per sample)
        uint8_t         *currentAudioBufferInt24_;

        /**
         *
         * The number of audio samples per syncPacket frame. Variable and updated based on the
         * last frame read from the syncPacket. Typically 2000 samples for 24 frames at 48KHz or 1960 samples for 25 frames at 48KHz
         *
         */
        uint32_t        currentFrameDuration_;

        /// Number of samples in an audio callback
        int32_t         callbackBufferSize_;
        
        /// The approximate time in milliseconds for an audio callback. Approximately 10ms per 512 samples at 48KHz.
        int32_t         callbackBufferTimeInMS_;
        
        /// The current frame read from the syncPacket. This read from multiple threads.
        boost::atomic<int32_t>         currentFrame_;
        
        /// Conversion utitilty for converting 24 bit fixed point to 32 bit floating point audio data.
        UTILS::ConverterInt24Float32   *converter_;
        
        /// Sample rate of the audio system. This is the sample rate of the audio callback (AES/EBU signal) and the sample rate of the media (DCP)
        const int32_t  sampleRate_;

        /// Boost thread for parsing frames. Calls SE_Client::ParseFrames
        boost::thread                   frameParsingThread_;

        /**
         *
         * Flag to keep parsing as long as the thread is running. Set to false in the SE_Client destructure so the thread can complete 
         * (calling join on the thread) before exiting the destructor
         *
         */
        boost::atomic<bool>             keepParsing_;

        /**
         *
         * The boost::lockfree::queue of a fixed size is used for managing audio buffers coming from the audio IO subsystem.
         * This is to prevent nondeterministic time spent in the audio callback.
         * The unusedSampleQueue_ is created and populated with a fix number of audio buffers in the SE_Client construtor.
         * Buffers of audio are moved between the unusedSampleQueue_ and sampleQueue_ in the audio IO callback thread and the
         * ParseFrames thread. There will never be more than the fixed number of buffers available. Pushing onto these queues
         * should never fail and is logged as an error if it occurs.
         * Popping from these queues can fail and is considered an underrun condition and is logged as an error if it occurs.
         *
         */
        typedef boost::lockfree::queue<float*, boost::lockfree::fixed_sized<true>> SampleQueue;

        /// Queue of preallocated and currently unused audio buffers
        SampleQueue     *unusedSampleQueue_;
        
        /// Queue of audio buffers received on the audio IO callback thread
        SampleQueue     *sampleQueue_;

        /// This is the callback to the validator installed by the client
        SyncPacketValidatorCallback validator_;

#ifdef COPY_BUFFERS_CLIENT
        /**
         *
         * The following variables are used for diagnostics of the audio data by capturing the data into a large preallocated
         * buffer and written to file ever copyBufferNumMinutes_.
         *
         */
        int32_t         copyBufferNumMinutes_;
        float           *copyBuffer_;
        float           *copyBufferCopyPostion_;
        int32_t         copyBufferSamplesAvailable_;
#endif
    };

}  // namespace SMPTE_SYNC

#endif // SEs_CLIENT_H
