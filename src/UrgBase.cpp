/**
 * Author: Yuki Suga (ysuga@ysuga.net)
 * copyright: yuki Suga
 */

#include <stdio.h> // for sprintf
#include <iostream>

#ifdef WIN32
#define _USE_MATH_DEFINES
#endif

#include <math.h>
#include "UrgBase.h"

using namespace ssr;
using namespace net::ysuga;

UrgBase::UrgBase(const char* filename, int baudrate, int defaultTimeoutMilliSec):
  m_Endflag(false), m_Interval(1), m_pData(NULL), m_DefaultTimeoutTime(defaultTimeoutMilliSec)
{
  std::cout << "[UrgBase] Initializaing URG(port=" << filename << ")" << std::endl;
  try {
	  std::cout << "[UrgBase] Opening COM port (port=" << filename << ", baudrate=" << baudrate << ")"<< std::endl;
	  m_pSerialPort = new SerialPort(filename, baudrate);
  } catch (net::ysuga::ComException& ex) {
	  std::cout << "[UrgBase] ComException: " << ex.what() << std::endl;
	  throw ex;
  }

  m_pTransport = new Transport(m_pSerialPort, this);
  
  try {
	  std::cout << "[UrgBase] Starting SCPI2.0 Protocol." << std::endl;
	  if(m_pTransport->startSCIP20(m_DefaultTimeoutTime)) {
		std::cout << "[UrgBase] Changing to SCIP2.0 ... successfull." << std::endl;
	  } else {
		std::cout << "[UrgBase] Changing to SCIP2.0 ... Already SCIP2.0." << std::endl;
	  }
  } catch (const ssr::TimeOutException & ex) {
	  std::cout << "[UrgBase] SCIP Protocol Version Search Timeout.\n";
	  throw ex;
  }
  reset();
  if(!updateInfo()) {
    std::cerr << "[UrgBase] UpdateInfo failed." << std::endl;
  }

  std::cout << "[UrgBase] Vendor       :" << m_VendorInfo << std::endl;
  std::cout << "[UrgBase] Product      :" << m_ProductInfo << std::endl;
  std::cout << "[UrgBase] Firmware Ver.:" << m_FirmwareVersion << std::endl;
  std::cout << "[UrgBase] Product Ver. :" << m_ProtocolVersion << std::endl;
  std::cout << "[UrgBase] Serial No.   :" << m_SerialNumber << std::endl;
  std::cout << "[UrgBase] Model Info   :" << m_ModelInfo << std::endl;
  std::cout << "[UrgBase] Max Measure  :" << m_MaxMeasure << " [mm]" << std::endl;
  std::cout << "[UrgBase] Min Measure  :" << m_MinMeasure << " [mm]" << std::endl;
  std::cout << "[UrgBase] Scan Starts  :" << m_AngleStartStep << " [step]" << std::endl;
  std::cout << "[UrgBase] Scan Ends    :" << m_AngleEndStep << " [step]" << std::endl;
  std::cout << "[UrgBase] Angle Div    :" << m_AngleDiv << " [step]" << std::endl;
  std::cout << "[UrgBase]  ==> Resolution (360.0/AngleDiv): " << 360.0/m_AngleDiv << " [deg]" << std::endl;
  std::cout << "[UrgBase] Scan RPM     :" << m_ScanRPM << " [rpm]" << std::endl;
  std::cout << "[UrgBase]  ==> Frequency:   " << m_ScanRPM/60 << " [Hz]" << std::endl;

  m_pData = new RangeData(m_AngleEndStep - m_AngleStartStep+1);
  m_pData->angularRes = 2 * M_PI / m_AngleDiv;
  m_pData->minAngle = -(m_AngleFrontStep - m_AngleStartStep) * m_pData->angularRes;
  m_pData->maxAngle = (m_AngleEndStep - m_AngleFrontStep) * m_pData->angularRes;
  m_pData->minRange = m_MinMeasure;
  m_pData->maxRange = m_MaxMeasure;

  std::cout << "[UrgBase] Initializationg Success.\n";
}

UrgBase::~UrgBase()
{
	if(this->m_Endflag) {
		std::cout << "[UrgBase] Stopping Urg Background Job" << std::endl;
		Stop();
		std::cout << "[UrgBase] Stopped." << std::endl;
	}
  //  delete m_pTranslator;
  delete m_pTransport;
  std::cout << "[UrgBase] Closing Serial Port." << std::endl;
  delete m_pSerialPort;
  std::cout << "[UrgBase] Closed." << std::endl;
  delete m_pData;
}

bool UrgBase::updateInfo()
{
  Packet p1("VV");
  m_pTransport->transmit(p1);
  m_pTransport->receive("VV", m_DefaultTimeoutTime);
  Packet p2("II");
  m_pTransport->transmit(p2);
  if(!m_pTransport->receive("II", m_DefaultTimeoutTime)) {
    return false;
  }
  Packet p3("PP");
  m_pTransport->transmit(p3);
  m_pTransport->receive("PP", m_DefaultTimeoutTime);

  return true;
}

void UrgBase::Run()
{
  m_Endflag = false;
  std::cout << "[UrgBase] UrgBase::Starting Background Job." << std::endl;
  while(!m_Endflag) {
    try {
      m_pTransport->receive(m_DefaultTimeoutTime);
      onUpdate();

      onPreSendCommand();
      onPostSendCommand();

      //Thread::Sleep(m_Interval); // [ms]
    } catch (TimeOutException& e) {
      std::cerr << "[UrgBase] Packet Receiver -- Timeout" << std::endl;
    } catch (CheckSumError& e) {
    }
  }
  std::cout << "[UrgBase] UrgBase::Exitting Background Job." << std::endl;
}


bool UrgBase::turnOn()
{
  Packet p("BM");
  m_pTransport->transmit(p);
  return m_pTransport->receive(m_DefaultTimeoutTime);
}

bool UrgBase::turnOff()
{
  Packet p("QT");
  m_pTransport->transmit(p);
  return m_pTransport->receive(m_DefaultTimeoutTime);
}

bool UrgBase::reset()
{
  std::cout << "[UrgBase] Reseting URG" << std::endl;
  Packet p("RS");
  m_pTransport->transmit(p);
  if(m_pTransport->receive("RS", m_DefaultTimeoutTime)) {
	  std::cout << "[UrgBase] Reset Succeeded." << std::endl;
	  return true;
  } else {
	  std::cout << "[UrgBase] Reset Failed." << std::endl;
	  return false;
  }
}


bool UrgBase::startMeasure(uint32_t startStep, 
			   uint32_t stopStep, 
			   uint32_t clustorCount,
			   uint32_t intervalCount, 
			   bool extended,
			   uint32_t scanCount)
{
  char cmd[3] = {'M', 'S', 0};
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
  // std::cout << " startmeasure - " << buffer << std::endl;
  m_pTransport->receive(m_DefaultTimeoutTime);
  Start();
  return true;
}
