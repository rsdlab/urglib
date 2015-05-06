#pragma once
#include <exception>
#include "SerialPort.h"
#include "Packet.h"

namespace ssr {

  class TimeOutException : std::exception {
  private:
    std::string msg;
    
  public:
    TimeOutException(const char* msg) {
      this->msg = "TimeOutException: ";
      this->msg += msg;
    }

    TimeOutException() {
      this->msg = "TimeOutException";
    }
    
    ~TimeOutException() throw() {
    }
    
  public:
    virtual const char* what() const throw() {
      return msg.c_str();
    }
  };

#define INFINITY_TIME 0xFFFF

  class UrgBase;
  class Transport {

  private:
    Packet m_Packet;
    net::ysuga::SerialPort* m_pSerialPort;
    UrgBase* m_pUrg;    

  public:
    Transport(net::ysuga::SerialPort* pSerialPort, UrgBase* pUrg);
    virtual ~Transport();
    
    int waitCommand(char* command, int timeoutMilliSe=INFINITY_TIME);

    bool receive(const char* expectedCmd = NULL, int timeoutMilliSe=INFINITY_TIME);
	
    bool receive(int timeoutMilliSec=INFINITY_TIME) {
		return receive(NULL, timeoutMilliSec);
	}

    bool transmit(const Packet& packet);

    bool startSCIP20(int timeoutMilliSec=INFINITY_TIME);

  private:
    bool readBlock(char *buffer, uint32_t size, int timeoutMilliSec=INFINITY_TIME);
    uint32_t readIntLine( int timeoutMilliSec=INFINITY_TIME);

    int32_t decodeCharactor(char* buffer, uint32_t size);
    uint32_t decode6BitCharactor(char* buffer, uint32_t size);
	bool readLine(char* buffer, int timeoutMilliSec=INFINITY_TIME);
    bool readStringLine(char* buffer, int timeoutMilliSec=INFINITY_TIME);

	Packet& getPacket() {return m_Packet;}

    bool onCmdMD(int timeoutMilliSec=INFINITY_TIME);
    bool onCmdMS(int timeoutMilliSec=INFINITY_TIME);
    bool onCmdVV(int timeoutMilliSec=INFINITY_TIME);
    bool onCmdPP(int timeoutMilliSec=INFINITY_TIME);
    bool onCmdII(int timeoutMilliSec=INFINITY_TIME);
    bool onCmdRS(int timeoutMilliSec=INFINITY_TIME);
  };
  
}
