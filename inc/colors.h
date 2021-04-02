#ifndef __COLORS_H__
#define __COLORS_H__

#include <string>

#ifdef WIN32
#include <windows.h>
HANDLE h_console;
bool init = false;
#endif


enum colors_t
{
  GREEN,
  RED,
  RESET,
  WHITE,
  CYAN
};


static std::string linuxColors(colors_t color)
{
  switch (color)
  {
    case(GREEN):
      return "\u001b[32m";
    case(RED):
      return "\u001b[31m";
    case(RESET):
      return "\u001b[0m";
    case(WHITE):
      return "\u001b[37m";
    case(CYAN):
      return "\u001b[36m";
    default:
      return "";
  }
}

static std::string setTextColor(colors_t color)
{
#ifndef WIN32
  return linuxColors(color);
#else
  if (!init)
  {
    h_console = GetStdHandle(STD_OUTPUT_HANDLE);
    init = true;
  }
  int color = 15;
  switch (color)
    {
      case(GREEN):
        color = 2;
      case(RED):
        color = 4;
      case(RESET):

      case(WHITE):
      case(CYAN):
      default:
        color = 15;
  }
  SetConsoleTextAttribute(hConsole, color);
  return "";
#endif
}
// TODO figure out how to get pretty colorz on winD'OHs
#endif