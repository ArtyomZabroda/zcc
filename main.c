#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>

struct ProgramArguments {
  char file_to_compile[201];
  int file_to_compile_len;
  char output_file_name[201];
  int output_file_name_len;
};

void ParseArguments(int argc, char** argv, struct ProgramArguments* parsed_args) {

  int opt;

  while ((opt = getopt(argc, argv, "o:")) != -1) {
    switch (opt) {
      case 'o':
        // TODO: Сделать проверку на количество символов
        parsed_args->output_file_name_len = snprintf(parsed_args->output_file_name,200,"%s",optarg);
        break;
    }
    // TODO: проверить наличие файла для компиляции
    strcpy(parsed_args->file_to_compile, argv[optind]);
    parsed_args->file_to_compile_len = strlen(parsed_args->output_file_name);
  }


}

int main(int argc, char** argv) {
  struct ProgramArguments args;

  ParseArguments(argc, argv, &args);

  printf("%s\n", args.file_to_compile);
  printf("%s\n", args.output_file_name);

  return 0;
}

