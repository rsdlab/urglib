#include <stdlib.h>
#include <iostream>
#include "Transport.h"
#include "Timer.h"
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

static bool cmdMSMD_errmsg(const int stat);

Transport::Transport(net::ysuga::SerialPort* pSerialPort, UrgBase* pUrg) :
  m_pSerialPort(pSerialPort), m_pUrg(pUrg)
{
}

Transport::~Transport()
{

}


int Transport::waitCommand(char* command, int timeoutMilliSec) {
  int retval;
 transport_receive_start:
  while(true) {
    readBlock(command+0, 1, timeoutMilliSec);
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


bool Transport::receive(const char* expectedCommand, int timeoutMilliSec)
{
  //  std::cout << "receiving data from urg" << std::endl;
  char command[3];
  int index = waitCommand(command);
  command[2] = 0;
  //  std::cout << "Command:" << command << " Received." << std::endl;
  if (expectedCommand != NULL) {
    //    std::cout << "Expect: " << expectedCommand << std::endl;
    while (expectedCommand[0] != command[0] || expectedCommand[1] != command[1]) {
      index = waitCommand(command, timeoutMilliSec);
    }
  }

  //std::cout << "Received Command: " << command << std::endl;
  m_Packet.cmd[0] = command[0];
  m_Packet.cmd[1] = command[1];
  switch(index) {
  case CMD_VV:
    return onCmdVV(timeoutMilliSec);
  case CMD_PP:
    return onCmdPP(timeoutMilliSec);
  case CMD_II:
    return onCmdII(timeoutMilliSec);
  case CMD_MS:
    return onCmdMS(timeoutMilliSec);
  case CMD_MD:
    return onCmdMD(timeoutMilliSec);
  case CMD_RS:
    return onCmdRS(timeoutMilliSec);
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

bool Transport::readBlock(char* buffer, uint32_t size, int timeoutMilliSec) {
	ssr::Timer timer;
	timer.tick();
	ssr::TimeSpec currentTime;
  while(m_pSerialPort->GetSizeInRxBuffer() < size) {
    net::ysuga::Thread::Sleep(1);
	timer.tack(&currentTime);
	if(currentTime.getUsec() > timeoutMilliSec*1000) {
		throw ssr::TimeOutException();
	}
  }
  m_pSerialPort->Read(buffer, size);
  return true;
}

bool Transport::readLine(char* buffer, int timeoutMilliSec, int maxChar) {
  int i = 0;
  while(1) {
	  if (i == maxChar-1) {
		  buffer[i] = 0;
		  return false;
	  }

    readBlock(buffer+i, 1, timeoutMilliSec);
    if(buffer[i] == 0x0A) {
      buffer[i] = 0;
      break;
    }
    i++;
  }
  return true;
}

bool Transport::readStringLine(char* buffer, int timeoutMilliSec, int maxChar) {
  bool ret = readLine(buffer, timeoutMilliSec, maxChar);
  if (strlen(buffer) >= 2) {
	  buffer[strlen(buffer) - 2] = 0;
  }
  return ret;
}

uint32_t Transport::readIntLine(int timeoutMilliSec) {
  char buffer[128];
  readStringLine(buffer, timeoutMilliSec);
  for(int i= 0;i < strlen(buffer);i++) {
    if(buffer[i] == ':') {
      return atoi(buffer+i+1);
    }
  }
  return atoi(buffer);
}

bool Transport::onCmdVV(int timeoutMilliSec) {
  //  std::cout << "onCmdVV" << std::endl;
  char buffer[128];
  readLine(buffer, timeoutMilliSec);
  readLine(buffer, timeoutMilliSec);
  readStringLine(m_pUrg->m_VendorInfo, timeoutMilliSec);
  readStringLine(m_pUrg->m_ProductInfo, timeoutMilliSec);
  readStringLine(m_pUrg->m_FirmwareVersion, timeoutMilliSec);
  readStringLine(m_pUrg->m_ProtocolVersion, timeoutMilliSec);
  readStringLine(m_pUrg->m_SerialNumber, timeoutMilliSec);
  readLine(buffer, timeoutMilliSec);
  
  return true;
}


bool Transport::startSCIP20(int timeoutMilliSec) {
  static const char key[8] = "SCIP2.0";
  m_pSerialPort->Write(key, 7);
  char lf = 0x0a;
  m_pSerialPort->Write(&lf, 1);
  char buffer[128];
  readLine(buffer, timeoutMilliSec);
  readLine(buffer, timeoutMilliSec);
  if(buffer[0] == '0' && buffer[1] == 0) {
    readLine(buffer, timeoutMilliSec);
    return true;
  } else if (buffer[0] == '0' && buffer[1] == '1') {
    return false;
  }
  std::cout << "[UrgBase] StartSCIP20 : unknown return value:" << buffer << std::endl;
  return false;
}

bool Transport::onCmdPP(int timeoutMilliSec) {
  char buffer[128];
  readLine(buffer, timeoutMilliSec);
  readLine(buffer, timeoutMilliSec);

  readStringLine(m_pUrg->m_ModelInfo);
  m_pUrg->m_MinMeasure = readIntLine(timeoutMilliSec);
  m_pUrg->m_MaxMeasure = readIntLine(timeoutMilliSec);
  m_pUrg->m_AngleDiv   = readIntLine(timeoutMilliSec);
  m_pUrg->m_AngleStartStep = readIntLine(timeoutMilliSec);
  m_pUrg->m_AngleEndStep   = readIntLine(timeoutMilliSec);
  m_pUrg->m_AngleFrontStep = readIntLine(timeoutMilliSec);
  m_pUrg->m_ScanRPM    = readIntLine(timeoutMilliSec);
  readStringLine(m_pUrg->m_RotateDirection, timeoutMilliSec);
  return true;
}


bool Transport::onCmdII(int timeoutMilliSec)
{
  char buffer[128];
  readLine(buffer, timeoutMilliSec);
  readLine(buffer, timeoutMilliSec);

  while (1) {
	  char stringBuffer[255];
	  readStringLine(stringBuffer, timeoutMilliSec);

	  if (strlen(stringBuffer) == 0) {
		  break;
	  }

	  if (strncmp(stringBuffer, "STAT", 4) == 0) {
		  strcpy(m_pUrg->m_SensorStatus, stringBuffer);
		  if (strncmp(m_pUrg->m_SensorStatus + 5, "Trouble", 7) == 0) {
			  m_pUrg->m_Trouble = true;
		  }
		  else {
			  m_pUrg->m_Trouble = false;
		  }
	  }
	  else if (strncmp(stringBuffer, "TIME", 4) == 0) {
		  strcpy(m_pUrg->m_SensorClock, stringBuffer);
	  }
	  else if (strncmp(stringBuffer, "MODL", 4) == 0) {
		  strcpy(m_pUrg->m_SensorModel, stringBuffer);
	  }
	  else if (strncmp(stringBuffer, "LASR", 4) == 0) {
		  if (strcmp(stringBuffer+5, "OFF") == 0) {
			  m_pUrg->m_LaserOn = false;
		  }
		  else {
			  m_pUrg->m_LaserOn = true;
		  }
	  }
	  else if (strncmp(stringBuffer, "SCSP", 4) == 0) {
		  strcpy(m_pUrg->m_ScanSpeed, stringBuffer);
	  }
	  else if (strncmp(stringBuffer, "MESM", 4) == 0) {
		  strcpy(m_pUrg->m_ScanMode, stringBuffer);
	  }
	  else if (strncmp(stringBuffer, "SBPS", 4) == 0) {
		  strcpy(m_pUrg->m_SerialCommunicationSpeed, stringBuffer);
	  }
  }

  return true;
}


uint32_t Transport::decode6BitCharactor(char* buffer, uint32_t size)
{
  uint32_t retval = 0;
  for(uint32_t i = 0;i < size;i++) {
    retval |= (uint32_t)(((uint8_t)buffer[i]) - 0x30) << (6*(size-i-1));
  }
  return retval;
}

int32_t Transport::decodeCharactor(char* buffer, uint32_t size)
{
  char str[16];
  for(uint32_t i = 0;i < size;i++) {
    str[i] = buffer[i];
  }
  str[size] = 0;
  return atoi(str);
}



bool Transport::onCmdMD(int timeoutMilliSec)
{
  //  std::cout << "onCmdMD" << std::endl;
  char firstLine[128];
  char buffer[128];
  readLine(firstLine, timeoutMilliSec);
  readLine(buffer, timeoutMilliSec);
  buffer[2] = 0;
  int stat = atoi(buffer);
  //  std::cout << "Status : " << stat << std::endl;
  if(stat == 99) {
    net::ysuga::MutexBinder b(m_pUrg->m_Mutex);
    m_pUrg->m_pData->clear();
    int32_t startStep = decodeCharactor(firstLine, 4);
    int32_t endStep = decodeCharactor(firstLine+4, 4);
    int32_t clustorCount = decodeCharactor(firstLine+8, 2);
    m_pUrg->m_pData->minAngle = -((int32_t)m_pUrg->m_AngleFrontStep - startStep) * m_pUrg->m_pData->angularRes;
    m_pUrg->m_pData->maxAngle = (endStep - m_pUrg->m_AngleFrontStep) * m_pUrg->m_pData->angularRes;
    
    readLine(buffer); // Time Stamp
    m_pUrg->m_pData->timestamp = decode6BitCharactor(buffer, 4);
	//std::cout << " - TS: " << m_pUrg->m_pData->timestamp << " data received." << std::endl;

	char tempStrBuffer[3];
	int fullfilledBytes = 0;
    while(1) {
      readLine(buffer, timeoutMilliSec);
      int len = strlen(buffer); // Have SUM data
	  //std::cout << " - Length: " << len << " data received." << std::endl;
      if(len == 0) {
	break;
	  }

	  len--; // Have SUM byte.

	  for (int i = 0; i < len; i++) {
		  tempStrBuffer[fullfilledBytes] = buffer[i];
		  fullfilledBytes++;
		  if (fullfilledBytes == 3) {
			  m_pUrg->m_pData->push(decode6BitCharactor(tempStrBuffer, 3));
			  fullfilledBytes = 0;
		  }
	  }

	  /*
      for(int i = 0;i+3 < len;i += 3) {
		  m_pUrg->m_pData->push(decode6BitCharactor(buffer + i, 3));
	  }*/
    }
  } else {
    return cmdMSMD_errmsg(stat);
  }
  return true;
}

bool Transport::onCmdMS(int timeoutMilliSec)
{
  char firstLine[128];
  char buffer[128];
  readLine(firstLine, timeoutMilliSec);
  readLine(buffer, timeoutMilliSec);
  buffer[2] = 0;
  int stat = atoi(buffer);
  if (stat == 99) {
    net::ysuga::MutexBinder b(m_pUrg->m_Mutex);
    m_pUrg->m_pData->clear();
    int32_t startStep = decodeCharactor(firstLine, 4);
    int32_t endStep   = decodeCharactor(firstLine+4, 4);
    int32_t clustorCount = decodeCharactor(firstLine+8, 2);
    m_pUrg->m_pData->minAngle = -((int32_t)m_pUrg->m_AngleFrontStep - startStep) * m_pUrg->m_pData->angularRes;
    m_pUrg->m_pData->maxAngle = (endStep - m_pUrg->m_AngleFrontStep) * m_pUrg->m_pData->angularRes;
    readLine(buffer, timeoutMilliSec); // Time Stamp
    m_pUrg->m_pData->timestamp = decode6BitCharactor(buffer, 4);
    while(1) {
      readLine(buffer, timeoutMilliSec);
      int len = strlen(buffer);
      if(len == 0) {
	break;
      }
      for(int i = 0;i+2 < len;i += 2) {
	m_pUrg->m_pData->push(decode6BitCharactor(buffer+i, 2));
      }
    }
  } else {
    return cmdMSMD_errmsg(stat);
  }
  
  return true;
}

bool Transport::onCmdRS(int timeoutMilliSec)
{
  char firstLine[128];
  char buffer[128];
  readLine(firstLine, timeoutMilliSec);
  readLine(buffer, timeoutMilliSec);
  if(buffer[0] == '0' && buffer[1] == '0' && buffer[2] == 'P') {
	  return true;
  } else {
	  return false;
  }
}




static void msg(const char* msg) {
  std::cout << "[UrgBase] MS/MD Message: " << msg << std::endl;
}

static void msg(const char* msg, const int stat) {
  std::cout << "[UrgBase] MS/MD Message: " << msg << "(" << stat << ")" << std::endl;
}

static bool cmdMSMD_errmsg(const int stat) {


  switch(stat) {
  case 0:
    msg("Properly received.");
    return true;
    break;
  case 1:
    msg("Start Step includes non-numeric charactor.");
    break;
  case 2:
    msg("Stop Step includes non-numeric charactor.");
    break;
  case 3:
    msg("Clustor Count includes non-numeric charactor.");
    break;
  case 4:
    msg("Stop Step exeeds maximum step");
    break;
  case 5:
    msg("Stop Step is less than Start Step.");
    break;
  case 6:
    msg("Interval Count includes non-numeric charactor.");
    break;
  case 7:
    msg("Scan Count includes non-numeric charactor.");
    break;
  default:
    if (stat >= 21 && stat < 50) {
      msg("Software Error with ", stat);
    } else if (stat >= 50 && stat < 97) {
      msg("Hardware Error with ", stat);
    } else if (stat == 98) {
      msg("Resumed");
      return true;
    } else {
      msg("Unknown Error with ", stat);
    }
  }
  return false;
}
