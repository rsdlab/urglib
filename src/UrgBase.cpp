#include <iostream>

#include "UrgBase.h"

using namespace ssr;
using namespace net::ysuga;

UrgBase::UrgBase(const char* filename, int baudrate):
  m_Endflag(false), m_Interval(1)
{
  std::cout << "Initializaing URG(" << filename << ")" << std::endl;
  m_pSerialPort = new SerialPort(filename, baudrate);
  m_pTransport = new Transport(m_pSerialPort, this);
  
  if(m_pTransport->startSCIP20()) {
    std::cout << "Changing to SCIP2.0 ... successfull." << std::endl;
  } else {
    std::cout << "Changing to SCIP2.0 ... Already SCIP2.0." << std::endl;
  }
  updateInfo();

  std::cout << "Vendor       :" << m_VendorInfo << std::endl;
  std::cout << "Product      :" << m_ProductInfo << std::endl;
  std::cout << "Firmware Ver.:" << m_FirmwareVersion << std::endl;
  std::cout << "Product Ver. :" << m_ProtocolVersion << std::endl;
  std::cout << "Serial No.   :" << m_SerialNumber << std::endl;
  std::cout << "Model Info   :" << m_ModelInfo << std::endl;
  std::cout << "Max Measure  :" << m_MaxMeasure << " [mm]" << std::endl;
  std::cout << "Min Measure  :" << m_MinMeasure << " [mm]" << std::endl;
  std::cout << "Scan Starts  :" << m_AngleStartStep << " [step]" << std::endl;
  std::cout << "Scan Ends    :" << m_AngleEndStep << " [step]" << std::endl;
  std::cout << "Angle Div    :" << m_AngleDiv << " [step]" << std::endl;
  std::cout << " ==> Resolution (360.0/AngleDiv): " << 360.0/m_AngleDiv << " [deg]" << std::endl;
  std::cout << "Scan RPM     :" << m_ScanRPM << " [rpm]" << std::endl;
  std::cout << " ==> Frequency:   " << m_ScanRPM/60 << " [Hz]" << std::endl;
}

UrgBase::~UrgBase()
{
  m_Endflag = true;
  Join();
  //  delete m_pTranslator;
  delete m_pTransport;
  delete m_pSerialPort;
}

void UrgBase::updateInfo()
{
  Packet p1("VV");
  m_pTransport->transmit(p1);
  m_pTransport->receive();
  Packet p2("II");
  m_pTransport->transmit(p2);
  m_pTransport->receive();
  Packet p3("PP");
  m_pTransport->transmit(p3);
  m_pTransport->receive();
}

void UrgBase::Run()
{
  m_Endflag = false;
  std::cout << "UrgBase::Starting Background Job." << std::endl;
  while(!m_Endflag) {
    try {
      m_pTransport->receive();
      //      m_pTranslator->translate(m_pTransport->getPacket());
      onUpdate();

      onPreSendCommand();
      onPostSendCommand();

      Thread::Sleep(m_Interval); // [ms]
    } catch (TimeOutException& e) {
      std::cerr << "Packet Receiver -- Timeout" << std::endl;
    } catch (CheckSumError& e) {
    }
  }
}


bool UrgBase::turnOn()
{
  Packet p("BM");
  m_pTransport->transmit(p);
  return m_pTransport->receive();
}

bool UrgBase::turnOff()
{
  Packet p("QT");
  m_pTransport->transmit(p);
  return m_pTransport->receive();
}

bool UrgBase::reset()
{
  Packet p("RS");
  m_pTransport->transmit(p);
  return m_pTransport->receive();
}


bool UrgBase::startMeasure(uint32_t startStep, 
			   uint32_t stopStep, 
			   uint32_t clustorCount,
			   uint32_t intervalCount, 
			   bool extended,
			   uint32_t scanCount)
{
  char cmd[2] = {'M', 'S'};
  if(extended) {
    cmd[1] = 'D';
  }
  if(startStep < m_AngleStartStep) {
    startStep = m_AngleStartStep;
  }

  if(stopStep  > m_AngleEndStep) {
    stopStep = m_AngleEndStep;
  }
    
  char buffer[256];
  sprintf(buffer, "%s%04d%04d%02d%01d%02d", cmd, startStep, stopStep,
	  clustorCount, intervalCount, scanCount);
  buffer[2+4+4+2+1+2] = 0x0A;

  m_pSerialPort->Write(buffer, 2+4+4+2+1+2+1);
  
  m_pTransport->receive();
  Start();
}
