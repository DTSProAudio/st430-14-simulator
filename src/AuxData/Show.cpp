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

#include "Show.h"

#include <assert.h>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <algorithm>
#include <vector>
#include <iterator>

#include <boost/algorithm/hex.hpp>

#include "Logger.h"
#include "Utils.h"

namespace SMPTE_SYNC
{
    Asset::Asset() :
          type_(eAssetType_Unknown)
        , editRateNumerator_(0)
        , editRateDenominator_(1)
        , frameRateNumerator_(0)
        , frameRateDenominator_(1)
        , intrinsicDuration_(0)
        , entryPoint_(0)
        , duration_(0)
        , dataEssenceCodingUL_("")
        , path_("")
        , volumeIndex_(0)
        , offset_(0)
        , length_(0)
    {
        SMPTE_SYNC_LOG << "Asset::Asset";

        Initialize(id_);
    }
    
    Asset::~Asset()
    {
        SMPTE_SYNC_LOG << "Asset::~Asset";
    }
    
    int32_t Asset::GetStartFrame(void)
    {
        return startFrame_;
    }
    
    int32_t Asset::GetEndFrame(void)
    {
        int32_t endFrame = 0;

        if (duration_ != 0)
            endFrame = startFrame_ + duration_ - 1;
        
        return endFrame;
    }

    std::string UUIDTypeToString(UUID uuid)
    {
        std::string uuid_str;
        boost::algorithm::hex(uuid, uuid + sizeof(UUID), std::back_inserter(uuid_str));
        return uuid_str;
    }

    void Asset::print(void)
    {
        SMPTE_SYNC_LOG << "\n\nAsset...";
        
        SMPTE_SYNC_LOG << "type_ = " << this->AssetTypeToString(type_);
        SMPTE_SYNC_LOG << "id_ = " << UUIDTypeToString(id_);
        SMPTE_SYNC_LOG << "editRateNumerator_ = " << editRateNumerator_;
        SMPTE_SYNC_LOG << "editRateDenominator_ = " << editRateDenominator_;
        SMPTE_SYNC_LOG << "frameRateNumerator_ = " << frameRateNumerator_;
        SMPTE_SYNC_LOG << "frameRateDenominator_ = " << frameRateDenominator_;
        SMPTE_SYNC_LOG << "intrinsicDuration_ = " << intrinsicDuration_;
        SMPTE_SYNC_LOG << "entryPoint_ = " << entryPoint_;
        SMPTE_SYNC_LOG << "duration_ = " << duration_;
        SMPTE_SYNC_LOG << "dataEssenceCodingUL_ = " << dataEssenceCodingUL_;
        SMPTE_SYNC_LOG << "path_ = " << path_;
        SMPTE_SYNC_LOG << "volumeIndex_ = " << volumeIndex_;
        SMPTE_SYNC_LOG << "offset_ = " << offset_;
        SMPTE_SYNC_LOG << "length_ = " << length_;
    }

    std::string Asset::AssetTypeToString(AssetType iType)
    {
        std::string typeStr = "";
        
        switch (iType) {
            case eAssetType_Unknown:
                typeStr = "eAssetType_Unknown";
                break;

            case eAssetType_MainPicture:
                typeStr = "eAssetType_MainPicture";
                break;

            case eAssetType_MainSound:
                typeStr = "eAssetType_MainSound";
                break;

            case eAssetType_AuxData:
                typeStr = "eAssetType_AuxData";
                break;

            default:
                break;
        }
        
        return typeStr;
    }
    
    Reel::Reel()
    {
        SMPTE_SYNC_LOG << "Reel::Reel";

        Initialize(id_);
    }
    
    Reel::~Reel()
    {
        SMPTE_SYNC_LOG << "Reel::~Reel";

        for (std::vector<Asset*>::iterator iter = assets_.begin(); iter != assets_.end(); iter++)
        {
            delete *iter;
        }
    }

    CPL::CPL()
    {
        SMPTE_SYNC_LOG << "CPL::CPL";

        Initialize(id_);
    }

    CPL::~CPL()
    {
        SMPTE_SYNC_LOG << "CPL::~CPL";

        for (std::vector<Reel*>::iterator iter = reels_.begin(); iter != reels_.end(); iter++)
        {
            delete *iter;
        }
    }

    Show::Show(int32_t iSampleRate) :
          sampleRate_(iSampleRate)
        , numberOfFrames_(0)
    {
        SMPTE_SYNC_LOG << "Show::Show";
    }
    
    Show::~Show()
    {
        SMPTE_SYNC_LOG << "Show::~Show";
        for (std::vector<CPL*>::iterator iter = timeline_.begin(); iter != timeline_.end(); iter++)
        {
            delete *iter;
        }
    }
    
    void Show::print(void)
    {
        SMPTE_SYNC_LOG << "\n\nShow...";
        
    }
    
    int32_t Show::GetLengthInFrames(void)
    {
        return numberOfFrames_;
    }

    bool Show::AddCPLToEndOfTimeline(CPL *iCPL)
    {
        SMPTE_SYNC_LOG << "Show::AddCPLToEndOfTimeline";
        assert(iCPL != nullptr);

        bool success = true;
        
        // For convenience, we set the Asset's startFrame_
        // and update the Show's numberOfFrames_
        //
        for (std::vector<Reel*>::iterator reelIter = iCPL->reels_.begin(); reelIter != iCPL->reels_.end(); reelIter++)
        {
            int32_t currentFrameCount = numberOfFrames_;
            
            for (std::vector<Asset*>::iterator assetIter = (*reelIter)->assets_.begin(); assetIter != (*reelIter)->assets_.end(); assetIter++)
            {
                (*assetIter)->startFrame_ = currentFrameCount;
                if ((*assetIter)->type_ == Asset::eAssetType_MainPicture)
                {
                    // Update the number of frames based on the duration of the main picture asset
                    //
                    numberOfFrames_ += (*assetIter)->duration_;
                }
            }
        }

        timeline_.push_back(iCPL);
        
        return success;
    }

    bool Show::GetAssetRangeForFrame(  int32_t iFrame
                                       , Asset::AssetType iType
                                       , int32_t &oStartFrame
                                       , int32_t &oEndFrame)
    {
        oStartFrame = 0;
        oEndFrame = 0;
        
        for (std::vector<CPL*>::iterator cplIter = timeline_.begin(); cplIter != timeline_.end(); cplIter++)
        {
            for (std::vector<Reel*>::iterator reelIter = (*cplIter)->reels_.begin(); reelIter != (*cplIter)->reels_.end(); reelIter++)
            {
                for (std::vector<Asset*>::iterator assetIter = (*reelIter)->assets_.begin(); assetIter != (*reelIter)->assets_.end(); assetIter++)
                {
                    if ((*assetIter)->type_ == iType)
                    {
                        if ((*assetIter)->GetStartFrame() <= iFrame && iFrame <= (*assetIter)->GetEndFrame())
                        {
                            oStartFrame = (*assetIter)->GetStartFrame();
                            oEndFrame = (*assetIter)->GetEndFrame();
                            
                            return true;
                        }
                    }
                }
            }
        }

        return false;
    }

    std::string Show::GetDataFilePath(int32_t iFrame, Asset::AssetType iType)
    {
        std::string path = "";
        
        for (std::vector<CPL*>::iterator cplIter = timeline_.begin(); cplIter != timeline_.end(); cplIter++)
        {
            for (std::vector<Reel*>::iterator reelIter = (*cplIter)->reels_.begin(); reelIter != (*cplIter)->reels_.end(); reelIter++)
            {
                for (std::vector<Asset*>::iterator assetIter = (*reelIter)->assets_.begin(); assetIter != (*reelIter)->assets_.end(); assetIter++)
                {
                    if ((*assetIter)->type_ == iType)
                    {
                        if ((*assetIter)->GetStartFrame() <= iFrame && iFrame <= (*assetIter)->GetEndFrame())
                        {
                            path = (*assetIter)->path_;
                            return path;
                        }
                    }
                }
            }
        }
        
        return path;
    }
    
    int32_t Show::GetLongestFrameLength(void)
    {
        int32_t longestFrame = 1;
        
        for (std::vector<CPL*>::iterator cplIter = timeline_.begin(); cplIter != timeline_.end(); cplIter++)
        {
            for (std::vector<Reel*>::iterator reelIter = (*cplIter)->reels_.begin(); reelIter != (*cplIter)->reels_.end(); reelIter++)
            {
                for (std::vector<Asset*>::iterator assetIter = (*reelIter)->assets_.begin(); assetIter != (*reelIter)->assets_.end(); assetIter++)
                {
                    int32_t fps = 0;
                    fps = (*assetIter)->editRateNumerator_ / (*assetIter)->editRateDenominator_;
                    
                    int32_t frameLength = sampleRate_ / fps;

                    if (frameLength > longestFrame)
                        longestFrame = frameLength;
                }
            }
        }
        
        return longestFrame;
    }

    bool Show::GetAssetFrameInfo(int32_t iFrame, FrameInfo &oFrameInfo)
    {
        bool foundFrame = false;
        
        oFrameInfo.Reset();
        
        for (std::vector<CPL*>::iterator cplIter = timeline_.begin(); cplIter != timeline_.end(); cplIter++)
        {
            for (std::vector<Reel*>::iterator reelIter = (*cplIter)->reels_.begin(); reelIter != (*cplIter)->reels_.end(); reelIter++)
            {
                for (std::vector<Asset*>::iterator assetIter = (*reelIter)->assets_.begin(); assetIter != (*reelIter)->assets_.end(); assetIter++)
                {
                    // There is always a main picture
                    // set the currentFrameDuration_ based on the main picture
                    //
                    if ((*assetIter)->type_ == Asset::eAssetType_MainPicture)
                    {
                        if ((*assetIter)->GetStartFrame() <= iFrame && iFrame <= (*assetIter)->GetEndFrame())
                        {
                            foundFrame = true;
                            
                            oFrameInfo.currentFrameDuration_ =  sampleRate_ / ((*assetIter)->editRateNumerator_ / (*assetIter)->editRateDenominator_);
                            oFrameInfo.primaryPictureTrackFileEditUnitIndex_ = iFrame;
                            Copy((*assetIter)->id_, oFrameInfo.primaryPictureTrackFileUUID_);
                            oFrameInfo.editUnitRateNumerator_ = (*assetIter)->editRateNumerator_;
                            oFrameInfo.editUnitRateDenominator_ = (*assetIter)->editRateDenominator_;
                        }
                    }
                    
                    if (foundFrame && (*assetIter)->type_ == Asset::eAssetType_MainSound)
                    {
                        oFrameInfo.primarySoundTrackFileEditUnitIndex_ = iFrame;
                        Copy((*assetIter)->id_, oFrameInfo.primarySoundTrackFileUUID_);
                        
                        // Get the CPL id from the CPL iterator
                        //
                        Copy((*cplIter)->id_, oFrameInfo.compositionPlaylistUUID_);
                    }

                    if (foundFrame && (*assetIter)->type_ == Asset::eAssetType_AuxData)
                    {
                        oFrameInfo.dataEssenceCodingUL_ = (*assetIter)->dataEssenceCodingUL_;
                    }
                }

                if (foundFrame)
                    return foundFrame;
            }
        }
        
        return foundFrame;
    }

}  // namespace SMPTE_SYNC
