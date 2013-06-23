#pragma once
#include <stdint.h>
#include "liburg.h"
#include "Thread.h"
//#include "Translator.h"
#include "SerialPort.h"
#include "Transport.h"

#define BAUDRATE 115200

namespace ssr {
  //class Translator;

  class LIBURG_API UrgBase : public net::ysuga::Thread {
  private:
    Transport  *m_pTransport;
    net::ysuga::SerialPort *m_pSerialPort;
    bool m_Endflag;
    uint8_t m_Interval;

    char m_VendorInfo[128];
    char m_ProductInfo[128];
    char m_FirmwareVersion[128];
    char m_ProtocolVersion[128];
    char m_SerialNumber[128];
    char m_ModelInfo[128];
    uint32_t m_MaxMeasure;
    uint32_t m_MinMeasure;
    uint32_t m_AngleDiv;
    uint32_t   m_AngleStartStep;
    uint32_t   m_AngleEndStep;
    uint32_t   m_AngleFrontStep;
    uint32_t   m_ScanRPM;
    char m_RotateDirection[32];

    bool m_LaserOn;
    char m_ScanSpeed[128];
    char m_ScanMode[128];
    char m_SerialCommunicationSpeed[128];
    char m_SensorClock[128];
    char m_SensorStatus[128];

  protected:
  public:
    UrgBase(const char* filename, int baudrate = BAUDRATE);

    virtual ~UrgBase();


  public:
    bool turnOn();
    bool turnOff();
    bool reset();

    bool startMeasure(uint32_t startStep = 0, uint32_t stopStep = 65535, uint32_t clustorCount = 1, uint32_t intervalCount = 0, bool extended=false, uint32_t scanCount = 0);

  public:
    virtual void Run();

  private:
    //    friend class Translator;
    friend class Transport;
    virtual void onUpdate() {}
    virtual void onPreSendCommand() {};
    virtual void onPostSendCommand() {};

    void updateInfo();
  };

}
