
#include "parser.h"

#include "symbol-table.h"
#include "type-checker.h"
#include "perterpreter.h"
#include "operators.h"
#include "object-factory.h"
#include "serial-device.h"
#include "perterpreter_exceptions.h"
#include "colors.h"

#include "can_api.h"
#include "ghc/filesystem.hpp"


#include <exception>
#include <iostream>
#include <fstream>
#include <chrono>
#include <thread>
#include <memory>
#include <QIODevice>
#include <QTimer>
#include <QDebug>

extern FILE *yyin;
extern std::string infilename;
extern Node * root;
extern int errors;




// used for pseudo garbage collection of the intermediate Object * created
// during evaluations
std::set<Object *> intermediate_operands;
bool halt_execution = false;


/// @brief: private helper function for clearing any intermediate operands 
///         constructed during expression evaluation. It's a good idea to call 
///         this at the end of all the functions in case something in the 
///         call-stack allocates a new object and forgets to delete it.
void clearIntermediateOps(SymbolTable * scope)
{
  auto temp_ops = intermediate_operands;

  for (auto op = temp_ops.begin(); 
       op != temp_ops.end();
       ++op)
  {
    Object * operand = *op;
    if (operand)
    {
      if (!scope->find(operand) && operand)
      {
        intermediate_operands.erase(operand);
      }
      else
      {
        intermediate_operands.extract(operand);
      }
    }
  }
}



/// @brief: Function that gets called when the user specifies the -g, 
///         --generate-template parameter in command line or uses that 
///         functionality of the GUI
void Perterpreter::createTemplateScript(std::string spath)
{
  using namespace ghc::filesystem;
  path p = canonical(path(spath));
  
  if (is_directory(p))
  {
    p /= "example.pers";  
  }

  std::cout << "Creating sample file at " << p << "\n";

  // make sure the file extension is appended
  if (p.extension() != ".pers")
  {
    p += ".pers";
  }

  std::ofstream tgt(p.generic_string().c_str(), std::ofstream::out);

  tgt << "// BEGIN GLOBAL VARIABLE INITIALIZATIONS\n"
         "// ========================================\n"
         "\n\n# This is where you create or modify any global variables\n\n"
         "// ========================================\n"
         "// END GLOBAL VARIABLE INITIALIZATIONS\n"

         "\n// BEGIN ROUTINE DECLARATIONS\n"
         "// ========================================\n"
         "routine \"routine name\" {\n  perrintln \"Howdy friends.\";\n}\n\n"
         "// ========================================\n"
         "// END ROUTINE DECLARATIONS\n"

         "\n// BEGIN TEST DECLARATIONS\n"
         "// ========================================\n"
         "test \"test name\" {\n  assert EQ 1;\n}\n\n"
         "// ========================================\n"
         "// END TEST DECLARATIONS\n"
         ;
  tgt.close();
}



/// @brief: Perform the lexical analysis/parsing and type checking only
/// @return: true on valid syntax, false on error
bool Perterpreter::performSyntaxAnalysis(ghc::filesystem::path filepath)
{
  infilename = filepath.generic_string().c_str();
  yyin = fopen(filepath.generic_string().c_str(), "r");
  extern int yydebug;

#ifdef DEBUG
  // yydebug = 1;
#endif
  if (yyparse() != 0 || errors > 0)
  {
    std::cerr << "Failed to parse.\n";
    std::cerr << errors << " errors.\n";
    _root = root;
    return false;
  }
  _root = root;

  // free the memory if it's already been allocated for routine list 
  // and test list
  if (routines)
    delete routines;
  if (tests)
    delete tests;

  routines = new Routines();
  tests = new Tests();
  checkTypes(root, global_table, tests, routines);
  if (errors > 0)
  {
    std::cerr << errors << " errors.\n";
    return false;
  }
  return true;
}




/// @brief: Wrapper for symbol table getObject function; Will return CAN 
///         message indexes and lengths as well as base objects.
///         Adds any new objects to the intermediate_ops vector
Object * Perterpreter::getObject(Node * node, SymbolTable * scope)
{
  Object * ret = 0;

  if (node->isIdentifier())
  {
    ret = scope->getObject(node->data.strval);
    // accessing a member of the object
    if (node->children.size() > 0)
    {
      Node * exp = node->children[0];
      CAN_Msg * msg = static_cast<CAN_Msg*>(ret);

      if (exp->node_type == length_node)
      {
        ret = new Integer(msg->length());
        intermediate_operands.emplace(ret);
      }
      else if (exp->node_type == index_node)
      {
        // return the index key

        Node * idx = exp->children[0];
        Integer * val = static_cast<Integer *>(perterpretExp(idx, scope));
        ret = new Integer(msg->get(val->value));

        intermediate_operands.emplace(ret);
        intermediate_operands.emplace(val);
      }
    }
  }
  else if (node->isLiteral())
  {
    ret = ObjectFactory::createObject(node);
    intermediate_operands.emplace(ret);
  }
  else if (node->node_type == unary_math_node)
  {
    ret = perterpretExp(node, scope);
  }
  return ret;
}





void Perterpreter::perterpretExpectAssert(Node * node, SymbolTable * scope)
{
  Node * exp = node->children[0];
  Object * expectation = perterpretExp(exp, scope);
  Integer * i = static_cast<Integer *>(expectation);
  std::string failure_string = "Expectation unsatisified on line ";

  if (!i->value)
  {
    failure_string += std::to_string(node->line_no) + "\n";
    std::cerr << failure_string;

    // mark the test as failed
    if (scope->type() == test_table)
    {
      Test * t = static_cast<Test *>(scope);
      t->failTest();
      t->setFailReason(failure_string);
    }
  }

  // kill the current execution
  if (node->node_type == assert_node)
  {
    // TODO assertion nodes
    halt_execution = true;
  }
}



/// @brief: Performs a binary expression evaluation
///         adds any newly created objects to the intermediate_ops vector
Object * Perterpreter::perterpretBinaryOp(Node * node, SymbolTable * scope)
{
  Node * lhs = node->children[0];
  Node * rhs = node->children[1];

  Object * ret = 0;
  Object * rhso = 0;
  Object * lhso = 0;

  lhso = perterpretExp(lhs, scope);
  rhso = perterpretExp(rhs, scope);

  if (node->node_type == comparison_node)
  {
    ret = new Integer(compare(lhso, rhso, node->data.strval));
    intermediate_operands.emplace(ret);
  }
  // concatenating a string
  else if (lhso->type() == str || rhso->type() == str)
  {
    ret = new String(concat(lhso, rhso));
    intermediate_operands.emplace(ret);
  }
  // math operation
  else
  {
    ret = new Integer(math(lhso, rhso, node->data.strval));
    intermediate_operands.emplace(ret);
  }

  return ret;
}



/// @brief: Interpret any arbitrary expression. Will push new objects to the 
///         intermediate ops vector
Object * Perterpreter::perterpretExp(Node * node, SymbolTable * scope)
{
  Object * ret = 0;
  Integer * i = 0;
  // binary operator  
  if (node->children.size() == 2)
  {
    ret = perterpretBinaryOp(node, scope);
  }
  else if (node->isLiteral())
  {
    Object * o = ObjectFactory::createObject(node);
    intermediate_operands.emplace(o);
    return o;
  }
  else if (node->isIdentifier())
  {
    return getObject(node, scope);
  }
  else if (node->node_type == unary_math_node)
  {
    Node * object = node->children[0];
    Object * o = 0;

    // if there are multiple children then the node must be a can object
    if (object->children.size() > 0)
    {
      Node * child_2 = object->getChild(index_node);

      if (child_2)
      {
        Node * idx_node = child_2->children[0];
        // get the can-message variable 
        CAN_Msg * msg = static_cast<CAN_Msg *>(scope->getObject(object->data.strval));
        // get the index value
        i = static_cast<Integer *>(getObject(idx_node, scope));
        int val = msg->get(i->value);
        msg->setData(i->value, unaryMath(val, node->data.strval));    
        i = new Integer(msg->get(i->value));    
      }
    }
    // if it's not a special can-message access then just do normal object creation and stuff
    else
    {
      o = getObject(object, scope);
      i = new Integer(unaryMath(o, node->data.strval));
    }
    intermediate_operands.emplace(i);
    ret = i;
  }
  
  return ret;
}




/// @brief: perform a delay call
void Perterpreter::perterpretDelay(Node * node, SymbolTable * scope)
{
  Node * delay = node->children[0];
  size_t delayval = 0;

  if (delay->isLiteral())
  {
    delayval = delay->data.intval;
  }
  else
  {
    Integer * i = static_cast<Integer *>
                  (perterpretExp(node->children[0], scope));
    delayval = i->value;
  }
  if (verbose)
  {
    qDebug() << "\nDelaying execution for" << delayval << "ms";
  }

  std::this_thread::sleep_for(std::chrono::milliseconds(delayval));
}



/// @brief: perform a variable assignment / reassignment
void Perterpreter::perterpretVardecl(Node * node, SymbolTable * scope)
{
  // fetch the object being assigned to
  Node * rhs = node->children[0];
  Object * lhso = scope->getObject(node->data.strval);
  Object * rhso = perterpretExp(rhs, scope);

  // length or index node too
  if (node->children.size() > 1 && lhso->type() == can_msg_obj)
  {
    CAN_Msg * can = static_cast<CAN_Msg *>(lhso);

    // second child is always the access modifier
    Node * exp = node->children[1];
    Integer * target_val = static_cast<Integer *>(rhso);

    if (exp->node_type == index_node)
    {
      Node * idx = exp->children[0];
      Integer * val = static_cast<Integer *>(perterpretExp(idx, scope));

      if (can->setData(val->value, target_val->value) != target_val->value)
      {
        outOfBoundsError(can->length(), val->value, node->line_no);
      }
    }
    else if (exp->node_type == length_node)
    {
      can->setLeng(target_val->value);
    }
  }
  else
  {
    scope->setObject(node->data.strval, rhso);
  }

}




/// @brief: performs a print / println
void Perterpreter::perterpretPrint(Node * node, SymbolTable * scope)
{
  Node * exp = node->children[0];
  Object * output = perterpretExp(exp, scope);

  printf("%s", output->stringify().c_str());

  if (node->node_type == println)
  {
    std::cout << std::endl;
  }

}




/// @brief: perform a prompt blocking call, and interpret the expression arg
///         and print it to stdout. Blocks until user presses enter
void Perterpreter::perterpretPrompt(Node * node, SymbolTable * scope)
{
  Node * exp = node->children[0];
  Object * value = perterpretExp(exp, scope);
  std::string whatever;

  printf("%s\n", value->stringify().c_str());
  printf("Press enter to continue...\n");

  std::getline(std::cin, whatever);
}




/// @brief: Interpret an if or if/else block
void Perterpreter::perterpretIf(Node * node, SymbolTable * scope)
{
  Node * exp = node->children[0];
  Node * statements = node->children[1];
  Node * elsenode = 0;
  Node * else_statements;

  Object * value = perterpretExp(exp, scope);
  Integer * i = static_cast<Integer *>(value);

  if (i->value)
  {
    perterpretNode(statements, scope);
  }
  else
  {
    // if there is an else clause perform that 
    if ((elsenode = node->getChild(else_node)))
    {
      else_statements = elsenode->children[0];
      perterpretNode(else_statements, scope);
    }
  }

}




void Perterpreter::perterpretCall(Node * node, SymbolTable * scope)
{
  // the routine name has to be the next argument and it has to be a literal
  std::string routine_name = node->data.strval;
  Routine * r = routines->getRoutine(routine_name, node->line_no);
  perterpretNode(r->getRoot(), r);

}




/// @brief: Execute the loops
void Perterpreter::perterpretLoop(Node * node, SymbolTable * scope)
{
  Node * exp = node->children[0];
  Node * statements = node->children[1];

  if (exp->node_type == forever_node)
  {
    while(PER == GREAT)
    {
      perterpretNode(statements, scope);
    }
    return;
  }
  else
  {
    Integer * i = static_cast<Integer *>(perterpretExp(exp, scope));

    while (i->value)
    {
      perterpretNode(statements, scope);
      i = static_cast<Integer *>(perterpretExp(exp, scope));
    }
  }  
}




/// @brief: Dispatcher for interpreting list nodes. Calling this with a 
///         non-list node will result in undefined behavior.
void Perterpreter::perterpretNode(Node * node, SymbolTable * scope)
{
  for (auto n = node->children.begin(); n != node->children.end() 
      && !halt_execution; ++n)
  {
    Node * child = *n;
    node_type_t nodetype = child->node_type;

    // recurse on the list nodes
    switch (nodetype)
    {
      case (vardecl_list_node):
      case (statement_list):
      {
        perterpretNode(child, scope);
        break;
      }
      case (unary_math_node):
        perterpretExp(child, scope);
        break;
      case (vardecl_node):
        perterpretVardecl(child, scope);
        break;
      case (call_node):
        perterpretCall(child, scope);
        break;
      case (delay_node):
        perterpretDelay(child, scope);
        break;
      case (loop_node):
        perterpretLoop(child, scope);
        break;
      case (expect_node):
      case (assert_node):
        perterpretExpectAssert(child, scope);
        break;
      case (print):
      case (println):
        perterpretPrint(child, scope);
        break;
      case (exit_node):
        // TODO exit code
        std::cout << node->data.strval << "\n";
        exit(0);
        break;
      case (serial_tx):
        perterpretSerialTx(child, scope);
        break;
      case (serial_rx):
        perterpretSerialRx(child, scope);
        break;
      case (prompt_node):
        perterpretPrompt(child, scope);
        break;
      case (digital_read):
        perterpretDRead(child, scope);
        break;
      case (digital_write):
        perterpretDWrite(child, scope);
        break;
      case (analog_read):
        perterpretAread(child, scope);
        break;
      case (analog_write):
        perterpretAwrite(child, scope);
        break;
      case (send_msg_node):
        perterpretSendMsg(child, scope);
        break;
      case (read_msg_node):
        perterpretReadMsg(child, scope);
        break;
      case (if_node):
        perterpretIf(child, scope);
        break;
      default:
        break;
    }
  
  }
}


void Perterpreter::runTest(Test * test)
{
  std::string test_name = test->getName();
  std::string output;
  std::string result;
  output += "======================================\n"
            WHITE_TEXT "Running test " CYAN_TEXT "'" + test_name
            + "'" RESET_TEXT "\n======================================\n";
  
  std::cout << output;
  perterpretNode(test->root, test);
  
  result = "======================================\n"
           WHITE_TEXT "Test " CYAN_TEXT "'" + test_name + "'" RESET_TEXT " [ ";

  if (test->testPassed())
  {
    result += GREEN_TEXT "PASSED" RESET_TEXT " ]\n";

  }
  else 
  {
    result += RED_TEXT "FAILED" RESET_TEXT " ]\n"
              "    Reason: " + test->getReason();
  }
  result += "======================================\n";
  std::cout << result;
  output += result;
  
  // make sure the output file is actually open
  if (test_output.is_open())
  {
    test_output << output;
  }
}


/// @brief: This is what it's all been for. PER has it's own language now
void Perterpreter::perterpret(std::string func)
{
  std::string output;

  if (func.empty())
  {
    // interpret all the tests only
    perterpretNode(root, global_table);

    for (auto t = tests->tests.begin(); t != tests->tests.end(); t++)
    {
      Test * test = t->second; 
      runTest(test);
      // TODO timer to periodically garbage collect
      // TODO color output conditionally 
    }
  }
  else if (tests->hasTest(func))
  {
    Test * test = tests->getTest(func);
    runTest(test);
  }
  else if (routines->hasRoutine(func))
  {
    Routine * r = routines->getRoutine(func);
    perterpretNode(r->root, r);
  }
  else
  {
    std::cerr << "Function '" << func << "' is not defined.\n";
    exit(-1);
  }
}



/// @brief: write some message to a connected generic serial device. 
/// @throws: SerialDeviceNotConnected if no device is connected
void Perterpreter::perterpretSerialTx(Node * node, SymbolTable * scope)
{
  if (serial_device->isOpen())
  {
    String * val = static_cast<String *>(getObject(node->children[0], scope));
    QString cmd = val->value.c_str();
    serial_device->sendCommand(cmd);
  }
  else
  {
    throw SerialDeviceNotConnected;
  }
}




/// @brief: Read from a connected generic serial device. 
/// @throws: SerialDeviceNotConnected if no device is connected
void Perterpreter::perterpretSerialRx(Node * node, SymbolTable * scope)
{
  if (serial_device->isOpen())
  {
    scope->setObject("RETVAL", 
                     new String(serial_device->serialRead().toStdString()));
  }
  else
  {
    throw SerialDeviceNotConnected;
  }
}




/// @brief: Perform the operation for writing a digital pin to the state 
///         specified in the script. Sends a serial command to an arduino 
///         controller
/// @throws: GPIODeviceNotConnected if no gpio device is connected
void Perterpreter::perterpretDWrite(Node * node, SymbolTable * scope)
{
  if (gpio_device->isOpen())
  {
    Node * pin_node = node->children[0];
    Integer * pin = static_cast<Integer*>(getObject(pin_node, scope));
    int state = node->data.intval;
    gpio_device->digitalWrite(pin->value, state);
  }
  else
  {
    throw GPIODeviceNotConnected;
  }
}



/// @brief: Perform the operation for writing a DAC pin to the state 
///         specified in the script. Sends a serial command to an arduino 
///         controller
/// @throws: GPIODeviceNotConnected if no gpio device is connected
void Perterpreter::perterpretAwrite(Node * node, SymbolTable * scope)
{
  if (gpio_device->isOpen())
  {
    Node * pin_node = node->children[0];
    Node * val_node = node->children[1];
    Integer * pin = static_cast<Integer*>(getObject(pin_node, scope));
    Integer * value = static_cast<Integer *>(getObject(val_node, scope));
    gpio_device->analogWrite(pin->value, value->value);
  }
  else
  {
    throw GPIODeviceNotConnected;
  }
}



/// @brief: Perform the operation for reading a digital pin's state 
///         specified in the script. Sends a serial command to an arduino 
///         controller
/// @throws: GPIODeviceNotConnected if no gpio device is connected
void Perterpreter::perterpretDRead(Node * node, SymbolTable * scope)
{
  if (gpio_device->isOpen())
  {
    Node * pin_node = node->children[0];
    Integer * pin = static_cast<Integer*>(getObject(pin_node, scope));
    Integer * value = new Integer(gpio_device->digitalRead(pin->value));
    scope->setRetval(value);
  }
  else
  {
    throw GPIODeviceNotConnected;
  }
}




/// @brief: Perform the operation for reading an analog pin's state 
///         specified in the script. Sends a serial command to an arduino 
///         controller
/// @throws: GPIODeviceNotConnected if no gpio device is connected
void Perterpreter::perterpretAread(Node * node, SymbolTable * scope)
{
  if (gpio_device->isOpen())
  {
    Node * pin_node = node->children[0];
    Integer * pin = static_cast<Integer*>(getObject(pin_node, scope));
    Integer * value = new Integer(gpio_device->analogRead(pin->value));
    scope->setRetval(value);
  }
  else
  {
    throw GPIODeviceNotConnected;
  }
}




void Perterpreter::perterpretSendMsg(Node * node, SymbolTable * scope)
{
  Node * addrnode = node->children[0];
  Node * msgnode = node->children[1];
  Integer * address = static_cast<Integer *>(getObject(addrnode, scope));
  CAN_Msg * msg = static_cast<CAN_Msg *>(getObject(msgnode, scope));
  std::stringstream ss;
  ss << std::hex << address->value;

  if (verbose)
  {
    qDebug() << "\nSending" << QString(msg->toString().c_str()) << "from" 
             << QString(("0x" + ss.str()).c_str()); 
  }

  if (!can_if)
  {
    throw CANDeviceNotConnected;
  }

  can_if->writeCanData(address->value, msg->Len(), msg->Data());
}




void Perterpreter::perterpretReadMsg(Node * node, SymbolTable * scope)
{
  Node * addrnode = node->children[0];
  Integer * address = static_cast<Integer *>(getObject(addrnode, scope));
  CAN_Msg * msg = 0;
  QTimer read_timer;
  std::stringstream ss;

  if (verbose)
  {
    ss << std::hex << address->value;
    // go ahead and read once, then set a timer for later
    qDebug() << "\nReading from CAN bus. Expecting to receive a message from" 
             << QString(("0x" + ss.str()).c_str());  // this is whack yo
  }

  if (!can_if)
  {
    throw CANDeviceNotConnected;
  }

  CanFrame frame = can_if->readCanData();

  // start a timer for reading the message
  read_timer.start(CAN_READ_TIMER_TIMEOUT);

  while (frame.can_id != address->value && read_timer.remainingTime() > 0)
  {
    frame = can_if->readCanData();
  }

  read_timer.stop();
  msg = new CAN_Msg(frame);

  scope->setObject("RETVAL", msg);

  if (frame.can_id != address->value)
  {
    std::cerr << "\nNo message received from " << std::hex << address->value << "\n";
  }

  if (frame.can_dlc > 0 && frame.can_id != 0)
  {
    if (verbose)
    {
      ss.str("");  
      ss << std::hex << frame.can_id;
      std::cout << "\nRead message " << frame << " from " 
                << "0x" + ss.str() << "\n";
    }
  }

  // TODO figure out how to display the messages to the test bench here if its being used
  if (data_map)
  {
    // data_map[frame.can_id] = frame;
  }
}




void Perterpreter::selectGpioDev()
{ 
  if (gpio_device->selectSerialPort().isNull())
  {
    throw NoGPIODevicesFound;
  }
};

