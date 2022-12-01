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

#ifndef SHOW_H
#define SHOW_H

#include <assert.h>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <vector>

#include "DataTypes.h"
#include "UUID.h"

namespace SMPTE_SYNC
{
    /**
     *
     * @brief Asset represents the asset from a XML based CPL file
     *
     */

    class Asset
    {
    public:

        /**
         *
         * @enum AssetType
         *
         * @brief Defines the different types of assets that can be stored from a CPL XML file
         *
         */

        typedef enum AssetType {
              eAssetType_Unknown                /**< Identifies the unknown asset type. This is the default asset type */
            , eAssetType_MainPicture            /**< Identifies the main picture asset type */
            , eAssetType_MainSound              /**< Identifies the main sound asset type */
            , eAssetType_AuxData                /**< Identifies the aux data asset type */
        } AssetType;
        
        /// Helper method for converting from an AssetType enum to a string for displaying to the user
        std::string AssetTypeToString(AssetType iType);
        
        /// Constructor
        Asset();

        /// Destructor
        ~Asset();
        
        /**
         *
         * Returns the start frame for the Asset
         *
         * @return int32_t value containing the start frame
         *
         */
        int32_t GetStartFrame(void);

        /**
         *
         * Returns the end frame for the Asset
         *
         * @return int32_t value containing the end frame
         *
         */
        int32_t GetEndFrame(void);
        
        /**
         *
         * Prints out the asset information to SMPTE_SYNC_LOG
         *
         */
        void print(void);
        
        /// Type of asset
        AssetType       type_;

        /// The UUID of the asset
        UUID            id_;
        
        /// Edit rate numerator
        int32_t         editRateNumerator_;

        /// Edit rate demoninator
        int32_t         editRateDenominator_;
        
        /// Frame rate numerator
        int32_t         frameRateNumerator_;

        /// Frame rate demoninator
        int32_t         frameRateDenominator_;
        
        /// Start frame is the postion in the timeline where this asset begins
        int32_t         startFrame_;

        /// Intrinsic duration of the asset
        int32_t         intrinsicDuration_;

        /// Entry point of the asset
        int32_t         entryPoint_;

        /// Duration of the asset
        int32_t         duration_;

        /// Data essence coding UL
        std::string     dataEssenceCodingUL_;

        /// File path to the asset
        std::string     path_;

        /// Volume index for the asset
        int32_t         volumeIndex_;

        /// Offset into the file for the asset
        int32_t         offset_;

        /// Length of the asset
        int32_t         length_;
    };

    /**
     *
     * @brief Reel represents the information abour reels in a CPL XML file
     *
     */
    
    class Reel
    {
    public:

        /// Constructor
        Reel();

        /// Destructor
        ~Reel();
        
        /// The UUID of the Reel
        UUID                id_;
        
        // Vector of assets contained in a Reel
        std::vector<Asset*> assets_;
    };

    /**
     *
     * @brief CPL represents the information contained in a specific CPL XML file
     *
     */

    class CPL
    {
    public:

        /// Constructor
        CPL();

        /// Destructor
        ~CPL();
        
        /// The UUID of the CPL
        UUID                id_;

        // Vector of Reels contained in a CPL
        std::vector<Reel*> reels_;
    };

    /**
     *
     * @brief Show represents media to be played out on the timeline. It is composed of a series of one or more CPLs
     *
     */

    class Show
    {
    public:

        /**
         *
         * Contructor
         *
         * @param iSampleRate is the base sample rate of the Show as specified by the audio subsystem used to playout the AES/EBU signal
         *
         */
        Show(int32_t iSampleRate);

        /// Destructor
        ~Show();
        
        /**
         *
         * Prints out the information about the CPLs in the show to SMPTE_SYNC_LOG
         *
         */
        void print(void);

        /**
         *
         * Computes the start and end frames of a specific asset for a specific frame on the timeline.
         * Since the Show timeline is composed of a vector of CPL objects, the frame could be in a different CPL and Asset
         *
         * @param iFrame is requested frame on the Show timeline
         * @param iType is requested AssetType type
         * @param oStartFrame is, if found, the start frame of the specific Asset for the requested iFrame in Show timeline
         * @param oEndFrame is, if found, the end frame of the specific Asset for the requested iFrame in Show timeline
         * @return bool true/false if the requested iFrame and Asset were found
         *
         */
        bool GetAssetRangeForFrame(  int32_t iFrame
                                   , Asset::AssetType iType
                                   , int32_t &oStartFrame
                                   , int32_t &oEndFrame);

        /**
         *
         * Finds the path to the specific file containing the requested Asset for a specific frame on the timeline.
         * Since the Show timeline is composed of a vector of CPL objects, the frame could be in a different CPL and Asset
         *
         * @param iFrame is requested frame on the Show timeline
         * @param iType is requested AssetType type
         * @return std::string to the file for the iFrame and Asset. This is an empty string if the iFrame and Asset could not be found
         *
         */
        std::string GetDataFilePath(int32_t iFrame, Asset::AssetType iType);
        
        /**
         *
         * Computes the longest frame found in the show.
         * For example, in 24 vs. 25 frames 24 frames is the longest frame as it takes more time or wall clock time to present to the viewer
         * Supported rates are 24, 25, 30, 48, and 50
         *
         * @return int32_t of the longest frame
         *
         */
        int32_t GetLongestFrameLength(void);

        /**
         *
         * Computes the length of the Show in frames
         *
         * @return int32_t of Show length
         *
         */
        int32_t GetLengthInFrames(void);

        /**
         *
         * Returns the FrameInfo if available in the Show
         * Searches all loaded CPL objecdts in the Show timeline to find a specific Asset base don the iFrame requested
         *
         * @param iFrame is requested frame on the Show timeline
         * @param oFrameInfo is FrameInfo for the requested iFrame
         * @return bool true/false if the requested iFrame was found
         *
         */
        bool GetAssetFrameInfo(int32_t iFrame, FrameInfo &oFrameInfo);

        /**
         *
         * The client to the Show adds a CPL object to the end of the Show timeline
         *
         * @param iCPL is the CPL to add to the end of the Show timeline
         * @return bool true/false if the CPL was properly added to the end of the Show timeline
         *
         */
        bool AddCPLToEndOfTimeline(CPL *iCPL);
        
    private:
        
        /// Vector of CPL pointers that represent the Show timeline. The CPL objects are listed in the order they should appear during playback
        std::vector<CPL*> timeline_;

        /// Sample rate of the Show
        int32_t sampleRate_;
        
        /// Total number of rames in the show timeline. Adding a new CPL to the Show timeline adds to the total number of frames
        int32_t numberOfFrames_;
    };

}  // namespace SMPTE_SYNC

#endif // SHOW_H
