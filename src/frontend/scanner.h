#ifndef SCANNER_H_
#define SCANNER_H_

#include "token.h"
#include "diag.h"

struct Scanner {
  struct DiagEngine *diag_engine;
  const char *source;
  int source_size;
  const char *start;
  const char *current;
  int line;
  int is_at_start_of_line;
  int angled_include;
};

void ScannerInit(struct Scanner *scanner, const char *source, int source_size, struct DiagEngine *diag_engine);
struct Token Scan(struct Scanner *scanner);
void PrintScannerOutput(struct Scanner *scanner);

#endif
