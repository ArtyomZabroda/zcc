#include "diag.h"
#include <stdio.h>

void DiagInit(struct DiagEngine *diag, const char *source) {
  diag->source = source;
}

void DiagReport(struct DiagEngine* diag, const char *current_ptr, enum DiagType diag_type) {
  fprintf(stderr, "error: %s\n", diag_descriptions[diag_type]);
}
