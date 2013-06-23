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

  class UrgBase;
  class Transport {

  private:
    Packet m_Packet;
  private:
    net::ysuga::SerialPort* m_pSerialPort;
    UrgBase* m_pUrg;    
  public:
    Packet& getPacket() {return m_Packet;}
  public:
    Transport(net::ysuga::SerialPort* pSerialPort, UrgBase* pUrg);
    virtual ~Transport();
    
    int waitCommand(char* command);
    bool readLine(char* buffer);
    bool readStringLine(char* buffer);
    uint32_t readIntLine();

    bool receive();
    bool transmit(const Packet& packet);

    bool startSCIP20();

    
  private:

    bool onCmdMD();
    bool onCmdMS();
    bool onCmdVV();
    bool onCmdPP();
    bool onCmdII();
  };
  
}
