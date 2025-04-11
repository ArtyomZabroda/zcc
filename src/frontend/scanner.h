#ifndef SCANNER_H_
#define SCANNER_H_

#include "token.h"
#include "buffered_file.h"
#include "diag.h"

struct Scanner {
  struct BufferedFile *source_file;
  struct DiagEngine *diag_engine;
  const char *lexeme_start;
  int line;
};

void ScannerInit(struct Scanner *scanner, struct BufferedFile *source_file, struct DiagEngine *diag_engine);
void ScannerFree(struct Scanner *scanner);
struct Token Scan(struct Scanner *scanner);
void PrintScannerOutput(struct Scanner *scanner);

#endif
