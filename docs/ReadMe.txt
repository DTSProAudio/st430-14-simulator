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

// "ReadMe.txt" file for the SMPTE Sync library

/**
 * @file Readme.txt
 * @date August 31, 2015
 */

// ================
// [Begin]


//*****
Summary

This file contains brief information on the SMPTE Sync library.

//**********
What is the SMPTE Sync Library?

SMPTE Sync is a code framework and implementation of the necessary components to
implement the SMPTE ST 430-14 D-Cinema Operations – Digital Sync Signal 
and Aux Data Transfer Protocol.

This library can be used as a reference implementation or test framework to validate 
other implementations of the SMPTE ST 430-14 specification.

//************************************************
What is contained in the package?

(...)/
			|
			->build    // (Mac OS X, Linux, and Windows project files)
			|
			->doc      // (this ReadMe.txt file)
			|
			->doxy     // (Doxygen file to generate API documentation)
			|
			->src      // (SMPTE Sync source files)
			|
			->tests    // (SMPTE Sync unit tests)
			|
	    ->lib/     // (external dependencies)
						|
						->asdcplib		// (MXF Parser)
						|
						->tinyxml			// (XML Parser)

Several source files under src/server/SS are copied from the Boost Software
Library and are licensed under the license at src/server/SS/LICENSE_1_0.txt.

//********************
What else is required?

SMPTE Sync depends on external third party libraries. The libraries that are
used are ASDCPLib, Boost, TinyXML and Google Test.

ASCDPLib and TinyXML are managed as submodules. They are downloaded when
cloning the repo using `git clone --recurse-submodules` or after cloning using
`git submodule init; git submodule update`.

Google Test is automatically installed by the build script.

Boost needs to be available.

//********************
Build system

CMake is used to build the library.

mkdir build
cd build
cmake ..
make
ctest

//*******************
Windows-specific instructions:

The Boost_INCLUDE_DIR needs to be set to the location where the boost library was installed.

//**********
Linux-specific instructions

The Boost library can be installed as follows:

sudo apt-get install libboost-all-dev

// ================
// [End]

