/* A Bison parser, made by GNU Bison 3.7.3.  */

/* Bison interface for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2015, 2018-2020 Free Software Foundation,
   Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.

   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */

/* DO NOT RELY ON FEATURES THAT ARE NOT DOCUMENTED in the manual,
   especially those whose name start with YY_ or yy_.  They are
   private implementation details that can be changed or removed.  */

#ifndef YY_YY_INC_PARSER_H_INCLUDED
# define YY_YY_INC_PARSER_H_INCLUDED
/* Debug traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif
#if YYDEBUG
extern int yydebug;
#endif
/* "%code requires" blocks.  */
#line 2 "src/parser.y"

  #include <string>
  #include <iostream>
  #include <cstring>

  #include "ast.h"

#line 57 "inc/parser.h"

/* Token kinds.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
  enum yytokentype
  {
    YYEMPTY = -2,
    YYEOF = 0,                     /* "end of file"  */
    YYerror = 256,                 /* error  */
    YYUNDEF = 257,                 /* "invalid token"  */
    routine = 258,                 /* routine  */
    test = 259,                    /* test  */
    loop = 260,                    /* loop  */
    delay = 261,                   /* delay  */
    read_pin = 262,                /* read_pin  */
    set_pin = 263,                 /* set_pin  */
    perrint = 264,                 /* perrint  */
    perrintln = 265,               /* perrintln  */
    expect = 266,                  /* expect  */
    exit_tok = 267,                /* exit_tok  */
    prompt = 268,                  /* prompt  */
    read_msg = 269,                /* read_msg  */
    send_msg = 270,                /* send_msg  */
    call = 271,                    /* call  */
    forever = 272,                 /* forever  */
    dout = 273,                    /* dout  */
    din = 274,                     /* din  */
    aout = 275,                    /* aout  */
    ain = 276,                     /* ain  */
    serialRx = 277,                /* serialRx  */
    serialTx = 278,                /* serialTx  */
    assert = 279,                  /* assert  */
    If = 280,                      /* If  */
    Else = 281,                    /* Else  */
    length = 282,                  /* length  */
    stringLiteral = 283,           /* stringLiteral  */
    identifier = 284,              /* identifier  */
    can_msg = 285,                 /* can_msg  */
    add = 286,                     /* add  */
    mult = 287,                    /* mult  */
    comparison = 288,              /* comparison  */
    andToken = 289,                /* andToken  */
    orToken = 290,                 /* orToken  */
    NE = 291,                      /* NE  */
    EQ = 292,                      /* EQ  */
    integerLiteral = 293,          /* integerLiteral  */
    hexLiteral = 294,              /* hexLiteral  */
    dstate = 295,                  /* dstate  */
    plusplus = 296,                /* plusplus  */
    minusminus = 297               /* minusminus  */
  };
  typedef enum yytokentype yytoken_kind_t;
#endif

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
union YYSTYPE
{
#line 10 "src/parser.y"

  Node * node;
  int i;
  std::string * str;

#line 122 "inc/parser.h"

};
typedef union YYSTYPE YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif

/* Location type.  */
#if ! defined YYLTYPE && ! defined YYLTYPE_IS_DECLARED
typedef struct YYLTYPE YYLTYPE;
struct YYLTYPE
{
  int first_line;
  int first_column;
  int last_line;
  int last_column;
};
# define YYLTYPE_IS_DECLARED 1
# define YYLTYPE_IS_TRIVIAL 1
#endif


extern YYSTYPE yylval;
extern YYLTYPE yylloc;
int yyparse (void);

#endif /* !YY_YY_INC_PARSER_H_INCLUDED  */
