# Created by and for Qt Creator This file was created for editing the project sources only.
# You may attempt to use it for building too, by modifying this file here.

#TARGET = interpreter

QT = core widgets serialport

CONFIG += c++17 warn_on debug console

INCLUDEPATH += ./inc
DESTDIR = ./build/
SRCDIR = ./src
OBJECTS_DIR = obj/


flex.output = src/lexer.cpp
flex.input = DESTDIR
flex.name = lexer
flex.CONFIG = target_predeps
flex.variable_out = SRCDIR


bison.output = src/bison.cpp
bison.input = DESTDIR
bison.name = lexer
bison.CONFIG = target_predeps
bison.variable_out = SRCDIR

unix:!macx {
   # linux compiler commands
   flex.commands = flex -o src/lexer.cpp src/lexer.l
   bison.commands = bison -v -g -d --color -o src/parser.cpp --defines=inc/parser.h src/parser.y
   QMAKE_EXTRA_COMPILERS += bison flex
   TARGET = /perterpreter
}
win32 {
   # windows compiler flags
   # TODO figure out how to build flex/bison files on winders
}


MOC_DIR = ./moc

DEFINES += STANDALONE GREAT=1 PER=GREAT DEBUG


HEADERS = \
   ./inc/ast.h \
   ./inc/canmsg.h \
   ./inc/cxxopts.hpp \
   ./inc/integer.h \
   ./inc/object-factory.h \
   ./inc/object.h \
   ./inc/operators.h \
   ./inc/parser.h \
   ./inc/perterpreter.h \
   ./inc/strobj.h \
   ./inc/symbol-table.h \
   ./inc/synterr.h \
   ./inc/type-checker.h \
   ./inc/typedefs.h \
   ./src/lexer.l \
   ./src/parser.y \
   ./inc/serial-device.h \
   ./inc/perterpreter_exceptions.h \


SOURCES = \
   ./src/ast.cpp \
   ./src/lexer.cpp \
   ./src/operators.cpp \
   ./src/parser.cpp \
   ./src/perterpreter.cpp \
   ./src/stringify.cpp \
   ./src/symbol-table.cpp \
   ./src/synterr.cpp \
   ./src/type-checker.cpp \
   ./src/main.cpp \

INCLUDEPATH = \
    ./inc

include(./DesktopCAN_API/canapi.pri)

#DEFINES = 

