#ifndef COMMON_H_
#define COMMON_H_

struct BufferedFile {
  const char *file_path;
  char *buffer;
  const char *current;
  int buf_size;
};

void BufferedFileInit(struct BufferedFile *bfile, const char *file_path);
void BufferedFileFree(struct BufferedFile *bfile);
char BufferedFileGetChar(struct BufferedFile *bfile);
void BufferedFileRollBack(struct BufferedFile *bfile, int n);

#endif
