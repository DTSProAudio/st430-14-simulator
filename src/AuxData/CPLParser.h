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


#ifndef CPLPARSER_H
#define CPLPARSER_H

#include <assert.h>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <vector>
#include "tinyxml.h"

#include "DataTypes.h"

namespace SMPTE_SYNC
{

    class CPL;
    class Reel;
    class Asset;
    
    /**
     * @brief Chunk represents the file information that is stored in a ASSETMAP.xml in the DCP
     *
     */

    class Chunk
    {
    public:

        /// Constructor
        Chunk() :
              path_("")
            , volumeIndex_(0)
            , offset_(0)
            , length_(0)
        {
            
        }
        
        /// Destructor
        ~Chunk()
        {
            
        }
        
        /// The path to the file
        std::string path_;

        /// The volume index of the file
        int32_t volumeIndex_;

        /// TODO: Is this correct?
        /// The offset into the file
        int32_t offset_;

        /// The length of the file
        int32_t length_;
    };

    /**
     * @brief AssetFileInfo represents various assets stored in the CPL such as MainPicture, MainSound, and axd-cpl:AuxData
     *
     */

    class AssetFileInfo
    {
    public:

        /// Constructor
        AssetFileInfo()
        {
            
        }

        /// Destructor
        ~AssetFileInfo()
        {
            
        }
        
        /// The UUID of the asset
        UUID id_;

        /// Vector of chunks in the CPL
        std::vector<Chunk> chunkList_;
    };
    
    /**
     * @brief CPLParser parses a specific CPL and assocaited ASSETMAP.xml file.
     *
     */

    class CPLParser
    {
    public:

        /**
         *
         * Constructor
         * Parses a CPL.
         *
         * @param iPath is the path to the CPL to be parsed
         *
         */
        CPLParser(const std::string &iPath);
        
        /// Destructor
        virtual ~CPLParser();
        
        /**
         *
         * Parses a specific CPL pointed to by the iPath and assumes an ASSETMAP.xml that resides next to the CPL
         * Creates a new CPL object. The client is responsible for deleting the CPL
         *
         * @return CPL pointer, nullptr if the parsing failed
         *
         */
        CPL* Parse(void);

        /**
         *
         * Determines if a XML file is a CPL file.
         * It does not determine if the CPL is completely valid or not, but determines if 
         * the file has a CompositionPlaylist XML element
         *
         * @return true/false if the XML file is a CPL file.
         *
         */
        bool IsFileCPL(void);

    private:

        /**
         *
         * Parses the ASSETMAP.xml file associated with the CPL.
         *
         */
        void ParseAssetMap(void);

        /**
         *
         * Updates the path to the Asset. This information comes from teh ASSETMAP.xml file.
         *
         * @param iAsset is Asset needing updating
         *
         */
        bool UpdateAssetPathInfo(Asset* iAsset);
        
        /**
         *
         * Reads all of the Reel objects from the CPL XML data and adds them to the CPL C++ Object.
         *
         * @param pParent is the TiXmlNode node to search for the Reels
         * @param iCPL is CPL C++ Object to add any found Reel objects to
         *
         */
        void MapReels(TiXmlNode* pParent, CPL *iCPL);

        /**
         *
         * Reads all of the Asset objects from the CPL XML data and adds them to the Reel C++ Object.
         *
         * @param pParent is the TiXmlNode node to search for the Asset
         * @param iReel is Reel C++ Object to add any found Asset objects to
         *
         */
        void MapAssets(TiXmlNode* pParent, Reel *iReel);

        /// The path to the CPL XML file to be parsed
        std::string  pathToCurrentCPL_;
        
        // Vector of all asset information found. Contains information from the ASSETMAP.xml file
        std::vector<AssetFileInfo> assetMap_;
    };

}  // namespace SMPTE_SYNC

#endif // CPLPARSER_H
