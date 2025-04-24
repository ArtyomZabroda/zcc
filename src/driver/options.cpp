#include "options.h"

namespace zcc::driver {

std::expected<Options, std::string> ParseOptions(int argc, char* argv[])  {
  Options opts;
  for (int i{1}; i < argc; ++i) {
    if (argv[i][0] != '-') {
      opts.source_paths.emplace_back(argv[i]);
    } else if (argv[i][1] != '\0') {
      switch (argv[i][1]) {
        case 'o':
          if (i + 1 < argc && argv[i + 1][0] != '-') {
            opts.output_path = argv[i + 1];
            ++i;
          }
          else {
            return std::unexpected("output file wasn't specified after -o");
          }
          break;
        default:
          return std::unexpected(std::format("option -{} is incorrect", argv[i][1]));
      }
    } else {
      return std::unexpected("expected an option after -");
    }
  }
  return opts;
}

}
