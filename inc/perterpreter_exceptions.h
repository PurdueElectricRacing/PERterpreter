#ifndef __PERTERPRETER_EXCEPTIONS_H__
#define __PERTERPRETER_EXCEPTIONS_H__

#include <exception>

class SerialDeviceNotConnected : public std::exception
{
  virtual const char *what() const throw()
  {
    return "No generic serial device is connected and serialRx or serialTx was"
           " called.\n";
  }
} static SerialDeviceNotConnected;


class TestTimeoutException : public std::exception
{
  virtual const char *what() const throw()
  {
    return "The current test exceeded the maximum execution duration.\n";
  }
} static TestTimeout;



class NoGPIODevicesFound : public std::exception
{
  virtual const char *what() const throw()
  {
    return "No GPIO interface devices were detected.\n";
  }
} static NoGPIODevicesFound;


class GPIODeviceNotConnected : public std::exception
{
  virtual const char *what() const throw()
  {
    return "No gpio device connected and read-pin or write-pin was called.\n";
  }
} static GPIODeviceNotConnected;


class CANDeviceNotConnected : public std::exception
{
  virtual const char *what() const throw()
  {
    return "No CAN interface connected and send-msg or read-msg was called.\n";
  }
} static CANDeviceNotConnected;



#endif