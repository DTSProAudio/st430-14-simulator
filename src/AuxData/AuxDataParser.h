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

#ifndef AUXDATAPARSER_H
#define AUXDATAPARSER_H

#include <assert.h>
#include <stdint.h>
#include <cstdio>
#include <cstdlib>
#include <string>

/// TODO: Further abstract ASDCP from this layer by creating an abstract interface and force the client to provide the implementation.
#define USE_ASDCP

#ifdef USE_ASDCP
#include <AS_DCP.h>
#endif

namespace SMPTE_SYNC
{

    /**
     * @brief AuxDataParser class implements a wrapper for the ASDCP MXF file reading.
     *
     */

    class AuxDataParser
    {
    public:

        /**
         *
         * Constructor 
         *
         * @param iStartFrame is starting frame to read from the MXF
         * @param iEndFrame is ending frame to read from the MXF
         * @param iAuxDataFilePath is the path to the MXF aux data file to be read
         *
         */
        AuxDataParser(int32_t iStartFrame
                      , int32_t iEndFrame
                      , const std::string &iAuxDataFilePath);
        
        /// Destructor
        virtual ~AuxDataParser();

        /**
         *
         * Opens the file at the iAuxDataFilePath (path_)
         *
         * @return bool if the file opened properly
         *
         */
        bool Open(void);

        /**
         *
         * Closes the file at the iAuxDataFilePath (path_)
         *
         * @return bool if the file closed properly
         *
         */
        bool Close(void);

        /**
         *
         * Returns the startFrame_ that is currently set on the object.
         *
         * @return int32_t representing the start frame
         *
         */
        int32_t GetStartFrame(void);

        /**
         *
         * Returns the endFrame_ that is currently set on the object.
         *
         * @return int32_t representing the end frame
         *
         */
        int32_t GetEndFrame(void);
        
        /**
         *
         * Attempts to copy the data requested data item number into a buffer.
         * Allocates memory for the oDataItem. Client must deallocate any memory allocated.
         *
         * @param iItemNumber is the requested item number
         * @param oDataItem is a pointer to a buffer with upon return will contain the data item information
         * @param oDataItemSize is the size of the data that was allocated
         * 
         * @return bool represents the success of reading the data
         *
         */
        bool GetDataItem(int32_t iItemNumber
                         , uint8_t **oDataItem
                         , uint32_t &oDataItemSize);
        
    private:

        /// The path to the MXF file being parsed
        std::string     path_;
        
        /// The start frame being requested
        int32_t         startFrame_;

        /// The end frame being requested
        int32_t         endFrame_;
        
#ifdef USE_ASDCP
        /// AS-DCP file reader
        Kumu::FileReader f;

        /// AS-DCP MXF reader
        ASDCP::DCData::MXFReader r;
#endif
    };

}  // namespace SMPTE_SYNC

#endif // AUXDATAPARSER_H
