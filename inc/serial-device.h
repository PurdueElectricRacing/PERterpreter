#ifndef __SERIAL_STUFF_H__
#define __SERIAL_STUFF_H__


#include "timer.h"

#include <string>
#include <QSerialPort>
#include <QSerialPortInfo>



#include <cstring>
#include <thread>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <iostream>
#include <exception>
#include <QDebug>
#include <QRegExp>




/// @brief: gets the list of connected serial devices and returns a pretty 
///         string for printing to console
static std::string getSerialPortsString(QList<QSerialPortInfo> ports)
{
  QString entries = "   Location  | Port Name | Description | Manufacturer \n"
                    "=======================================================\n";
  int i = 0;

  for (auto port = ports.begin(); port != ports.end(); ++port, i++)
  {
    QSerialPortInfo pinfo = *port;
    QString portname = pinfo.portName();
    QString desc = pinfo.description();
    QString man = pinfo.manufacturer();
    QString loc = pinfo.systemLocation();

    // all this just to get the index...
    entries += QString((std::to_string(i).c_str())) + ": " + loc + " | " 
            + portname + " | " + desc + " | " + man + "\n";
  }

  return entries.toStdString();
}



class SerialDevice
{
public:
  SerialDevice() {};

  SerialDevice(QSerialPortInfo port)
  {
    device.setPort(port);
    
    if (!port.isNull())
    {
      open();
    }
  };

  virtual ~SerialDevice() {};



  void open() 
  {
    device.setBaudRate(115200);

    if (!device.open(QIODevice::ReadWrite))
    {
      std::cerr << "Error opening " << device.portName().toStdString() << ": " 
                << "\n";
    }
    // device.setParity(QSerialPort::NoParity);
    // device.setStopBits(QSerialPort::OneStop);
    // device.setFlowControl(QSerialPort::NoFlowControl);
    // device.setDataBits(QSerialPort::Data8);
  };

  void close() { device.close(); };




  /// @brief: wrapper for setting the device based on string input
  /// @return: true on success, false on error
  bool setSerialDevice(std::string dev)
  {
    return setSerialDevice(QSerialPortInfo(dev.c_str()));
  }



  /// @brief: sets the current device's port info to port
  ///         prints any error messages to stderr
  /// @return: True on success, false on error
  bool setSerialDevice(QSerialPortInfo port, bool default_open = true)
  {
    // find the serial device
    if (port.isNull())
    {
      std::cerr << port.portName().toStdString() << " is not a valid port.\n";
      return false;
    }
    
    QSerialPort temp(port);
    // check if the device is already open.
    // This can be changed later to allow multiple pointers to the same dev
    // if need be
    if (temp.isOpen())
    {
      std::cerr << port.portName().toStdString() << " is already open.\n";
      return false;
    }

    // close the device if it's already open
    if (device.isOpen())
    {
      std::cerr << device.portName().toStdString() << " is open. Closing.\n";
      device.close();
    }

    device.setPort(port);
    
    // by default open the device
    if (default_open)
    {
      open();
      qDebug() << "Delaying for 1000ms because the Arduino is a butthole";
      Timer t(3000);
      t.start();
      while(!t.expired())
      {

      }
    }

      
    return device.isOpen();
  };




  /// @brief: A function for prompting the user to select a serial port from ///
  ///         those connected. 
  /// @return: the selected serial port info object
  QSerialPortInfo selectSerialPort()
  {
    auto ports = QSerialPortInfo::availablePorts();
    std::string ports_string = getSerialPortsString(ports);
    int port_idx = -1;
    
    if (ports.size() == 0)
    {
      std::cerr << "selectSerialPort: No serial devices detected. \n";
      return QSerialPortInfo();
    }

    std::cout << ports_string;
    std::string port_select;

    // just continue prompting until correct input is received
    while (port_idx == -1 || port_idx >= ports.size())
    {
      std::cin >> port_select;
      std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

      try
      { 
        port_idx = std::stoi(port_select);
      } 
      catch(const std::exception& e)
      {
        std::cerr << port_select << " is not a valid integer.\n";
        port_idx = -1;
        continue;
      }

      if (port_idx < ports.size() && port_idx > -1)
      {
        setSerialDevice(ports[port_idx]);
        qDebug() << "Read channels: " << device.readChannelCount() 
                 << "| Write Channels:" << device.writeChannelCount();
        return ports[port_idx];
      }
      else
      {
        std::cout << port_idx << " is not a valid port number.\n";
      }
    }
    return QSerialPortInfo();
  }



  /// @brief: Send an arbitrary command to an arbitrary serial device
  ///
  /// @return: true on success, false on error
  bool sendCommand(QByteArray cmd)
  {
    // make sure the device is open
    if (device.isOpen())
    {
      // qDebug() << "SerialDevice::sendCommand: Sending command" << cmd;
      int written = device.write(cmd);
      if (written == -1)
      {
        std::cerr << "SerialDevice::sendCommand: error sending to device.\n";
      }

      device.waitForBytesWritten();
      return true;
    }  
    else
    {
      std::cerr << " Unable to send command to " 
                << device.portName().toStdString() << ": the device is not open.\n";
      return false;
    }
  }


  /// @brief: read whatever is in the buffer and return it as a qstring
  QString serialRead()
  {
    if (device.waitForReadyRead())
    {
      QString data = device.readAll();
      // qDebug() << "SerialDevice::serialRead: Received" << data << "from" 
              //  << device.portName();
      return data;
    }
    std::cerr << device.portName().toStdString() << ": " 
              <<  device.errorString().toStdString() << "\n";
    device.clearError();
    return QString("-1");
  };

  bool isOpen() { return device.isOpen(); };

protected:
  QSerialPort device;
};




#endif