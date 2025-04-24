#include <gtest/gtest.h>
#include <string>
#include <vector>
#include "options.h"

namespace zcc::driver {

TEST(OptionsTest, FilenamesAreInitializedCorrectly) {
  int argc = 3;
  char* argv[3];
  argv[0] = new char[] {"no_path"};
  argv[1] = new char[] {"hello.c"};
  argv[2] = new char[] {"world.cpp"};
  std::expected<Options, std::string> options = ParseOptions(argc, argv);
  EXPECT_STREQ(options->source_paths[0].c_str(), argv[1]);
  EXPECT_STREQ(options->source_paths[1].c_str(), argv[2]);

  delete[] argv[0];
  delete[] argv[1];
}

TEST(OptionsTest, OptionIsNotRecognizedAsAFilename) {
  int argc = 4;
  char* argv[4];
  argv[0] = new char[] {"no_path"};
  argv[1] = new char[] {"hello.c"};
  argv[2] = new char[] {"-o"};
  argv[3] = new char[] {"hello"};
  std::expected<Options, std::string> options = ParseOptions(argc, argv);
  EXPECT_EQ(options->source_paths.size(), 1);

  delete[] argv[0];
  delete[] argv[1];
  delete[] argv[2];
  delete[] argv[3];
}

TEST(OptionsTest, OutputFileIsInitializedCorrectly) {
  int argc = 4;
  char* argv[4];
  argv[0] = new char[] {"no_path"};
  argv[1] = new char[] {"-o"};
  argv[2] = new char[] {"hello"};
  argv[3] = new char[] {"hello.c"};
  std::expected<Options, std::string> options = ParseOptions(argc, argv);
  EXPECT_STREQ(options->output_path.c_str(), argv[2]);

  delete[] argv[0];
  delete[] argv[1];
  delete[] argv[2];
  delete[] argv[3];
}

TEST(OptionsTest, OutputFileWasNotSpecified) {
  int argc = 2;
  char* argv[2];
  argv[0] = new char[] {"no_path"};
  argv[1] = new char[] {"-o"};
  std::expected<Options, std::string> options = ParseOptions(argc, argv);
  EXPECT_FALSE(options.has_value());

  delete[] argv[0];
  delete[] argv[1];
}

TEST(OptionsTest, IncorrectOption) {
  int argc = 4;
  char* argv[4];
  argv[0] = new char[] {"no_path"};
  argv[1] = new char[] {"-q"};
  argv[2] = new char[] {"hello"};
  argv[3] = new char[] {"hello.c"};

  std::expected<Options, std::string> options = ParseOptions(argc, argv);
  EXPECT_FALSE(options.has_value());

  delete[] argv[0];
  delete[] argv[1];
  delete[] argv[2];
  delete[] argv[3];
}

TEST(OptionsTest, HyphenWithoutOption) {
  int argc = 4;
  char* argv[4];
  argv[0] = new char[] {"no_path"};
  argv[1] = new char[] {"-"};
  argv[2] = new char[] {"hello"};
  argv[3] = new char[] {"hello.c"};

  std::expected<Options, std::string> options = ParseOptions(argc, argv);
  EXPECT_FALSE(options.has_value());

  delete[] argv[0];
  delete[] argv[1];
  delete[] argv[2];
  delete[] argv[3];
}

}

