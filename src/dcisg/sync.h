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

#ifndef SYNC_H
#define SYNC_H

#include <stdint.h>
#include "UUID.h"

namespace SMPTE_SYNC
{
    /// Section 5.3.1 of [ADSSTP]

    #define SYNCMARKER  0xAAF0
    #define SYNCMARKER_TWOS_COMPLEMENT  0x5510

    #define BASE_PAYLOAD_LENGTH 42
    
    /// Section 5.3.2 of [ADSSTP]

    #define NSTATES 3
    #define STOPPED 0
    #define PAUSED  1
    #define PLAYING 2

    /// Section 5.3.1 of [ADSSTP]

    /**
     * @brief syncPacket class C++ data structure and serialization for a syncPacket as defined in Section 5.3.1 of
     * SMPTE ST 430-10:2010 D-Cinema Operations — Auxiliary Content Synchronization Protocol
     *
     */
    typedef class syncPacket
    {
    public:
        
        /// Constructor
        syncPacket()
        {
            marker_ = SYNCMARKER;
            length_ = BASE_PAYLOAD_LENGTH;
            timelineEditUnitIndex_ = 0;
            playoutID_ = 0;
            editUnitDuration_ = 0;
            sampleDurationEnum_ = 0;
            sampleDurationDenom_ = 0;
            primaryPictureOutputOffset_ = 0;
            primaryPictureScreenOffset_ = 0;
            primaryPictureTrackFileEditUnitIndex_ = 0;
            primarySoundTrackFileEditUnitIndex_ = 0;
            extensionLength_ = 0;
            extension_ = nullptr;
            
            for (int i = 0; i < 16; i++)
            {
                primaryPictureTrackFileUUID_[i] = 0;
                primarySoundTrackFileUUID_[i] = 0;
                compositionPlaylistUUID_[i] = 0;
            }
        }
        
        /// Destructor
        ~syncPacket()
        {
            delete [] extension_;
        }
        
        /// Initializes or resets the values owned by syncPacket
        void Reset(void)
        {
            marker_ = SYNCMARKER;
            length_ = BASE_PAYLOAD_LENGTH;
            timelineEditUnitIndex_ = 0;
            playoutID_ = 0;
            editUnitDuration_ = 0;
            sampleDurationEnum_ = 0;
            sampleDurationDenom_ = 0;
            primaryPictureOutputOffset_ = 0;
            primaryPictureScreenOffset_ = 0;
            primaryPictureTrackFileEditUnitIndex_ = 0;
            primarySoundTrackFileEditUnitIndex_ = 0;
            extensionLength_ = 0;
            if (extension_ != nullptr)
            {
                delete extension_;
                extension_ = nullptr;
            }
            
            for (int i = 0; i < 16; i++)
            {
                primaryPictureTrackFileUUID_[i] = 0;
                primarySoundTrackFileUUID_[i] = 0;
                compositionPlaylistUUID_[i] = 0;
            }
        }
        
        /// TODO: Do we realy need count as this is a preallocated buffer based on the number of samples per frame
        /**
         * Serializes the syncPacket into a buffer
         *
         * @param buffer is the buffer to write the syncPacket into
         * @param count is the number of bytes written into the buffer
         *
         */
        bool WriteSyncPacket(uint8_t *buffer, uint32_t *count);

        /// Sets the packet length
        void SetLength(uint16_t length);

        /// Sets the status to STOPPED, PAUSED, or PLAYING
        void SetStatus(uint8_t status);
        
        /// Sets the timelineEditUnitIndex_
        void SetTimelineEditUnitIndex(uint32_t timelineEditUnitIndex);
        
        /// Sets the playoutID_
        void SetPlayoutID(uint32_t playoutID);
        
        /// Sets the editUnitDuration_
        void SetEditUnitDuration(uint16_t iDuration);
        
        /**
         * Sets the sample duration
         *
         * @param enumerator is enumerator of a rational sample rate
         * @param denominator is denominator of a rational sample rate
         *
         */
        void SetSampleDuration(uint32_t enumerator, uint32_t denominator);
        
        /// Sets the primaryPictureOutputOffset_
        void SetPrimaryPictureOutputOffset(int32_t iOffset);
        
        /// Sets the primaryPictureScreenOffset_
        void SetPrimaryPictureScreenOffset(uint32_t iOffset);
        
        /// Sets the primaryPictureTrackFileEditUnitIndex_
        void SetPrimaryPictureTrackFileEditUnitIndex(uint32_t iIndex);
        
        /**
         * Sets the primaryPictureTrackFileUUID_
         *
         * @param uuid is the UUID to set
         * @return true/false if the UUID is valid and could be set
         *
         */
        bool SetPrimaryPictureTrackFileUUID(UUID uuid);

        /**
         * Sets the primaryPictureTrackFileUUID_
         *
         * @param uuid is a char buffer containing a string that represents a UUID
         * @return true/false if the UUID is valid and could be set
         *
         */
        bool SetPrimaryPictureTrackFileUUID(char *uuid);
        
        /// Sets the primarySoundTrackFileEditUnitIndex_
        void SetPrimarySoundTrackFileEditUnitIndex(uint32_t iIndex);
        
        /**
         * Sets the primarySoundTrackFileUUID_
         *
         * @param uuid is the UUID to set
         * @return true/false if the UUID is valid and could be set
         *
         */
        bool SetPrimarySoundTrackFileUUID(UUID uuid);

        /**
         * Sets the primarySoundTrackFileUUID_
         *
         * @param uuid is a char buffer containing a string that represents a UUID
         * @return true/false if the UUID is valid and could be set
         *
         */
        bool SetPrimarySoundTrackFileUUID(char *uuid);
        
        /**
         * Sets the compositionPlaylistUUID_
         *
         * @param uuid is the UUID to set
         * @return true/false if the UUID is valid and could be set
         *
         */
        bool SetCompositionPlaylistUUID(UUID uuid);
        
        /**
         * Sets the compositionPlaylistUUID_
         *
         * @param uuid is a char buffer containing a string that represents a UUID
         * @return true/false if the UUID is valid and could be set
         *
         */
        bool SetCompositionPlaylistUUID(char *uuid);
        
        /**
         * Sets the extention which is currently reserved for future updates to the specification
         * Allocates memory to store the extension
         *
         * @param extension is a buffer of uint16_t containing the extension data
         * @param extensionLength is length of the extension data
         * @return true/false if the extension was properly set
         *
         */
        bool SetExtension(uint16_t *extension, uint16_t extensionLength);

        uint16_t    marker_;
        uint16_t    length_;
        uint16_t    flags_;
        uint32_t    timelineEditUnitIndex_;
        uint32_t    playoutID_;
        uint16_t    editUnitDuration_;
        uint32_t    sampleDurationEnum_;
        uint32_t    sampleDurationDenom_;
        int32_t     primaryPictureOutputOffset_;
        uint32_t    primaryPictureScreenOffset_;
        uint32_t    primaryPictureTrackFileEditUnitIndex_;
        UUID        primaryPictureTrackFileUUID_;
        uint32_t    primarySoundTrackFileEditUnitIndex_;
        UUID        primarySoundTrackFileUUID_;
        UUID        compositionPlaylistUUID_;
        uint16_t    extensionLength_;
        uint16_t*   extension_;
        
    } syncPacket;

    /**
     * Writes a uint16_t value to the buffer i.e. the syncPacket data stream
     *
     * @param value is the uint16_t data to be written
     * @param buffer is buffer to write the data to
     * @param count is number of bytes written
     * @param first is set if this is the first value in the syncPacket to be written, if so a extra bit is set signifying it is the first piece of data in the syncPacket
     * @return true/false if the uint16_t data was properly written
     *
     */
    bool WriteUInt16(uint16_t value, uint8_t *buffer, uint8_t *count, bool first);

    /**
     * Writes a uint32_t value to the buffer i.e. the syncPacket data stream
     *
     * @param value is the uint32_t data to be written
     * @param buffer is buffer to write the data to
     * @param count is number of bytes written
     * @param first is set if this is the first value in the syncPacket to be written, if so a extra bit is set signifying it is the first piece of data in the syncPacket
     * @return true/false if the uint32_t data was properly written
     *
     */
    bool WriteUInt32(uint32_t value, uint8_t *buffer, uint8_t *count, bool first);

    /**
     * Writes a int32_t value to the buffer i.e. the syncPacket data stream
     *
     * @param value is the int32_t data to be written
     * @param buffer is buffer to write the data to
     * @param count is number of bytes written
     * @param first is set if this is the first value in the syncPacket to be written, if so a extra bit is set signifying it is the first piece of data in the syncPacket
     * @return true/false if the int32_t data was properly written
     *
     */
    bool WriteInt32(int32_t value, uint8_t *buffer, uint8_t *count, bool first);

    /**
     * Writes a UUID value to the buffer i.e. the syncPacket data stream
     *
     * @param value is the UUID data to be written
     * @param buffer is buffer to write the data to
     * @param count is number of bytes written
     * @param first is set if this is the first value in the syncPacket to be written, if so a extra bit is set signifying it is the first piece of data in the syncPacket
     * @return true/false if the UUID data was properly written
     *
     */
    bool WriteUUID(UUID uuid, uint8_t *buffer, uint8_t *count, bool first);

    /**
     * Reads a uint16_t value from the buffer i.e. the syncPacket data stream
     * Advances the iBuf by the
     *
     * @param iBuf is the uint16_t data to be written
     * @param oVal is buffer to write the data to
     * @return true/false if the uint16_t data was properly written
     *
     */
    void ReadUInt16(uint8_t **iBuf, uint16_t &oVal);

    /**
     * Reads a uint32_t value from the buffer i.e. the syncPacket data stream
     *
     * @param iBuf is the uint32_t data to be written
     * @param oVal is buffer to write the data to
     * @return true/false if the uint32_t data was properly written
     *
     */
    void ReadUInt32(uint8_t **iBuf, uint32_t &oVal);

    /**
     * Reads a uint32_t value from the buffer i.e. the syncPacket data stream
     *
     * @param iBuf is the uint32_t data to be written
     * @param oVal is buffer to write the data to
     * @return true/false if the uint32_t data was properly written
     *
     */
    void ReadInt32(uint8_t **iBuf, int32_t &oVal);

    /**
     * Reads a UUID value from the buffer i.e. the syncPacket data stream
     *
     * @param iBuf is the UUID data to be written
     * @param oVal is buffer to write the data to
     * @return true/false if the UUID data was properly written
     *
     */
    void ReadUUID(uint8_t **iBuf, UUID &oVal);

}  // namespace SMPTE_SYNC

#endif // SYNC_H
