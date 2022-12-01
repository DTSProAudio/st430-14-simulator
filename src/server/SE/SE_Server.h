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

#ifndef SE_SERVER_H
#define SE_SERVER_H

#include "SE_State.h"

#include <string>

#include "boost/thread/thread.hpp"
#include "boost/lockfree/queue.hpp"

#include "sync.h"
#include "DataTypes.h"

//#define COPY_BUFFERS_SERVER

namespace SMPTE_SYNC
{
    namespace UTILS {
        class ConverterInt24Float32;
    }

    /**
     *
     * @brief SE_Server class implements syncPacket generator and play state machine for the server
     *
     */

    class SE_Server : public SE_State
    {
    public:

        /**
         *
         * Constructor
         *
         * @param iSampleRate is the sample rate of the AES/EBU signal and DCP audio media
         * @param iCallbackBufferSize is size in samples of the audio IO callbacks
         * @param iMaxFrameDurationInSamples is number of samples for the largest frame in the DCP. This is typically 2000 for 24fps at 48Khz. This value is used to allocate space for the buffer containing the audio data to be played out.
         * @param iProcessingWaitTime is delay before playing out a syncPacket when going from a pause or stopped state to a play state. Used in delay when switching between CPLs.
         *
         */
        SE_Server(int32_t iSampleRate
                  , int32_t iCallbackBufferSize
                  , int32_t iMaxFrameDurationInSamples
                  , int32_t iProcessingWaitTime
                  );

        /// Destructor
        virtual ~SE_Server(void);
        
        /**
         *
         * Resets the SE_Server to an initial state. 
         * The state_ is set to eState_NoData. 
         * The current or partially populated syncPacket is cleared. 
         * The SE_Server starts to accumulate and parse audio data that is read from the audio IO callback
         *
         */
        void Reset(void);

        /// TODO: Possibly remove this call and have the only initialization done through the constructor of the SE_Server.
        /**
         *
         * Initializes the SE_Server with new information. 
         * Initialization is only possible when the state is eState_NoData.
         *
         * @param iSampleRate is the sample rate of the AES/EBU signal and DCP audio media
         * @param iMaxFrameDurationInSamples is number of samples for the largest frame in the DCP. This is typically 2000 for 24fps at 48Khz. This value is used to allocate space for the buffer containing the audio data to be played out.
         * @param iShowLengthInFrames is length of the entire show based on all loaded CPL objects
         * @return true/false if the initialization succeeded
         *
         */
        bool Initialize(int32_t iSampleRate
                        , int32_t iMaxFrameDurationInSamples
                        , int32_t iShowLengthInFrames);
        
        /**
         *
         * Sets the playoutID_ of the DCS such that the SE_Server can generate the syncPacket with the proper playout id
         * Only possible to set this when the state is eState_NoData
         *
         * @param iID is the playout id of the DCS
         * @return true/false if the playout id was set successfully
         *
         */
        void SetPlayoutID(uint32_t iID);

        /**
         *
         * Sets the primaryPictureOutputOffset_ of the DCS such that the SE_Server can generate the syncPacket with the offset information
         * Only possible to set this when the state is eState_NoData
         *
         * @param iOffset is the offset value
         * @return true/false if the offset was set successfully
         *
         */
        bool SetPrimaryPictureOutputOffset(int32_t iOffset);

        /**
         *
         * Sets the primaryPictureScreenOffset_ of the DCS such that the SE_Server can generate the syncPacket with the offset information
         * Only possible to set this when the state is eState_NoData
         *
         * @param iOffset is the offset value
         * @return true/false if the offset was set successfully
         *
         */
        bool SetPrimaryPictureScreenOffset(uint32_t iOffset);

        /**
         *
         * Sets the SE_Server play position
         * Pauses the SE_Server before setting the position
         *
         * @param iFrameNumber is the frame to set. Frame 0 is the beginning of the Show
         *
         */
        void SetFrame(int32_t iFrameNumber);

        /**
         *
         * Sets the SE_Server play position to frame 0 which is the beginning of the Show
         * Pauses the SE_Server before setting the position
         *
         */
        void ReturnToStart();

        /**
         *
         * Pauses the SE_Server
         *
         */
        void Pause();

        /**
         *
         * Stops the playback of the SE_Server
         *
         */
        void Stop();

        /**
         *
         * Starts the playback of the SE_Server
         *
         */
        void Play();
        
        /**
         *
         * Implements the callback for copying data into the output buffers. This is called from the audio IO callback thread. It is critical not to block or consume much CPU time in this call
         * Audio buffers are popped from the sampleQueue_ and memcpy is used to copy the audio data into the outputChannelData. Once the buffer is copied it will be added to the unusedSampleQueue_
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
         * Sets the SE_Server play position
         * Pauses the SE_Server before setting the position
         *
         * @param iFrameNumber is the frame to set. Frame 0 is the beginning of the Show
         *
         */
        int32_t GetCurrentFrame(void);

        /**
         *
         * Sets the flag isProcessorReady_ which is used to determine when playback can start. 
         * Playback should only start when the processor is ready or when the SE_Server times out and starts ignorning this flag.
         *
         * @param iReady is ready or not
         *
         */
        void SetProcessorIsReady(bool iReady);

        /**
         *
         * Sets a callback for requesting data back a specific frame
         *
         * @param iCallback is the callback for requesting frame data
         *
         */
        void SetGetFrameDataCallback(GetFrameDataCallback iCallback);

    private:

        /**
         *
         * SetupPacket builds the specific syncPacket by setting the various parameters and serializing the data into a 24-bit audio buffer
         *
         * @return true/false if the syncPacket was successfully built
         *
         */
        bool SetupPacket(void);

        /**
         *
         * Function that generates frames. Called by the frameBuilderThread_. 
         * This thread runs approximately ever threadSleepTimeInMS_ which is 2x faster than the queue depth to ensure it
         * is filled in a timely fashion
         *
         */
        void BuildFrames(void);

        /// Data that is filled by GetFrameDataCallback and used to generate a syncPacket
        FrameInfo       frameData_;

        /// The sample rate of both the AES/EBU audio signal and the DCP audio media
        uint32_t        sampleRate_;
        
        /// The playout id of the DCS
        boost::atomic<uint32_t>        playoutID_;
        
        /// The offset as configured by the user
        int32_t         primaryPictureOutputOffset_;

        /// The offset as configured by the user
        uint32_t        primaryPictureScreenOffset_;

        /**
         *
         * The maximum frame duration measured in number of samples from a Show. This is used to allocate memory store the 24-bit audio data for a frame
         * A maximum is used as the frame rate can change over the course of the show and we want to allocate our memory on initialization and not on changes.
         *
         */
        int32_t         maxFrameDuration_;
        
        /// The duration of the frame currently being read as specified in the syncPacket
        int32_t         currentFrameDuration_;

        /// The size in bytes of a 24-bit audio signal of the frame currently being read as specified in the syncPacket
        int32_t         currentFrameSize_;

        /// The number of audio samples per audio IO callback
        int32_t         callbackBufferSize_;

        /**
         *
         * The offset in samples into the current audio buffer read from the audio IO callback is tracked.
         * This is used since an audio buffer is not an even multiple of a frame size and the frame may not start at the beginning of an audio buffer
         *
         */
        int32_t         offsetIntoCurrentAudioBuffer_;

        /**
         *
         * The offset in samples into the current syncPacket frame is tracked.
         * This is used since it typically takes multiple audio buffers to fill a syncPacket frame.
         *
         */
        int32_t         offsetIntoFrame_;

        /**
         *
         * A syncPacket object used to serialize the sync information into a 24-bit audio signal
         *
         */
        syncPacket      syncSamp_;

        /// The length of a show in frames which is used by the SE_Server to stop playback at the end of the show
        int32_t         showLengthInFrames_;
        
        /// The currentFrame_ is updated when in the state eState_Playing. Multiple clients from multiple threads read this value.
        boost::atomic<int32_t>         currentFrame_;

        /// A preallocated  buffer of 24-bit data to store a syncPacket frame in
        uint8_t         *currentFrameBuffer_;
        
        /**
         *
         * This is the audio buffer that was popped off of the sampleQueue_ and is being copied into a currentFrameBuffer_
         * This is tracked independently as the currentAudioBuffer_ may span multiple syncPacket frames.
         *
         */
        float           *currentAudioBuffer_;
        
        /// Conversion utility for converting from the internal audio IO callback unite of 32-bit floating point audio to 24-bit fixed point of the syncPacket data
        UTILS::ConverterInt24Float32   *converter_;

        /// The callback function for getting frame data information to be played back
        GetFrameDataCallback frameDataCallback_;
        
        
        /// TODO: Finish implementation for delay between CPL
        /// A delay before playing back.
        int32_t         processingWaitTime_;

        /// Protects the flag isProcessorReady_ and playStarTimeInSeconds_ which is used to determine when playback can start.
        boost::mutex    isProcessorReadyMutex_;

        /// Used to determine when playback can start.
        bool            isProcessorReady_;

        /// Used to track the time when play start was initiated and when it should begin.
        time_t          playStarTimeInSeconds_;

        /// Used to guard resetting the SE_Server play engine
        boost::mutex    needsResetMutex_;

        /**
         *
         * This thread runs approximately ever threadSleepTimeInMS_ which is 2x faster than the queue depth to ensure it
         * is filled in a timely fashion. This calls SE_Server::BuildFrames
         *
         */
        boost::thread                   frameBuilderThread_;
        
        /**
         *
         * Flag to keep BuildFrames being called as long as the thread is running. Set to false in the SE_Server destructure so the thread can complete
         * (calling join on the thread) before exiting the destructor
         *
         */
        boost::atomic<bool>             keepBuildingFrames_;
        
        /// The number of milliseconds to sleep the thread which is 2x faster than the queue depth to ensure it is filled in a timely fashion
        int32_t                         threadSleepTimeInMS_;

        /**
         *
         * The boost::lockfree::queue of a fixed size is used for managing audio buffers coming from the audio IO subsystem.
         * This is to prevent nondeterministic time spent in the audio callback.
         * The unusedSampleQueue_ is created and populated with a fix number of audio buffers in the SE_Server construtor.
         * Buffers of audio are moved between the unusedSampleQueue_ and sampleQueue_ in the audio IO callback thread and the
         * BuildFrames thread. There will never be more than the fixed number of buffers available. Pushing onto these queues
         * should never fail and is logged as an error if it occurs.
         * Popping from these queues can fail and is considered an underrun condition and is logged as an error if it occurs.
         *
         * The sample queues store pre-allocated buffers of float arrays
         * that are the size of an audio buffer for the audioDeviceIOCallback
         * The audio buffers are typically 32, 64, 128, 256, 512, etc in size
         *
         * If the audio callback buffer size is changed, the buffers need to be
         * properly reset/rebuilt (drain, delete, allocate, fill)
         *
         */
        typedef boost::lockfree::queue<float*, boost::lockfree::fixed_sized<true>> SampleQueue;
        
        /// Queue of preallocated and currently unused audio buffers
        SampleQueue     *unusedSampleQueue_;

        /// Queue of audio buffers received on the audio IO callback thread
        SampleQueue     *sampleQueue_;

#ifdef COPY_BUFFERS_SERVER
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

#endif // SE_SERVER_H
