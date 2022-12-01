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

#include "CPLParser.h"

#include <assert.h>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <algorithm>
#include <vector>

#include "tinyxml.h"

#include "Logger.h"
#include "Show.h"
#include "Utils.h"

namespace SMPTE_SYNC
{
    static const std::string sAssetMapFileName = "ASSETMAP.xml";

    // XML constants
    //
    static const std::string SMPTE_SYNC_ASSET_UIR_BASE = "urn:dts:m2x:asset:";
    static const std::string SMPTE_SYNC_COMPOSITIONPLAYLIST = "CompositionPlaylist";
    static const std::string SMPTE_SYNC_REELLIST = "ReelList";
    static const std::string SMPTE_SYNC_REEL = "Reel";
    static const std::string SMPTE_SYNC_ASSETLIST = "AssetList";

    static const std::string SMPTE_SYNC_AUXDATA = "axd-cpl:AuxData";
    static const std::string SMPTE_SYNC_MAINSOUND = "MainSound";
    static const std::string SMPTE_SYNC_MAINPICTURE = "MainPicture";

    static const std::string SMPTE_SYNC_ID = "Id";
    static const std::string SMPTE_SYNC_EDITRATE = "EditRate";
    static const std::string SMPTE_SYNC_FRAMERATE = "FrameRate";
    static const std::string SMPTE_SYNC_INTRINSICDURATION = "IntrinsicDuration";
    static const std::string SMPTE_SYNC_ENTRYPOINT = "EntryPoint";
    static const std::string SMPTE_SYNC_DURATION = "Duration";
    static const std::string SMPTE_SYNC_DATAESSENCECODING = "axd-cpl:DataEssenceCoding";
    static const std::string SMPTE_SYNC_ASSET = "Asset";
    static const std::string SMPTE_SYNC_CHUNKLIST = "ChunkList";
    static const std::string SMPTE_SYNC_CHUNK = "Chunk";
    static const std::string SMPTE_SYNC_PATH = "Path";
    static const std::string SMPTE_SYNC_VOLUMEINDEX = "VolumeIndex";
    static const std::string SMPTE_SYNC_OFFSET = "Offset";
    static const std::string SMPTE_SYNC_LENGTH = "Length";
    static const std::string SMPTE_SYNC_ASSETMAP = "AssetMap";

#define STRINGS_EQUAL 0
    
    CPLParser::CPLParser(const std::string &iPath) :
        pathToCurrentCPL_(iPath)
    {
        SMPTE_SYNC_LOG << "CPLParser::CPLParser";
    }
    
    CPLParser::~CPLParser()
    {
        SMPTE_SYNC_LOG << "CPLParser::~CPLParser";

    }
    
    bool CPLParser::IsFileCPL(void)
    {
        SMPTE_SYNC_LOG << "CPLParser::IsFileCPL pathToCurrentCPL_ = " << pathToCurrentCPL_ << "\n";
        bool isCPL = false;
        
        std::ifstream inputStream(pathToCurrentCPL_, std::ifstream::in);
        
        if (!inputStream.good())
        {
            SMPTE_SYNC_LOG << "Input file cannot be read. Wrong filename?" << std::endl;
            return false;
        }
        
        inputStream.seekg(0, std::ios::end);
        std::streampos sizeOfFile = inputStream.tellg();
        inputStream.seekg(0, std::ios::beg);
        
        char *buf = new char[sizeOfFile];
        inputStream.read(buf, sizeOfFile);
        
        TiXmlDocument cplXMLDoc;
        cplXMLDoc.Parse(buf, 0, TIXML_ENCODING_UTF8);
        
        delete [] buf;

        TiXmlHandle cplXMLDocHandle( &cplXMLDoc );
        TiXmlNode* cplNode = cplXMLDocHandle.FirstChild(SMPTE_SYNC_COMPOSITIONPLAYLIST).ToElement();
        
        if (cplNode != nullptr)
            isCPL = true;

        return isCPL;
    }

    CPL* CPLParser::Parse(void)
    {
        SMPTE_SYNC_LOG << "CPLParser::ParseCPL pathToCurrentCPL_ = " << pathToCurrentCPL_ << "\n";
        
        if (this->IsFileCPL() == false)
            return nullptr;
        
        std::ifstream inputStream(pathToCurrentCPL_, std::ifstream::in);

        if (!inputStream.good())
        {
            SMPTE_SYNC_LOG << "Input file cannot be read. Wrong filename?" << std::endl;
            return nullptr;
        }

        inputStream.seekg(0, std::ios::end);
        std::streampos sizeOfFile = inputStream.tellg();
        inputStream.seekg(0, std::ios::beg);

        char *buf = new char[sizeOfFile];
        inputStream.read(buf, sizeOfFile);
        
        TiXmlDocument cplXMLDoc;
        cplXMLDoc.Parse(buf, 0, TIXML_ENCODING_UTF8);
        
        delete [] buf;
        
        this->ParseAssetMap();

        TiXmlHandle cplXMLDocHandle( &cplXMLDoc );
        TiXmlElement* reelList = cplXMLDocHandle.FirstChild(SMPTE_SYNC_COMPOSITIONPLAYLIST).FirstChild(SMPTE_SYNC_REELLIST).ToElement();

        CPL *cpl = nullptr;

        if (reelList)
        {
            cpl = new CPL();
            
            // Read the ID of the CPL
            //
            TiXmlElement* parameter = cplXMLDocHandle.FirstChild(SMPTE_SYNC_COMPOSITIONPLAYLIST).FirstChild(SMPTE_SYNC_ID).ToElement();
            if (parameter)
            {
                std::string idStr = parameter->GetText();
                
                SMPTE_SYNC_LOG << idStr;
                // Strip off the urn:uuid:
                //
                idStr = idStr.substr(idStr.rfind(":") + 1, idStr.length());
                StringToUUID(idStr, cpl->id_);
            }

            this->MapReels(reelList, cpl);
        }
        
        return cpl;
    }
    
    void CPLParser::MapReels(TiXmlNode* iNode, CPL *iCPL)
    {
        if (iNode == nullptr)
            return;
        
        if (iCPL == nullptr)
            return;
        
        for (TiXmlNode *child = iNode->FirstChild(); child != nullptr; child = child->NextSibling())
        {
            std::string val = child->Value();
            if (val.compare(SMPTE_SYNC_REEL) == STRINGS_EQUAL)
            {
                Reel *reel = new Reel();
                
                // Read the ID of the CPL
                //
                TiXmlElement* parameter = child->FirstChild(SMPTE_SYNC_ID)->ToElement();
                if (parameter)
                {
                    std::string idStr = parameter->GetText();
                    
                    SMPTE_SYNC_LOG << idStr;
                    // Strip off the urn:uuid:
                    //
                    idStr = idStr.substr(idStr.rfind(":") + 1, idStr.length());
                    StringToUUID(idStr, reel->id_);
                }

                iCPL->reels_.push_back(reel);
                
                TiXmlHandle handle(child);
                TiXmlElement* assetList = handle.FirstChild(SMPTE_SYNC_ASSETLIST).ToElement();
                this->MapAssets(assetList, reel);
            }
        }
    }

    void CPLParser::MapAssets(TiXmlNode* iNode, Reel *iReel)
    {
        if (iNode == nullptr)
            return;
        
        for (TiXmlNode *child = iNode->FirstChild(); child != nullptr; child = child->NextSibling())
        {
            std::string val = child->Value();

            bool supportedType = false;
            if (
                val.compare(SMPTE_SYNC_MAINPICTURE) == STRINGS_EQUAL
                || val.compare(SMPTE_SYNC_MAINSOUND) == STRINGS_EQUAL
                || val.compare(SMPTE_SYNC_AUXDATA) == STRINGS_EQUAL
                )
            {
                supportedType = true;
            }
            
            if (!supportedType)
                continue;
            
            Asset *asset = new Asset();
            iReel->assets_.push_back(asset);

            if (val.compare(SMPTE_SYNC_MAINPICTURE) == STRINGS_EQUAL)
                asset->type_ = Asset::eAssetType_MainPicture;

            if (val.compare(SMPTE_SYNC_MAINSOUND) == STRINGS_EQUAL)
                asset->type_ = Asset::eAssetType_MainSound;

            if (val.compare(SMPTE_SYNC_AUXDATA) == STRINGS_EQUAL)
                asset->type_ = Asset::eAssetType_AuxData;

            TiXmlHandle handle(child);
            
            TiXmlElement* parameter = handle.FirstChild(SMPTE_SYNC_ID).ToElement();
            if (parameter)
            {
                std::string idStr = parameter->GetText();
                
                SMPTE_SYNC_LOG << idStr;
                // Strip off the urn:uuid:
                //
                idStr = idStr.substr(idStr.rfind(":") + 1, idStr.length());
                StringToUUID(idStr, asset->id_);

                this->UpdateAssetPathInfo(asset);
            }
            
            parameter = handle.FirstChild(SMPTE_SYNC_EDITRATE).ToElement();
            if (parameter)
            {
                std::string editRateStr = parameter->GetText();
                
                asset->editRateNumerator_ = atoi(editRateStr.substr(0, editRateStr.find(" ")).c_str());
                asset->editRateDenominator_ = atoi(editRateStr.substr(editRateStr.find(" "), editRateStr.length()).c_str());

                SMPTE_SYNC_LOG << parameter->GetText();
                SMPTE_SYNC_LOG << "asset->editRateNumerator_ = " << asset->editRateNumerator_;
                SMPTE_SYNC_LOG << "asset->editRateDenominator_ = " << asset->editRateDenominator_;
            }

            parameter = handle.FirstChild(SMPTE_SYNC_FRAMERATE).ToElement();
            if (parameter)
            {
                std::string frameRateStr = parameter->GetText();
                
                asset->frameRateNumerator_ = atoi(frameRateStr.substr(0, frameRateStr.find(" ")).c_str());
                asset->frameRateDenominator_ = atoi(frameRateStr.substr(frameRateStr.find(" "), frameRateStr.length()).c_str());
                
                SMPTE_SYNC_LOG << parameter->GetText();
                SMPTE_SYNC_LOG << "asset->frameRateNumerator_ = " << asset->frameRateNumerator_;
                SMPTE_SYNC_LOG << "asset->frameRateDenominator_ = " << asset->frameRateDenominator_;
            }

            parameter = handle.FirstChild(SMPTE_SYNC_INTRINSICDURATION).ToElement();
            if (parameter)
            {
                SMPTE_SYNC_LOG << parameter->GetText();
                asset->intrinsicDuration_ = atoi(parameter->GetText());
            }
            
            parameter = handle.FirstChild(SMPTE_SYNC_ENTRYPOINT).ToElement();
            if (parameter)
            {
                SMPTE_SYNC_LOG << parameter->GetText();
                asset->entryPoint_ = atoi(parameter->GetText());
            }
            
            parameter = handle.FirstChild(SMPTE_SYNC_DURATION).ToElement();
            if (parameter)
            {
                SMPTE_SYNC_LOG << parameter->GetText();
                asset->duration_ = atoi(parameter->GetText());
            }
            
            parameter = handle.FirstChild(SMPTE_SYNC_DATAESSENCECODING).ToElement();
            if (parameter)
            {
                SMPTE_SYNC_LOG << parameter->GetText();
                asset->dataEssenceCodingUL_ = parameter->GetText();
            }
        }
    }

    bool CPLParser::UpdateAssetPathInfo(Asset* iAsset)
    {
        SMPTE_SYNC_LOG << "CPLParser::UpdateAssetPathInfo";
        assert(iAsset != nullptr);

        for (std::vector<AssetFileInfo>::iterator iter = assetMap_.begin(); iter != assetMap_.end(); iter++)
        {
            if (IsEqual(iAsset->id_, iter->id_))
            {
                if ( iter->chunkList_.size() > 0)
                {
                    iAsset->path_ = iter->chunkList_[0].path_;
                    iAsset->volumeIndex_ = iter->chunkList_[0].volumeIndex_;
                    iAsset->offset_ = iter->chunkList_[0].offset_;
                    iAsset->length_ = iter->chunkList_[0].length_;

                    return true;
                }
            }
        }
        
        return false;
    }

    void CPLParser::ParseAssetMap(void)
    {
        SMPTE_SYNC_LOG << "CPLParser::ParseAssetMap";

        std::string pathToCPLDirectory = pathToCurrentCPL_.substr(0, pathToCurrentCPL_.rfind("/") + 1);
        std::string filename = pathToCPLDirectory + sAssetMapFileName;

        SMPTE_SYNC_LOG << "Using asset map: " << filename;

        std::ifstream inputStream(filename, std::ifstream::in);
        
        if (!inputStream.good())
        {
            SMPTE_SYNC_LOG << "Input file cannot be read. Wrong filename?" << std::endl;
            return;
        }
        
        inputStream.seekg(0, std::ios::end);
        std::streampos sizeOfFile = inputStream.tellg();
        inputStream.seekg(0, std::ios::beg);
        
        char *buf = new char[sizeOfFile];
        inputStream.read(buf, sizeOfFile);
        
        TiXmlDocument cplXMLDoc;
        cplXMLDoc.Parse(buf, 0, TIXML_ENCODING_UTF8);
        
        delete [] buf;
        
        TiXmlHandle cplXMLDocHandle( &cplXMLDoc );
        TiXmlElement* assetList = cplXMLDocHandle.FirstChild(SMPTE_SYNC_ASSETMAP).FirstChild(SMPTE_SYNC_ASSETLIST).ToElement();
        
        if (assetList)
        {
            for (TiXmlNode *child = assetList->FirstChild(); child != nullptr; child = child->NextSibling())
            {
                std::string val = child->Value();
                if (val.compare(SMPTE_SYNC_ASSET) == STRINGS_EQUAL)
                {
                    AssetFileInfo fileInfo;
                    
                    TiXmlHandle handle(child);
                    TiXmlElement* parameter = handle.FirstChild(SMPTE_SYNC_ID).ToElement();
                    if (parameter)
                    {
                        std::string idStr = parameter->GetText();
                        SMPTE_SYNC_LOG << idStr;

                        // Strip off the urn:uuid:
                        //
                        idStr = idStr.substr(idStr.rfind(":") + 1, idStr.length());
                        StringToUUID(idStr, fileInfo.id_);
                        
                        TiXmlElement *chunkElement = handle.FirstChild(SMPTE_SYNC_CHUNKLIST).FirstChild(SMPTE_SYNC_CHUNK).ToElement();
                        if (chunkElement)
                        {
                            Chunk chunk;
                            
                            TiXmlHandle chunkHandle(chunkElement);
                            
                            TiXmlElement* parameter = chunkHandle.FirstChild(SMPTE_SYNC_PATH).ToElement();
                            if (parameter)
                            {
                                SMPTE_SYNC_LOG << parameter->GetText();
                                
                                /// TODO: Validate this is correct
                                chunk.path_ = parameter->GetText();
                                chunk.path_ = pathToCPLDirectory + chunk.path_;
                            }
                            
                            parameter = chunkHandle.FirstChild(SMPTE_SYNC_VOLUMEINDEX).ToElement();
                            if (parameter)
                            {
                                SMPTE_SYNC_LOG << parameter->GetText();
                                chunk.volumeIndex_ = atoi(parameter->GetText());
                            }
                            
                            parameter = chunkHandle.FirstChild(SMPTE_SYNC_OFFSET).ToElement();
                            if (parameter)
                            {
                                SMPTE_SYNC_LOG << parameter->GetText();
                                chunk.offset_ = atoi(parameter->GetText());
                            }
                            
                            parameter = chunkHandle.FirstChild(SMPTE_SYNC_LENGTH).ToElement();
                            if (parameter)
                            {
                                SMPTE_SYNC_LOG << parameter->GetText();
                                chunk.length_ = atoi(parameter->GetText());
                            }
                            
                            fileInfo.chunkList_.push_back(chunk);
                        }
                    }
                    assetMap_.push_back(fileInfo);
                }
            }
        }
    }
    
}  // namespace SMPTE_SYNC
