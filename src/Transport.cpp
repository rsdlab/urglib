#include <iostream>
#include "Transport.h"
#include "UrgBase.h"

#define NUM_CMD 17
const char* cmd[NUM_CMD] = {
  "VV", "PP", "II", "BM", "QT",
  "MD", "MS", "GD", "GS", "SS", 
  "CR", "TM", "RS", "RT", "ME",
  "HS", "DB"
};

enum CMD {
  CMD_VV, CMD_PP, CMD_II, CMD_BM, CMD_QT,
  CMD_MD, CMD_MS, CMD_GD, CMD_GS, CMD_SS,
  CMD_CR, CMD_TM, CMD_RS, CMD_TR, CMD_ME,
  CMD_HS, CMD_DB
};

using namespace ssr;

Transport::Transport(net::ysuga::SerialPort* pSerialPort, UrgBase* pUrg) :
  m_pSerialPort(pSerialPort), m_pUrg(pUrg)
{
}

Transport::~Transport()
{

}


int Transport::waitCommand(char* command) {
  int retval;
 transport_receive_start:
  while(true) {
    readBlock(command+0, 1);
    for( int i = 0; i < NUM_CMD;i++) {
      if(command[0] == cmd[i][0]) {
	goto transport_receive_next;
      }
    }
  }
 transport_receive_next:
  readBlock(command+1, 1);
  for (int i = 0;i < NUM_CMD;i++) {
    if(command[0] == cmd[i][0] && command[1] == cmd[i][1]) {
      return i;
    } 
  }
  goto transport_receive_start;
}


bool Transport::receive(const char* expectedCommand)
{
  char command[3];
  int index = waitCommand(command);
  command[2] = 0;
  if (expectedCommand != NULL) {
    while (expectedCommand[0] != command[0] || expectedCommand[1] != command[1]) {
      index = waitCommand(command);
    }
  }

  //  std::cout << "Received Command: " << command << std::endl;
  m_Packet.cmd[0] = command[0];
  m_Packet.cmd[1] = command[1];
  switch(index) {
  case CMD_VV:
    return onCmdVV();
  case CMD_PP:
    return onCmdPP();
  case CMD_II:
    return onCmdII();
  case CMD_MS:
    return onCmdMS();
  case CMD_MD:
    return onCmdMD();
  default:
    break;

  }
  return false;
}

bool Transport::transmit(const Packet& packet)
{
  char command[3];
  command[0] = packet.cmd[0];
  command[1] = packet.cmd[1];
  command[2] = 0;
  m_pSerialPort->Write(&(packet.cmd[0]), 1);
  //  net::ysuga::Thread::Sleep(20);
  m_pSerialPort->Write(&(packet.cmd[1]), 1);
  //  net::ysuga::Thread::Sleep(20);
  int len = packet.length();
  for(int i = 0;i < len;i++) {
    m_pSerialPort->Write(&(packet[i]), 1);
    //    net::ysuga::Thread::Sleep(20);
  }
  static const char lf = 0x0A;
  m_pSerialPort->Write(&lf, 1);
  return true;
}

bool Transport::readBlock(char* buffer, uint32_t size) {
  while(m_pSerialPort->GetSizeInRxBuffer() < 1) {
    net::ysuga::Thread::Sleep(10);
  }
  m_pSerialPort->Read(buffer, 1);
  return true;
}

bool Transport::readLine(char* buffer) {
  int i = 0;
  while(1) {
    readBlock(buffer+i, 1);
    if(buffer[i] == 0x0A) {
      buffer[i] = 0;
      break;
    }
    i++;
  }

  return true;
}

bool Transport::readStringLine(char* buffer) {
  bool ret = readLine(buffer);
  buffer[strlen(buffer)-2] = 0;
  return ret;
}

uint32_t Transport::readIntLine() {
  char buffer[128];
  readStringLine(buffer);
  for(int i= 0;i < strlen(buffer);i++) {
    if(buffer[i] == ':') {
      return atoi(buffer+i+1);
    }
  }
  return atoi(buffer);
}

bool Transport::onCmdVV() {
  //  std::cout << "onCmdVV" << std::endl;
  char buffer[128];
  readLine(buffer);
  readLine(buffer);
  readStringLine(m_pUrg->m_VendorInfo);
  readStringLine(m_pUrg->m_ProductInfo);
  readStringLine(m_pUrg->m_FirmwareVersion);
  readStringLine(m_pUrg->m_ProtocolVersion);
  readStringLine(m_pUrg->m_SerialNumber);
  readLine(buffer);
  
  return true;
}


bool Transport::startSCIP20() {
  static const char key[8] = "SCIP2.0";
  m_pSerialPort->Write(key, 7);
  char lf = 0x0a;
  m_pSerialPort->Write(&lf, 1);
  char buffer[128];
  readLine(buffer);
  readLine(buffer);
  if(buffer[0] == '0' && buffer[1] == 0) {
    readLine(buffer);
    return true;
  } else if (buffer[0] == '0' && buffer[1] == '1') {
    return false;
  }
  std::cout << "startSCIP20 : unknown return value:" << buffer << std::endl;
  return false;
}

bool Transport::onCmdPP() {
  char buffer[128];
  readLine(buffer);
  readLine(buffer);

  readStringLine(m_pUrg->m_ModelInfo);
  m_pUrg->m_MaxMeasure = readIntLine();
  m_pUrg->m_MinMeasure = readIntLine();
  m_pUrg->m_AngleDiv   = readIntLine();
  m_pUrg->m_AngleStartStep = readIntLine();
  m_pUrg->m_AngleEndStep   = readIntLine();
  m_pUrg->m_AngleFrontStep = readIntLine();
  m_pUrg->m_ScanRPM    = readIntLine();
  readStringLine(m_pUrg->m_RotateDirection);
  return true;
}


bool Transport::onCmdII()
{
  char buffer[128];
  readLine(buffer);
  readLine(buffer);

  readStringLine(buffer);
  if(strcmp(buffer, "OFF") == 0) {
    m_pUrg->m_LaserOn = false;
  } else {
    m_pUrg->m_LaserOn = true;
  }
  readStringLine(m_pUrg->m_ScanSpeed);
  readStringLine(m_pUrg->m_ScanMode);
  readStringLine(m_pUrg->m_SerialCommunicationSpeed);
  readStringLine(m_pUrg->m_SensorClock);
  readStringLine(m_pUrg->m_SensorStatus);
  
  return true;
}


uint32_t Transport::decodeCharactor(char* buffer, uint32_t size)
{
  uint32_t retval = 0;
  for(uint32_t i = 0;i < size;i++) {
    retval |= (uint32_t)(((uint8_t)buffer[i]) - 0x30) << (6*(size-i-1));
  }
  return retval;
}


bool Transport::onCmdMD()
{
  //  std::cout << "onCmdMD" << std::endl;
  char buffer[128];
  readLine(buffer);
  readLine(buffer);
  buffer[2] = 0;
  int stat = atoi(buffer);
  //  std::cout << "Status : " << stat << std::endl;
  if(stat == 99) {
    readBlock(buffer, 5);
    m_pUrg->m_pData->timestamp = decodeCharactor(buffer, 4);
    while(1) {
      readLine(buffer);
      int len = strlen(buffer);
      if (len == 0) {
	break;
      }
    }
  }

  return true;
}

bool Transport::onCmdMS()
{
  std::cout << "onCmdMS" << std::endl;
  char buffer[128];
  readLine(buffer);
  readLine(buffer);
  buffer[2] = 0;
  int stat = atoi(buffer);
  //std::cout << "Status : " << stat << std::endl;
  if (stat == 99) {
    m_pUrg->m_pData->clear();
    readLine(buffer); // Time Stamp
    m_pUrg->m_pData->timestamp = decodeCharactor(buffer, 4);
    while(1) {
      readLine(buffer);
      int len = strlen(buffer);
      std::cout << " - Length: " << len << " data received." << std::endl;
      if(len == 0) {
	break;
      }
      for(int i = 0;i < len;i += 2) {
	m_pUrg->m_pData->push(decodeCharactor(buffer+i, 2));
      }
    }
    std::cout << " - " << m_pUrg->m_pData->length << " data received." << std::endl;
  }


  return true;
}
