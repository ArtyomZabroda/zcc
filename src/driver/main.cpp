#include <iostream>
#include <options.h>

#include "options.h"

namespace zcc::driver {

int DriverMain(int argc, char** argv) {
  std::expected<Options, std::string> options = ParseOptions(argc, argv);
  return 0;
}

}

int main(int argc, char** argv) {
  return zcc::driver::DriverMain(argc, argv);
}

