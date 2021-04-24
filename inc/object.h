#ifndef __OBJECT_H__
#define __OBJECT_H__

#include <string>
#include "typedefs.h"
#include <iostream>
#include <vector>
struct Object
{
  Object() {};
  virtual ~Object() {}
  virtual obj_t type(){return none;};
  virtual std::string stringify(){ 
    return std::to_string((size_t)this) + ": type: none";
  };
  virtual std::vector<uint8_t> vectorize(){
    return std::vector<uint8_t>();
  };

  virtual size_t size() {
    return sizeof(Object);
  }

  obj_t o_type = none;
};


#endif