# Created by and for Qt Creator This file was created for editing the project sources only.
# You may attempt to use it for building too, by modifying this file here.

#TARGET = interpreter

QT = core widgets serialport

CONFIG += c++17 warn_on debug

INCLUDEPATH += $$PWD/inc
DESTDIR = build/
SRCDIR = $$PWD/src
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
}
win32 {
   # windows compiler flags
   # TODO figure out how to build flex/bison files on winders
}




MOC_DIR = $$PWD/moc
TARGET = /perterpreter

DEFINES += STANDALONE GREAT=1 PER=GREAT DEBUG


HEADERS = \
   $$PWD/inc/ast.h \
   $$PWD/inc/canmsg.h \
   $$PWD/inc/cxxopts.hpp \
   $$PWD/inc/integer.h \
   $$PWD/inc/object-factory.h \
   $$PWD/inc/object.h \
   $$PWD/inc/operators.h \
   $$PWD/inc/parser.h \
   $$PWD/inc/perterpreter.h \
   $$PWD/inc/strobj.h \
   $$PWD/inc/symbol-table.h \
   $$PWD/inc/synterr.h \
   $$PWD/inc/type-checker.h \
   $$PWD/inc/typedefs.h \
   $$PWD/src/lexer.l \
   $$PWD/src/parser.y \
   $$PWD/inc/serial-device.h \
   $$PWD/inc/perterpreter_exceptions.h \


SOURCES = \
   $$PWD/src/ast.cpp \
   $$PWD/src/lexer.cpp \
   $$PWD/src/operators.cpp \
   $$PWD/src/parser.cpp \
   $$PWD/src/perterpreter.cpp \
   $$PWD/src/stringify.cpp \
   $$PWD/src/symbol-table.cpp \
   $$PWD/src/synterr.cpp \
   $$PWD/src/type-checker.cpp \
   $$PWD/src/main.cpp \

INCLUDEPATH = \
    $$PWD/inc

include($$PWD/../CAN_API/canapi.pri)

#DEFINES = 

