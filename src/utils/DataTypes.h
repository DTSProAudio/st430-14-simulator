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

#ifndef DATATYPES_H_INCLUDED
#define DATATYPES_H_INCLUDED

#include <stdint.h>
#include <string>
#include <deque>
#include <vector>

#include "boost/function.hpp"

#include "sync.h"

namespace SMPTE_SYNC {

    /**
     * 
     * The DCS port as defined by SMPTE ST 430-10:2010 D-Cinema Operations — Auxiliary Content Synchronization Protocol
     *
     */
    static uint16_t sDCS_Port = 4170;

    /**
     *
     * The text used for plaintest ACCEPT-KIND as defined in
     * SMPTE ST 430-14:2015 D-Cinema Operations – Digital Sync Signal and Aux Data Transfer Protocol
     *
     */
    static std::string sPlainText = "plaintext";

    /**
     *
     * The text used for encrypted ACCEPT-KIND as defined in
     * SMPTE ST 430-14:2015 D-Cinema Operations – Digital Sync Signal and Aux Data Transfer Protocol
     *
     */
    static std::string sEncrypted = "encrypted";
    
    /**
     *
     * @brief FrameInfo struct is used to collect information together for a specific frame.
     * The SE_Server requests this information from the Show
     *
     * @struct FrameInfo
     *
     */
    typedef struct FrameInfo
    {
        /// Constructor
        FrameInfo()
        {
            this->Reset();
        }
        
        /// Resets the data used in the FrameInfo to an initial state
        void Reset(void)
        {
            currentFrameDuration_ = 0;
            primaryPictureTrackFileEditUnitIndex_ = 0;
            Initialize(primaryPictureTrackFileUUID_);
            primarySoundTrackFileEditUnitIndex_ = 0;
            Initialize(primarySoundTrackFileUUID_);
            Initialize(compositionPlaylistUUID_);
            dataEssenceCodingUL_ = "";
            editUnitRateNumerator_ = 0;
            editUnitRateDenominator_ = 0;
        }
        
        /// The duration of the current frame
        uint32_t    currentFrameDuration_;
        uint32_t    primaryPictureTrackFileEditUnitIndex_;
        UUID        primaryPictureTrackFileUUID_;
        uint32_t    primarySoundTrackFileEditUnitIndex_;
        UUID        primarySoundTrackFileUUID_;
        UUID        compositionPlaylistUUID_;
        std::string          dataEssenceCodingUL_;
        int32_t    editUnitRateNumerator_;
        int32_t    editUnitRateDenominator_;
    } FrameInfo;

    /**
     *
     * @brief This is a list of path to CPL files. The CPL objects will be added to the Show based on the order they appear in this list.
     *
     */
    typedef std::vector<std::string> CPLFileList;

    /**
     *
     * @brief Audio IO Callback function definition. 32-bit floating point data was selected due to the common usage of this type in various audio frameworks and subsystems
     *
     */
    typedef boost::function<void(const float** inputChannelData,
                                 int numInputChannels,
                                 float** outputChannelData,
                                 int numOutputChannels,
                                 int numSamples)> AudioCallback;

    /**
     *
     * @brief Callback function definition for setting a RPLLocation
     *
     */
    typedef boost::function<void(const std::string &iPath)> SetRPLLocationCallback;

    /**
     *
     * @brief Callback function definition for requesting the current frame being played back by the SE_Server or received by the SE_Client
     *
     */
    typedef boost::function<int32_t(void)> CurrentFrameCallback;

    /**
     *
     * @brief Callback function definition for determining if the client/processor is ready to start start receiving a play state for the syncPacket
     *
     */
    typedef boost::function<void(bool)> IsReadyCallback;

    /**
     *
     * @brief Callback function definition for updating the playout ID
     *
     */
    typedef boost::function<void(uint32_t)> SetPlayoutIDCallback;

    /**
     *
     * @brief Callback function definition for getting the current frame information from the Show. Used by SE_Server.
     *
     */
    typedef boost::function<bool(int32_t iFrame, FrameInfo& oFrameInfo)> GetFrameDataCallback;
    
} // namespace SMPTE_SYNC

#endif  // DATATYPES_H_INCLUDED

