#ifndef DIAG_H_
#define DIAG_H_ 

enum DiagType {
#define DIAG(ENUM, DESC) ENUM,
#include "diag_entries.inc"
DIAG_TYPES_NR
#undef DIAG
};

static const char* diag_descriptions[] = {
#define DIAG(ENUM, DESC) DESC,
#include "diag_entries.inc"
#undef DIAG
};

struct DiagEngine {
  struct BufferedFile *file;
};

void DiagInit(struct DiagEngine *diag, struct BufferedFile *file);
void DiagReport(struct DiagEngine* diag, const char *current_ptr, enum DiagType diag_type);

#endif
