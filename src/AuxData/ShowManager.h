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

#ifndef SHOWMANAGER_H
#define SHOWMANAGER_H

#include <assert.h>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <vector>

#include "boost/atomic.hpp"

#include "DataTypes.h"
#include "AuxData.h"

namespace SMPTE_SYNC
{
    class CPLParser;
    class AuxDataParser;
    class Show;
    class Asset;
    class FrameInfo;
    
    /**
     *
     * @brief ShowManager owns and manages the CPLParser, Show and all related objects.
     * The client interfaces to the Show through the Show Manager.
     * The ShowManager must be loaded before the SE_Server or SS_Server are started.
     * The SE_Server and SS_Server access this data from different threads and expect the ShowManager to be in a consistent state. 
     * The showLoaded_ flag represents the loaded state of the show.
     *
     */

    class ShowManager
    {
    public:

        /**
         *
         * Contructor
         *
         * @param iSampleRate is the base sample rate of the Show as specified by the audio subsystem used to playout the AES/EBU signal
         * @param iCPLList is the list of CPL XML files to be loaded by the show. The order of the items in the list is the order the CPL objects appear in the Show timeline
         *
         */
        ShowManager(int32_t iSampleRate, const CPLFileList &iCPLList);

        /**
         *
         * Contructor
         *
         * @param iSampleRate is the base sample rate of the Show as specified by the audio subsystem used to playout the AES/EBU signal
         *
         */
        ShowManager(int32_t iSampleRate);
        
        /// Destructor
        ~ShowManager();

        /**
         *
         * Adds a path to a CPL XML file to be added to the Show timeline. 
         * Each CPL that is added will be added to the Show timeline in the order of the AddCPL calls
         * Checks to see if the file is a CPL file before adding it ot the Show timeline.
         *
         * @param iCPLPath is path to the CPL XML file to be added to the show.
         * @return bool true/false if the iCPLPath was added to the parsing list
         *
         */
        bool AddCPL(const std::string &iCPLPath);

        /**
         *
         * Adds a CPLFileList to be added to the Show timeline.
         * Each CPL that is added will be added to the Show timeline in the order of the items in the CPLFileList or subsequent calls to CPLFileList or AddCPL
         * Checks to see if all of the file are CPL files before adding them to the Show timeline. If any file is not a CPL, no files are added to the Show timeline.
         *
         * @param iCPLList is a CPLFileList containing paths to the CPL XML file to be added to the show.
         * @return bool true/false if the iCPLList was added to the parsing list
         *
         */
        bool AddCPLList(const CPLFileList &iCPLList);

        /**
         *
         * Resets the ShowManager
         * Deletes the show_ and clears the CPLList_. Resets the showLoaded_ flag.
         * Each CPL that is added will be added to the Show timeline in the order of the AddCPL calls
         *
         * @param iCPLPath is path to the CPL XML file to be added to the show.
         * @return bool true/false if the iCPLPath was added to the parsing list
         *
         */
        bool Reset(void);

        /**
         *
         * Loads the list of CPL XML files and creates a Show object representing them
         * Deletes any existing CPLParser object and creates a new one
         * Deletes the show_ and clears the CPLList_. Resets the showLoaded_ flag.
         * Each CPL that is added will be added to the Show timeline in the order of the AddCPL calls
         *
         * @return bool true/false if the Show was succefully loaded and the Show has a length > 0
         *
         */
        bool Load(void);

        /**
         *
         * Checks the showLoaded_ flag
         *
         * @return bool true/false if the Show is currently loaded
         *
         */
        bool IsShowLoaded(void);

        /// TODO: Should we expose a Show pointer to the client or force them through the ShowManager?
        
        /**
         *
         * Computes the longest frame found in the show.
         * For example, in 24 vs. 25 frames 24 frames is the longest frame as it takes more time or wall clock time to present to the viewer
         *
         * @return int32_t of the longest frame
         *
         */
        int32_t GetLongestFrameLength(void);

        /**
         *
         * Returns the sample rate of the Show
         *
         * @return int32_t value of the sample rate of the show
         *
         */
        int32_t GetSampleRate(void);

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
        bool GetFrame(int32_t iFrame, FrameInfo& oFrameInfo);

        /**
         *
         * Populates a vector<char> for the requested data.
         * Potentially creates an AuxDataParser and parses data from a MXF file
         *
         * @param iCodingUL is requested coding UL for the data
         * @param iStart is requested start frame
         * @param iCount is requested number of frames
         * @param iAccept is requested accept type
         * @param oContent is vector<char> of data added to if the input values can be satisfied
         * @return bool true/false if the requested iFrame was found
         *
         */
        bool GetDataItems(const std::string &iCodingUL,
                          int32_t iStart,
                          int32_t iCount,
                          const std::string &iAccept,
                          std::vector<char>& oContent);
        
    private:

        /**
         *
         * Attempts to create a new AuxDataParser and calls Open on the AuxDataParser
         * Requires that there is no existing AuxDataParser. That is, auxDataParser_ is nullptr
         *
         * @param iStartFrame is requested start frame
         * @return bool true/false if the requested iFrame was found and the AuxDataParser was created and AuxDataParser::Open succeeded
         *
         */
        bool OpenAuxDataParser(int32_t iStartFrame);
        
        /// List of CPL XML files to parse and add to the Show timeline
        CPLFileList     CPLList_;
        
        /// Pointer to the current AuxDataParser
        AuxDataParser   *auxDataParser_;
        
        /// Pointer to the Show
        Show            *show_;

        /// Sample rate of the Show
        int32_t         sampleRate_;

        // Tracks if the Show is loaded or not
        boost::atomic<bool> showLoaded_;
    };

}  // namespace SMPTE_SYNC

#endif // SHOWMANAGER_H
