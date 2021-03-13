#include "perterpreter.h"
#include "cxxopts.hpp"
#include <QCoreApplication>
#include <string>



int main (int argc, char ** argv)
{
  using namespace cxxopts;
  QCoreApplication app(argc, argv);
  CanInterface * candev = 0;
  Perterpreter p;
  
  std::string infile, device, io, logfile, toutfile, sdest;
  std::vector<std::string> routines_to_run, tests_to_run;
  std::set<int> baud_rates = {
    125000,
    250000,
    500000,
    1000000
  };

  bool verb = false, syntax_check = false;
  bool good_syntax = false, no_gpio = false, no_can = false;
  int baud = 500000;
try
{
  Options options("perterpreter", "Interpreter for the PER automated" 
                                  "testing / scripting language");

  options.custom_help("filename [(-V, --validate} | (-rt, --run-tests testlist) | (-rr, --run-routines routinelist)] | -g dest [OPTIONS...]");

  options.positional_help("");

  options.add_options("Options")
    ("d,device", 
     "Name of the serial device for generic serial Tx/Rx.",
     value<std::string>(device))
    
    ("io", 
     "Name of the serial device controlling GPIO.",
     value<std::string>(io))
    
    ("v,verbose", 
     "Enable verbose printing of all script commands",
     value<bool>(verb))
    
    ("h,help", "Print help message")
    
    ("l,log", 
     "Filepath to use as the log file. I don't think this is necessary and "
     "honestly I'm not going to implement it until someone says they want it", 
     value<std::string>(logfile))

    ("t,test-output", 
     "Filename where test output should be logged", 
      value<std::string>(toutfile))

    ("infile", 
     "Filename for script file to run.", 
     value<std::string>(infile))

    ("g,generate-sample", 
     "Create a sample .pers file and put it in <dest>.", 
     value<std::string>(sdest))

    ("V,validate-syntax", "If this switch is passed, the interpreter will not" 
                          "execute. It will instead only perform syntax parsing"
                          " and type checking. This is useful for checking "
                          "scripts before actually running them if you don't "
                          "want to run them.", value<bool>(syntax_check))

    ("T,run-tests", "This is a comma-separated list of tests to run if you "
                     "don't want to run all of them. Default is to run all "
                     "tests. Example: '-T \"test 1\",\"test 2\". Ensure no "
                     "spaces between values. Optionally, you can pass this "
                     "parameter multiple times with each test name being the "
                     "subsequent parameter e.g. -T \"test 1\" -T \" test 2\"."
                     "The order specified will be the order scripts are run in",
                     value<std::vector<std::string> >(tests_to_run))

    ("R,run-routines", "This is a comma-separated list of routines to run if "
                        "you want to run any of them. Default behavior is to "
                        "not run any. See -T for examples, as the syntax is "
                        "the same. The order specified will be the order scripts are run in", 
                        value<std::vector<std::string> >(routines_to_run))
    ("b,baud", "The baud rate to use for the CAN device (only makes an impact"
               " on windows. If on Linux, use the `setup_can.sh` script)",
               value<int>(baud))
    ("no-gpio", "Passing this switch will disable the requirement for selecting"
                "a serial GPIO device. Only use this if your script makes no "
                "calls to pin read or write functions. If one of these "
                "functions is called with this switch passed, an exception "
                "will be thrown and the program will terminate.",
                value<bool>(no_gpio))
    ("no-can", "Passing this switch will disable the requirement for having"
               "a CAN device attached. Only use this if your script makes no "
               "calls to send-msg or read-msg. If one of these "
               "functions is called with this switch passed, an exception "
               "will be thrown and the program will terminate.",
               value<bool>(no_can))
    
    ;
  
  options.parse_positional({"infile"});
  auto cmdline = options.parse(argc, argv);

  if (cmdline.arguments().size() == 0)
  {
    printf("No arguments passed.\n");
    exit(-1);
  }

  if (cmdline.count("help"))
  {
    std::cout << options.help({"", "Options"}) << "\n";
    exit(0);
  }

  if (verb)
  {
    p.setVerbose(true);
  }
  // -g command provided
  if (cmdline.count("g"))
  {
    if (!sdest.empty())
    {
      p.createTemplateScript(sdest);
      exit(0);
    }
    else
    {
      std::cerr << "No destination provided for -g switch";
      exit(-1);
    }
  }

  // TODO routines to run and tests to run
  
  if (infile.empty())
  {
    std::cerr << "No input file specified.\n";
    std::cout << options.help({"", "Options"}) << "\n";
    exit(-1);
  }

  // print a passive-aggressive message about file extension
  if (infile.find(".pers") == std::string::npos)
  {
    std::cerr << "Invalid file extension. You seem to have forgotten the "
                  "'.pers' extension.\n";
    exit(-1);
  }

  if (!ghc::filesystem::exists(infile))
  {
    std::cerr << "File not found \"" << infile << "\"\n";
    exit(-1);
  }


  if (baud_rates.find(baud) == baud_rates.end())
  {
    std::cerr << "Invalid baud rate " << baud << "\n";
    exit(-1);
  }


  // this is a global variable that the error printer uses.
  good_syntax = p.performSyntaxAnalysis(infile);
  printf("\n");
  
  // terminate if this switch was passed
  if (syntax_check)
  {
    return good_syntax;
  }
  
  if (!good_syntax)
  {
    exit(-1);
  }

  if (!device.empty())
  {
    p.setSerialDev(device);
  }
  else
  {
    std::cerr << "No serial device selected. Any calls to serial-tx or "
                 "serial-rx will throw an exception.\n";
  }

  if (!io.empty())
  {
    p.setGpioDev(io);
  }
  else if (!no_gpio)
  {
    std::cout << "\n\nNo GPIO device was specified. Choose one from the list below\n\n";
    p.selectGpioDev();
  }
  else
  {
    std::cout << "\n\nno-gpio switch specified. Any calls to pin read or write"
                 " functions will throw exceptions.\n";
  }


  if (!no_can)
  {
    candev = NewCanDevice(baud);
    candev->Open();

    p.setCanInterface(candev);
  }
  else
  {
    std::cout << "\n\nno-can switch specified. Any calls to read-msg or"
                 " send-msg will raise an exception.\n";
  }

  if (!logfile.empty())
  {
    p.setLogFile(logfile);
  }

  if (!toutfile.empty())
  {
    p.setTestOutputFile(toutfile);
  }

  p.perterpret();
}
catch (std::exception& e)
{
  std::cerr << "Exception raised: " << e.what() << "\n";
}
  
}