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

	  class BufferOverrunException : std::exception {
  private:
    std::string msg;
    
  public:
    BufferOverrunException(const char* msg) {
      this->msg = "BufferOverrun: ";
      this->msg += msg;
    }

    BufferOverrunException() {
      this->msg = "BufferOverrun";
    }
    
    ~BufferOverrunException() throw() {
    }
    
  public:
    virtual const char* what() const throw() {
      return msg.c_str();
    }
  };

  class RangeData {
  public:
    struct {
      double r;
      double p;
      double y;
    } orientation;

    struct {
      double x;
      double y;
      double z;
    } pose;

  public:
    uint32_t timestamp;
    uint32_t *range;
    uint32_t length;
	uint32_t maxLength;
  public:
    double minAngle;
    double maxAngle;
    double angularRes;
    double minRange;
    double maxRange;
    
  public:
    RangeData(uint32_t size) :
    timestamp(0) , length(0) {
      range = new uint32_t[size];
	  maxLength = size;
      pose.x = pose.y = pose.z = 0;
      orientation.r = orientation.p = orientation.y = 0;
    }

    RangeData(const RangeData& rangeData) {
      this->timestamp = rangeData.timestamp;
      this->length = rangeData.length;
	  this->maxLength = rangeData.maxLength;
      range = new uint32_t[maxLength];
      for(int i=  0;i < length;i++) {
	range[i] = rangeData.range[i];
      }
      this->pose = rangeData.pose;
      this->orientation = rangeData.orientation;

      this->minAngle = rangeData.minAngle;
      this->maxAngle = rangeData.maxAngle;
      this->angularRes = rangeData.angularRes;
      this->minRange = rangeData.minRange;
      this->maxRange = rangeData.maxRange;
    }

    void operator=(const RangeData& rangeData) {
      this->timestamp = rangeData.timestamp;
      this->length = rangeData.length;
	  this->maxLength = rangeData.maxLength;
	  range = new uint32_t[rangeData.maxLength];
      for(int i=  0;i < length;i++) {
	range[i] = rangeData.range[i];
      }
      this->pose = rangeData.pose;
      this->orientation = rangeData.orientation;

      this->minAngle = rangeData.minAngle;
      this->maxAngle = rangeData.maxAngle;
      this->angularRes = rangeData.angularRes;
      this->minRange = rangeData.minRange;
      this->maxRange = rangeData.maxRange;
    }

    virtual ~RangeData() {
      delete range;
    }

    void clear() {
      length = 0;
    }

    void push(uint32_t range_data) {
		if(length >= maxLength) {
			throw ssr::BufferOverrunException();
		}
      range[length] = range_data;
      length++;
    }
  };

  class LIBURG_API UrgBase : public net::ysuga::Thread {
  private:
    Transport  *m_pTransport;
    net::ysuga::SerialPort *m_pSerialPort;
    volatile bool m_Endflag;
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
	int m_DefaultTimeoutTime;

    bool m_LaserOn;
	char m_SensorModel[128];
    char m_ScanSpeed[128];
    char m_ScanMode[128];
    char m_SerialCommunicationSpeed[128];
    char m_SensorClock[128];
    char m_SensorStatus[128];
	bool m_Trouble;

  protected:

  public:
	  bool isTrouble() {
		  return m_Trouble;
	  }

  public:
    void setOrientation(double r, double p, double y) {
      m_pData->orientation.r = r;
      m_pData->orientation.p = p;
      m_pData->orientation.y = y;
    }

    void setPose(double x, double y, double z) {
      m_pData->pose.x = x;
      m_pData->pose.y = y;
      m_pData->pose.z = z;
    }

    net::ysuga::Mutex m_Mutex;
    RangeData* m_pData;
  public:
    RangeData& getRangeData() {
      net::ysuga::MutexBinder b(m_Mutex);
      return *m_pData;
    }

    void LockData() {
      m_Mutex.Lock();
    }

    void UnlockData() {
      m_Mutex.Unlock();
    }

    UrgBase(const char* filename, int baudrate = BAUDRATE, int defaultTimeoutMilliSec = 1000);

    virtual ~UrgBase();


  public:
    bool turnOn();
    bool turnOff();
    bool reset();

    bool startMeasure(uint32_t startStep = 0, uint32_t stopStep = 65535, uint32_t clustorCount = 1, uint32_t intervalCount = 0, bool extended=false, uint32_t scanCount = 0);

  public:
    virtual void Run();
	void Stop() {
	  m_Endflag = true;
	  Join();
	}

  private:
    //    friend class Translator;
    friend class Transport;
    virtual void onUpdate() {}
    virtual void onPreSendCommand() {};
    virtual void onPostSendCommand() {};

    bool updateInfo();

  private:
  };

}
