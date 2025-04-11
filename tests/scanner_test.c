#include "buffered_file.h"
#include "diag.h"
#include "token.h"
#include "unity/unity.h"
#include "scanner.h"
#include <stdio.h>



void setUp(void) {
  
}

void tearDown(void) {
}

void OpenTestFile(const char *file_path, struct BufferedFile *file, struct DiagEngine *diag, struct Scanner *scanner) {
  BufferedFileInit(file, file_path);
  DiagInit(diag, file);
  ScannerInit(scanner, file, diag);
}

void CloseTestFile(struct BufferedFile *file, struct Scanner *scanner) {
  ScannerFree(scanner);
  BufferedFileFree(file);
}

void test_Keywords(void) {
  struct BufferedFile file;
  struct Scanner scanner;
  struct DiagEngine diag;
  int i;
  struct Token tok;

  OpenTestFile("tests/extra/scanner_keywords.txt", &file, &diag, &scanner);

  while ((tok = Scan(&scanner)).type != TOKEN_TYPE_EOF) {
    TEST_ASSERT_EQUAL_INT(tok.type, TOKEN_TYPE_IDENTIFIER);
  }
  
  CloseTestFile(&file, &scanner);
}

void test_Identifiers(void) {
  struct BufferedFile file;
  struct Scanner scanner;
  struct DiagEngine diag;
  int i;
  struct Token tok;

  OpenTestFile("tests/extra/scanner_identifiers.txt", &file, &diag, &scanner);

  while ((tok = Scan(&scanner)).type != TOKEN_TYPE_EOF) {
    TEST_ASSERT_EQUAL_INT(tok.type, TOKEN_TYPE_IDENTIFIER);
  }
  
  CloseTestFile(&file, &scanner);
}

void test_PPNumbers(void) {
  struct BufferedFile file;
  struct Scanner scanner;
  struct DiagEngine diag;
  int i;
  struct Token tok;

  OpenTestFile("tests/extra/scanner_numbers.txt", &file, &diag, &scanner);

  while ((tok = Scan(&scanner)).type != TOKEN_TYPE_EOF) {
    printf("%s\t%d\n", tok.data, tok.type);
    TEST_ASSERT_EQUAL_INT(tok.type, TOKEN_TYPE_NUMERIC_CONSTANT);
  }
  
  CloseTestFile(&file, &scanner);
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_Keywords);
    RUN_TEST(test_Identifiers);
    RUN_TEST(test_PPNumbers);
    return UNITY_END();
}
