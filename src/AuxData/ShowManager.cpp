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

#include "ShowManager.h"

#include <assert.h>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>

#include "CPLParser.h"
#include "AuxDataParser.h"
#include "Logger.h"
#include "Utils.h"
#include "Show.h"

namespace SMPTE_SYNC
{
    
    ShowManager::ShowManager(int32_t iSampleRate) :
          sampleRate_(iSampleRate)
        , auxDataParser_(nullptr)
        , show_(nullptr)
        , showLoaded_(false)
    {
        SMPTE_SYNC_LOG << "ShowManager::ShowManager\n";
    }

    ShowManager::ShowManager(int32_t iSampleRate
                             , const CPLFileList &iCPLList) :
          sampleRate_(iSampleRate)
        , auxDataParser_(nullptr)
        , show_(nullptr)
    {
        SMPTE_SYNC_LOG << "ShowManager::ShowManager\n";

        CPLList_ = iCPLList;
        
        this->Load();
    }
    
    ShowManager::~ShowManager()
    {
        SMPTE_SYNC_LOG << "ShowManager::~ShowManager";

        delete auxDataParser_;
        auxDataParser_ = nullptr;
    }

    bool ShowManager::AddCPL(const std::string &iCPLPath)
    {
        bool success = true;
        
        CPLParser *cplParser = new CPLParser(iCPLPath);
        assert(cplParser != nullptr);
        
        if (!cplParser->IsFileCPL())
            success = false;;
        
        if (success)
            CPLList_.push_back(iCPLPath);
        
        delete cplParser;
        
        return success;
    }
    
    bool ShowManager::AddCPLList(const CPLFileList &iCPLList)
    {
        bool success = true;

        for (CPLFileList::const_iterator iter = iCPLList.begin(); iter != iCPLList.end() && success; iter++)
        {
            CPLParser *cplParser = new CPLParser(*iter);
            if (cplParser->IsFileCPL())
            {
                success = false;
            }
            delete cplParser;
        }

        if (success)
            CPLList_.insert(CPLList_.end(), iCPLList.begin(), iCPLList.end());

        return success;
    }

    bool ShowManager::Reset(void)
    {
        SMPTE_SYNC_LOG << "ShowManager::Reset";

        showLoaded_ = false;
        
        delete show_;
        show_ = nullptr;
        
        CPLList_.clear();
        
        return true;
    }
    
    bool ShowManager::Load(void)
    {
        SMPTE_SYNC_LOG << "ShowManager::Load";

        if (show_ != nullptr)
            return false;
        
        if (CPLList_.size() == 0)
            return false;
        
        assert(show_ == nullptr);
        
        show_ = new Show(sampleRate_);
        
        for (CPLFileList::iterator iter = CPLList_.begin(); iter != CPLList_.end(); iter++)
        {
            CPLParser *cplParser = new CPLParser(*iter);
            CPL *newCPL = cplParser->Parse();

            delete cplParser;

            if (newCPL != nullptr)
                show_->AddCPLToEndOfTimeline(newCPL);
        }
        
        
        if (show_->GetLengthInFrames() == 0)
            return false;
        
        showLoaded_ = true;

        return true;
    }

    bool ShowManager::IsShowLoaded(void)
    {
        return showLoaded_;
    }
    
    int32_t ShowManager::GetLongestFrameLength(void)
    {
        if (show_ == nullptr)
            return 0;
        
        int32_t frameTime = 0;
        
        if (show_ == nullptr)
            return frameTime;
        
        frameTime = show_->GetLongestFrameLength();
        
        return frameTime;
    }

    int32_t ShowManager::GetLengthInFrames(void)
    {
        if (show_ == nullptr)
            return 0;
        
        int32_t length = 0;
        
        if (show_ == nullptr)
            return length;
        
        length = show_->GetLengthInFrames();
        
        SMPTE_SYNC_LOG << "ShowManager::GetLengthInFrames " << length;

        return length;
    }

    int32_t ShowManager::GetSampleRate(void)
    {
        return sampleRate_;
    }
    
    bool ShowManager::GetFrame(int32_t iFrame, FrameInfo& oFrameInfo)
    {
        if (show_ == nullptr)
            return false;
        
        return show_->GetAssetFrameInfo(iFrame, oFrameInfo);
    }
    
    bool ShowManager::OpenAuxDataParser(int32_t iStartFrame)
    {
        if (auxDataParser_ != nullptr)
            return false;

        int32_t startFrame = 0;
        int32_t endFrame = 0;

        bool frameAvailable = show_->GetAssetRangeForFrame(iStartFrame
                                     , Asset::eAssetType_AuxData
                                     , startFrame
                                     , endFrame);
        if (frameAvailable)
        {
            std::string auxDataFilePath = show_->GetDataFilePath(startFrame, Asset::eAssetType_AuxData);
            
            auxDataParser_ = new AuxDataParser(startFrame, endFrame, auxDataFilePath);
            
            if (auxDataParser_->Open())
            {
                return true;
            }
            else
            {
                SMPTE_SYNC_LOG << "ShowManager::Load - Failed to open auxDataFilePath = " << auxDataFilePath;
                return false;
            }
        }
        
        return false;
    }

    bool ShowManager::GetDataItems(const std::string &iDataEssenceCodingUL_,
                                 int32_t iStart,
                                 int32_t iCount,
                                 const std::string &iEncryptionType,
                                 std::vector<char>& oContent)
    {
        SMPTE_SYNC_LOG << "ShowManager::GetDataItems iDataEssenceCodingUL_ = "
        << iDataEssenceCodingUL_
        << " iStart = "
        << iStart
        << " iCount = "
        << iCount
        << " iEncryptionType = "
        << iEncryptionType;
        
        if (!this->IsShowLoaded())
            return false;
        
        uint8_t *oDataItem = nullptr;
        uint32_t oDataItemSize = 0;

        int32_t startFrame = iStart;
        int32_t endFrame = iStart + iCount;
        int32_t itemsRead = 0;

        // Write the payload
        //
        while (startFrame < endFrame)
        {
            if (auxDataParser_ != nullptr)
            {
                // If our requested frame is outside of the range of our current auxDataParser_
                // we need to delete the current parser and then load a new parser
                //
                if (startFrame < auxDataParser_->GetStartFrame() || auxDataParser_->GetEndFrame() < startFrame)
                {
                    delete auxDataParser_;
                    auxDataParser_ = nullptr;

                    if (!this->OpenAuxDataParser(startFrame))
                    {
                        SMPTE_SYNC_LOG << "ShowManager::GetDataItems - unable to OpenAuxDataParser for startFrame = " << startFrame;
                        break;
                    }
                }
            }
            else
            {
                if (!this->OpenAuxDataParser(startFrame))
                {
                    SMPTE_SYNC_LOG << "ShowManager::GetDataItems - unable to OpenAuxDataParser for startFrame = " << startFrame;
                    break;
                }
            }
            
            if (auxDataParser_->GetDataItem(startFrame, &oDataItem, oDataItemSize))
            {
                SMPTE_SYNC_LOG << "ShowManager::GetDataItems - startFrame = " << startFrame;

                itemsRead++;
                AuxDataBlock *auxData = new AuxDataBlock;
                
                auxData->editUnitIndex_ = startFrame;

                FrameInfo frameInfo;
                show_->GetAssetFrameInfo(startFrame, frameInfo);

                auxData->editUnitRateNumerator_ = frameInfo.editUnitRateNumerator_;
                auxData->editUnitRateDenominator_ = frameInfo.editUnitRateDenominator_;

                auxData->sourceDataEssenceCodingUL_.SetFromString(frameInfo.dataEssenceCodingUL_);
                auxData->sourceDataItemLength_ = oDataItemSize;
                auxData->sourceDataItem_ = new uint8_t[auxData->sourceDataItemLength_];
                memcpy(auxData->sourceDataItem_, oDataItem, auxData->sourceDataItemLength_);
                
                int32_t dataSize = auxData->GetSizeInBytes();
                uint8_t *buf = new uint8_t[dataSize];
                uint8_t *bufOrig = buf;
                memset(buf, 0, dataSize);
                
                auxData->write(&buf);
                
                std::string tmp((const char*)bufOrig, dataSize);
                
                oContent.insert(oContent.end(), tmp.begin(), tmp.end());

                delete [] oDataItem;
                // Make sure we set it to null as the next loop will
                // reuse this same pointer
                // If GetDataItem fails we should never get here,
                // practice defensiving programming anyways.
                //
                oDataItem = nullptr;
                
                delete [] bufOrig;
                delete auxData;
            }
            else
            {
                break;
            }
            
            startFrame++;
        }
        
        // Now that we have read all of our items,
        // We need to setup the header properly
        // and then PRE-PEND the header to the
        // beginning of the data.
        //
        
        // Setup and write the header
        //
        AuxDataBlockTransferHeader header;
        header.editUnitRangeStartIndex_ = iStart;
        header.editUnitRangeCount_ = itemsRead;
        
        int32_t headerSize = header.GetSizeInBytes();
        uint8_t *headerBuf = new uint8_t[headerSize];
        uint8_t *headerBufOrig = headerBuf;
        memset(headerBuf, 0, headerSize);
        
        header.write(&headerBuf);
        
        std::string tmp((const char*)headerBufOrig, headerSize);
        
        oContent.insert(oContent.begin(), tmp.begin(), tmp.end());
        
        delete [] headerBufOrig;
        
        return true;
    }

}  // namespace SMPTE_SYNC
