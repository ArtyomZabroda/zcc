#include "buffered_file.h"
#include "stdio.h"
#include "stdlib.h"
#include "crash.h"

int CalculateCharAmount(FILE *file) {
  int char_amount = 0;
  while (fgetc(file) != EOF) {
    ++char_amount;
  }
  if (ferror(file)) {
    return -1;
  }
  rewind(file);
  return char_amount;
}

void BufferedFileInit(struct BufferedFile *bfile, const char *file_path) {
  FILE *file;
  int char_amount;

  bfile->file_path = file_path;
  
  file = fopen(file_path, "r");
  if (file == NULL) {
    Crash(EXIT_FAILURE, "cannot open %s", file_path);
  }

  char_amount = CalculateCharAmount(file);

  if (char_amount == -1) {
    Crash(EXIT_FAILURE, "cannot read %s", file_path);
  }

  /* Allocate 1 extra char for a null terminator */
  bfile->buf_size = char_amount + 1;
  bfile->buffer = malloc(sizeof(char) * (bfile->buf_size));

  fread(bfile->buffer, sizeof(char), char_amount, file);
  bfile->buffer[char_amount] = '\0';

  bfile->current = bfile->buffer;
  fclose(file);
}

void BufferedFileFree(struct BufferedFile *bfile) {
  free(bfile->buffer);
  bfile->buffer = NULL;
  bfile->current = NULL;
  bfile->file_path = "";
}

char BufferedFileGetChar(struct BufferedFile *bfile) {
  char c = *bfile->current++;
  return c;
}

void BufferedFileRollBack(struct BufferedFile *bfile, int n) {
  int i;
  for (i = 0; i < n && bfile->current != bfile->buffer; ++i) {
    --bfile->current;
  }
}
