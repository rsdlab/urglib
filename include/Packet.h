#pragma once

#include <string>
#include <string.h>
#include <stdint.h>



namespace ssr {
  class Translator;

  /**
   * Check Sum Error
   */
  class CheckSumError : public std::exception {
  private:
    std::string msg;
    
  public:
    CheckSumError() {
      this->msg = "CheckSumError";
    }
    
    ~CheckSumError() throw() {
    }
    
  public:
    virtual const char* what() const throw() {
      return msg.c_str();
    }
  };
  
  
  /**
   * Communication Packet
   */
  class Packet {
  private:
    uint8_t *m_pBuffer;
    uint8_t m_BufferLength;

  public:
    char cmd[2];
    char str[16];
    uint8_t *getBuffer() const {return m_pBuffer;}
    uint8_t length() const {return m_BufferLength;}

  public:
  Packet(const char* command = NULL, const uint8_t length = 0) : m_pBuffer(NULL) {
      std::cout << "Command:" << std::endl;
      if(command == NULL) {
	cmd[0] = cmd[1] = 0;
      } else {
	cmd[0] = command[0];
	cmd[1] = command[1];
      }
      if (length > 0) {
	m_pBuffer = new uint8_t[length];
      }
      m_BufferLength = length;
    }

    Packet(const Packet& packet){
      this->cmd[0] = packet.cmd[0];
      this->cmd[1] = packet.cmd[1];
      m_pBuffer = new uint8_t[packet.length()];
      memcpy(m_pBuffer, packet.getBuffer(), packet.length());
      m_BufferLength = packet.length();
    }

    void operator=(const Packet& packet){
      this->cmd[0] = packet.cmd[0];
      this->cmd[1] = packet.cmd[1];
      m_pBuffer = new uint8_t[packet.length()];
      memcpy(m_pBuffer, packet.getBuffer(), packet.length());
      m_BufferLength = packet.length();
    }

    virtual ~Packet(){
      delete m_pBuffer;
    }

  public:
    friend class Translator;

    uint8_t& operator[](const int index) const {
      return (m_pBuffer[index]);
    }

    uint8_t  uchar(const int offset) const {
      return (m_pBuffer[offset]);
    }

    uint16_t ushort(const int offset) const {
      return m_pBuffer[offset] | (m_pBuffer[offset+1] << 8);
    }
     
    void ushort(const int offset, const uint16_t value) {
      m_pBuffer[offset  ] = value & 0xff;
      m_pBuffer[offset+1] = (value >> 8) & 0xff;
    }
  };

}
