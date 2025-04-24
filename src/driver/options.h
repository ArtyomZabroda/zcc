#ifndef DRIVER_OPTIONS_H_
#define DRIVER_OPTIONS_H_

#include <vector>
#include <string>
#include <expected>
#include "crash.h"

namespace zcc::driver {

struct Options {
  std::vector<std::string> source_paths;
  
  std::string output_path;
};
  
std::expected<Options, std::string> ParseOptions(int argc, char* argv[]);

}



#endif
