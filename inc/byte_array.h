
#ifndef __BYTE_ARRAY_H__
#define __BYTE_ARRAY_H__


#include "object.h"
#include <cstring>
#include <sstream>
#include <iomanip>
#include <exception>
#include <iostream>
#include <vector>



/// @brief: Array class which allows the sending of arbitrary bytes over serial

class ByteArray : public Object
{

protected:
  std::vector<uint8_t> data;
  size_t leng;


public:

  ByteArray(std::string input = ""){
    leng = 0;
    parse(input);
    o_type = byte_array_obj;
  };

  virtual ~ByteArray(){};

  void parse(std::string input)
  {
    size_t i = 0; 
    size_t last_sep = input.find("|");

    try
    {
      while (last_sep != std::string::npos)
      {
        std::string byteval = input.substr(0, last_sep);
        size_t val = 0;
        input.erase(0, last_sep + 1);
        byteval = "0x" + byteval;
        val = std::stoul(byteval, 0, 16);

        if (val > 0xFF)
        {
          return;
        }
        data.push_back(val);

        last_sep = input.find("|");
        i++;
      }
      if (!input.empty())
      {
        input = "0x" + input;
        data.push_back(std::stoul(input, 0, 16));
      }
      leng = data.size();
    }
    catch (std::exception& e)
    {
      std::cerr << e.what();
    }
  };

  virtual obj_t type() {return byte_array_obj;};

  virtual std::string stringify() { 
    std::string ret;

    for (size_t i = 0; i < leng; i++)
    {
      // do not move this variable out of the loop
      // it needs re-init every iteration
      std::stringstream ss;
      ss << std::hex << (size_t) data[i];
      ret += ss.str();
      if (i < leng - 1)
        ret += "|";
    }
    return ret;
  };

  bool validData() {return data.size() > 0;};

  uint8_t get(int idx)
  {
    if (idx >= leng)
    {
      return data[idx % leng];
    }
    return data[idx];
  };

  size_t length() {
    return leng;
  }

  uint8_t setData(size_t index, size_t val) 
  {
    if (index > leng)
    {
      return -1;
    }
    data[index] = val;
    return val;
  }

  size_t setLeng(uint8_t newlen) { 
    if (newlen > leng)
    {
      size_t diff = newlen - leng;
      for (int i = 0; i < diff; i++)
      {
        data.push_back(0);
      }
    }
    leng = newlen; 
    return newlen;
  };

  std::string toString()
  {
    std::string ret = "[";
    for (int i = 0; i < leng; i++)
    {
      ret += " " + std::to_string(data[i]);
    }

    ret += " ]";
    return ret;
  }

  uint8_t * Data() { return data.data(); };
  int Len() { return leng; };

  std::ostream& operator<<(std::ostream &stream)
  {
    stream << toString();
    return stream;
  }

  virtual std::vector<uint8_t> vectorize()
  {
    return data;
  }

  virtual size_t size() {
    return leng;
  }

};

#endif