#ifndef __COLORS_H__
#define __COLORS_H__


// TODO figure out how to get pretty colorz on winD'OHs
#ifndef WIN32
#define GREEN_TEXT "\u001b[32m"
#define RESET_TEXT "\u001b[0m"
#define RED_TEXT "\u001b[31m"
#define WHITE_TEXT "\u001b[37m"
#define CYAN_TEXT "\u001b[36m"
#else
#define GREEN_TEXT ""
#define RESET_TEXT ""
#define RED_TEXT ""
#define WHITE_TEXT ""
#define CYAN_TEXT ""
#endif
#endif