#ifndef __PERTERPRETER_H__
#define __PERTERPRETER_H__

#include "object.h"
#include "strobj.h"
#include "integer.h"
#include "canmsg.h"
#include "symbol-table.h"
#include "gpio-interface.h"
#include "can_api.h"
#include "ghc/filesystem.hpp"
#include "timer.h"

#include <iostream>
#include <fstream>
#include <QSerialPort>
#include <map>
#include <thread>

#define CAN_READ_TIMER_TIMEOUT 1500 // msecs to wait for a specific message to arrive

class Perterpreter
{
public:
  Node * _root = 0;
  Perterpreter(GpioDevice * gpio = 0, SerialDevice * misc = 0) {
    routines = new Routines();
    tests = new Tests();
    global_table = new SymbolTable();
    global_table->initGlobals();
    
    if (!gpio)
    {
      gpio_created = true;
      gpio_device = new GpioDevice();
    }
    else
    {
      gpio_device = gpio;
    }

    if (!misc)
    {
      ser_created = true;
      serial_device = new SerialDevice();
    }
    else 
    {
      serial_device = misc;
    }
    can_if = 0;
  }

  virtual ~Perterpreter() {
    delete global_table;
    delete routines;
    delete tests;
    if (!_root)
    {
      delete _root;
    }
    if (gpio_created)
    {
      delete gpio_device;
    }
    if (ser_created)
    {
      delete serial_device;
    }
    if (can_created)
    {
      delete can_if;
    }

    if (test_output.is_open())
    {
      test_output.close();
    }
  }


  /// open a file for logging test output to
  void setTestOutputFile(std::string path) {
    test_output.open(path, std::ofstream::out);
  };

  bool setGpioDev(std::string dev) { return gpio_device->setSerialDevice(dev);};
  bool setSerialDev(std::string dev) {return serial_device->setSerialDevice(dev);};
  void setCanInterface(CanInterface * itf) { can_if = itf; };
  void selectGpioDev();
  bool performSyntaxAnalysis(ghc::filesystem::path filepath);

  void setVerbose(int verb) {verbose = verb;};
  void createTemplateScript(std::string spath);

  void perterpret(std::string func="");



private:
  void perterpretVardecl(Node * node, SymbolTable * scope);
  void perterpretCall(Node * node);
  void perterpretDelay(Node * node, SymbolTable * scope);
  void perterpretLoop(Node * node, SymbolTable * scope);
  void perterpretExpectAssert(Node * node, SymbolTable * scope);
  void perterpretPrint(Node * node, SymbolTable * scope);
  void perterpretSerialTx(Node * node, SymbolTable * scope);
  void perterpretSerialRx(SymbolTable * scope);
  void perterpretPrompt(Node * node, SymbolTable * scope);
  void perterpretDRead(Node * node, SymbolTable * scope);
  void perterpretDWrite(Node * node, SymbolTable * scope);
  void perterpretAread(Node * node, SymbolTable * scope);
  void perterpretAwrite(Node * node, SymbolTable * scope);
  void perterpretSendMsg(Node * node, SymbolTable * scope);
  void perterpretReadMsg(Node * node, SymbolTable * scope);
  void perterpretNode(Node * node, SymbolTable * scope);
  void perterpretIf(Node * node, SymbolTable * scope);
  void runTest(Test * test);
  void perterpretSetTimeout(Node * node, SymbolTable * scope);
  
  Object * perterpretExp(Node * node, SymbolTable * scope);
  Object * perterpretBinaryOp(Node * node, SymbolTable * scope);
  Object * getObject(Node * node, SymbolTable * scope);


  Routines *routines = 0;
  Tests *tests = 0;
  SymbolTable * global_table = 0;
   
  SerialDevice *serial_device;
  GpioDevice *gpio_device;
  CanInterface *can_if;
  Object * retval = 0;
  bool verbose = false;
  bool gpio_created = false;
  bool ser_created = false;
  bool can_created = false;
  std::thread timeout_thread;
  Timer execution_begin;

  std::ofstream test_output;

};



#endif