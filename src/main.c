#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>

#include "buffered_file.h"
#include "crash.h"
#include "scanner.h"
#include "diag.h"
#include "dyn_array.h"

struct Options {
  char **source_paths;
  int source_paths_size;
  int source_paths_alloc;

  char* output_path;

  int print_scanner_output;
};

void InitOptions(struct Options *options) {
  DYNARRAY_INIT(options->source_paths, options->source_paths_size, options->source_paths_alloc);
  options->output_path = NULL;
  options->print_scanner_output = 0;/* TODO: Free options */ 
}

void FreeOptions(struct Options *options) {
  DYNARRAY_FREE(options->source_paths, options->source_paths_size, options->source_paths_alloc);
}

int CheckOption(const char *start, const char *opt) {
  int arg_len = strlen(start);
  return arg_len == strlen(opt) && memcmp(start, opt, arg_len) == 0;
}

int main(int argc, char **argv) {
  struct Options options;
  struct Scanner scanner;
  struct DiagEngine diag;
  struct BufferedFile file;
  int i;
  if (argc == 1) {
    Crash(2, "no input files");
  }

  InitOptions(&options);

  for (i = 1; i < argc; ++i) {
    if (argv[i][0] != '-') {
      DYNARRAY_ADD(options.source_paths, options.source_paths_size, options.source_paths_alloc, argv[i]);
    }
    else if (argv[i][1] != '\0') {
      switch (argv[i][1]) {
        case 'o':
          if (i + 1 < argc && argv[i + 1][0] != '-') {
            options.output_path = argv[i + 1];
            ++i;
          } else {
            /* TODO: throw error */ 
          }
          
          break;
        case '-':
          if (CheckOption(argv[i], "--print-tokens")) options.print_scanner_output = 1;
      }
    }
    else {
      /* TODO: do something about dash symbol */ 
    }
  }

  BufferedFileInit(&file, options.source_paths[0]);
  DiagInit(&diag, &file);
  ScannerInit(&scanner, &file, &diag);
  
  if (options.print_scanner_output) {
    PrintScannerOutput(&scanner);
  }

  ScannerFree(&scanner);
  BufferedFileFree(&file);

  FreeOptions(&options);
  return 0;
}
