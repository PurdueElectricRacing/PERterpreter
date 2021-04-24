#ifndef __GPIO_IF_H__
#define __GPIO_IF_H__

#include "serial-device.h"

#include <thread>
#include <iostream>
#include <string>
#include <QSerialPort>
#include <QSerialPortInfo>


enum GpioCommands
{
  READ_ADC   = 0, 
  READ_GPIO  = 1, 
  WRITE_DAC  = 2, 
  WRITE_GPIO = 3,
};

struct Command
{
  Command(uint8_t command=0, uint8_t pin=0, uint16_t value=0)
  {
    data[0] = (command << 4) & 0xF0;
    data[0] |= (pin & 0x0F);
    data[1] = value >> 8;
    data[2] = value & 0x00FF;
  };

  void reinit(char data[])
  {
    this->data[0] = data[0];
    this->data[1] = data[1];
    this->data[2] = data[2];
  };

  uint8_t command() { return (data[0] & 0xF0) >> 4; };
  uint8_t pin() { return data[0] & 0x0F; };
  uint16_t value() { 
    uint16_t ret = (uint16_t) data[1] << 8;
    uint8_t rhs = data[2];
    ret |= rhs;
    return ret; 
  };
  int size() { return 3 * sizeof(char); };
  char data[3];
};



class GpioDevice : public SerialDevice
{
public:
  GpioDevice() {
    
  };

  GpioDevice(QSerialPortInfo) {
    
  };

  virtual ~GpioDevice() {

  };



  /// @brief: writes pin to value (state if it's a digital write)
  void digitalWrite(int pin, int value)
  {
    Command c(WRITE_GPIO, pin, value);
    sendCommand(QByteArray(c.data, 3));
  };


  /// @brief: reads the digital input in and returns the int value
  int digitalRead(int pin)
  {
    int ret = -1;
    Command c(READ_GPIO, pin);

    // make sure the message was sent before trying to read
    if (sendCommand(QByteArray(c.data, 3)))
    {
      bool ok = false;
      auto data = serialRead();
      QString s = data;
      ret = s.toInt(&ok);

      if (!ok)
      {
        std::cerr << "Invalid data was received: " << data.toStdString() << "\n";
      }
    }

    return ret;
  };



  /// @brief: write an analog value to the specified pin
  void analogWrite(int pin, int value)
  {
    Command c(WRITE_DAC, pin, value);
    sendCommand(QByteArray(c.data, 3));
  };



  /// @brief: read the value on the ADC pin and return as an int
  ///
  /// @return: the value read for the pin or -1 if error
  int analogRead(int pin)
  {
    int ret = -1;

    Command c(READ_ADC, pin);

    // make sure the message was sent before trying to read
    if (sendCommand(QByteArray(c.data, 3)))
    {
      bool ok = false;
      auto data = serialRead();
      ret = data.toInt(&ok);

      if (!ok)
      {
        std::cerr << "Invalid data was received: " << data.toStdString() << "\n";
      }
    }

    return ret;
  };
  

};


#endif
