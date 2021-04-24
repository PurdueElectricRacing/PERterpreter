#ifndef __INTEGER_H__
#define __INTEGER_H__

#include "object.h"
#include "typedefs.h"

struct Integer : public Object 
{
  int value;

  Integer(int value = 0) 
  { 
    o_type = integer; 
    this->value = value; 
  };
  
  virtual ~Integer() {};
  virtual obj_t type() {return integer;};
  virtual std::string stringify() { return std::to_string(value); };


  virtual std::vector<uint8_t> vectorize() {
    
    int size;
    // determine how many bytes the value is occupying.
    if (value < 0x10000) 
    {
      if (value < 0x100) // 8 bit
      {
        size = 1;
      }
      else // 16 bit
      {
        size = 2;
      }
    } 
    else 
    {
      if (value < 0x100000000L) // 32 bit
      {
        size = 3;
      }
      else // 64 bit
      {
        size = 4;
      }
    }
    
    std::vector<uint8_t> array(size);

    for (int i = 0; i < size; i++)
    {
      array[size - i - 1] = (value >> (i * 8));
    }

    return array;
  };

  virtual size_t size() {
      return sizeof(int);
  };
  
};

#endif