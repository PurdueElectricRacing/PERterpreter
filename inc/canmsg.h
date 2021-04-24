#ifndef __CAN_MSG_H__
#define __CAN_MSG_H__

#include "object.h"
#include <cstring>
#include <sstream>
#include <iomanip>
#include <exception>
#include <iostream>
#include "can_interface.h"
#include "byte_array.h"


class CAN_Msg : public ByteArray
{
public:
  CAN_Msg(std::string input = "") {
    parse(input);
    o_type = can_msg_obj;
  };

  CAN_Msg(CanFrame frame) {
    leng = frame.can_dlc;
    for (uint8_t i = 0; i < leng; i++)
    {
      data.push_back(frame.data[i]);
    }
  };

  virtual ~CAN_Msg(){};
  virtual obj_t type() {return can_msg_obj;};

};

#endif