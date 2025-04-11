#include "diag.h"
#include <stdio.h>

void DiagInit(struct DiagEngine *diag, struct BufferedFile *file) {
  diag->file = file;
}

void DiagReport(struct DiagEngine* diag, const char *current_ptr, enum DiagType diag_type) {
  fprintf(stderr, "error: %s\n", diag_descriptions[diag_type]);
}
