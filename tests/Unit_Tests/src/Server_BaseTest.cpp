#if 0
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

//
//  Server_BaseTest.cpp
//
//

#include "Server_BaseTest.h"
#include "gtest/gtest.h"

#include "Commands.h"
#include "CommandDataTypes.h"
#include "Utils.h"
#include "CommandFactory.h"

using namespace SMPTE_SYNC;
using namespace std;

void Command_Connect_Callback(Command *iCommand)
{
    if (iCommand == nullptr)
        return;
    
    CommandClass cmdClass = iCommand->GetCommandClass();
    CommandType cmdType = iCommand->GetCommandType();
    CommandID commandID = Utils::CommandIDFromClassAndType(cmdClass, cmdType);
    
    cout << "Command_Connect_Callback commandID = 0x" << std::hex << std::setw(8) << std::setfill('0') << commandID << "\n";
}

TEST(Command_BaseTest, Command_BaseTest_Case1)
{
    Command *commandPtr = new Command();
    
    int64_t test1 = commandPtr->GetFromDevice();
    commandPtr->SetFromDevice(2);
    test1 = commandPtr->GetFromDevice();

    delete commandPtr;
}

TEST(Command_BaseTest, Command_BaseTest_Case2)
{
    CommandFactory *cmdFactory = new CommandFactory();
    cmdFactory->Initialize(nullptr);

    CommandID commandID = Utils::CommandIDFromClassAndType(eCommandClass_Connection, eConnection_Connect);
    
    cmdFactory->RegisterCommandCallback(commandID, Command_Connect_Callback);
    
    Command *testCmd = cmdFactory->CreateCommand(eCommandClass_Connection, eConnection_Connect);
    
    cmdFactory->ExecuteCommand(testCmd);
    cmdFactory->RecycleCommand(testCmd);
    
    delete cmdFactory;
}

TEST(Command_BaseTest, Command_BaseTest_Case3)
{
    CommandFactory *cmdFactory = new CommandFactory();
    cmdFactory->Initialize(nullptr);
    
    Command *testCmd = cmdFactory->CreateCommand(eCommandClass_Connection, eConnection_ConnectTo);

    ConnectTo *payloadPtr = dynamic_cast<ConnectTo *>(testCmd->GetPayload());
    payloadPtr->set_device(0xFFFFFFFF);
    
    int32_t headerSize = 34;
    int32_t maxPayloadSize = 2048;
    
    uint8_t *commandWriteBuffer = new uint8_t[headerSize + maxPayloadSize];
    memset(commandWriteBuffer, 0x0, headerSize + maxPayloadSize);

    uint8_t *writeHeaderPtr = commandWriteBuffer;
    uint8_t *writePayloadPtr = commandWriteBuffer + headerSize;
    
    testCmd->WriteHeader(&writeHeaderPtr);
    testCmd->WritePayload(&writePayloadPtr);
    
    Command *testReadCmd = cmdFactory->CreateCommand(eCommandClass_Connection, eConnection_ConnectTo);

    // Setup pointers to read the buffer
    //
    uint8_t *commandReadBufferPtr = commandWriteBuffer;
    
    testReadCmd->ReadHeader(&commandReadBufferPtr);
    testReadCmd->ReadPayload(&commandReadBufferPtr);

    ConnectTo *readPayLoadPtr = dynamic_cast<ConnectTo *>(testReadCmd->GetPayload());

    if (readPayLoadPtr->device())
    {
        int64_t device = readPayLoadPtr->device();
        device++;
    }
    
    delete cmdFactory;
    delete testReadCmd;
    delete testCmd;
    delete [] commandWriteBuffer;
}
#endif