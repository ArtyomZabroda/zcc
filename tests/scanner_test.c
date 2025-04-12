#include "token.h"
#include "unity.h"
#include "scanner.h"
#include <stdio.h>



void setUp(void) {
  
}

void tearDown(void) {
}

void test_Keywords(void) {
  const char source[] = 
    "auto break case char const continue default do"
    "double else enum extern float for goto if"
    "int long register return short signed sizeof static"
    "struct switch typedef union unsigned void volatile while";

  struct Scanner scanner;
  int i;
  struct Token tok;

  ScannerInit(&scanner, source, sizeof(source), NULL);

  /* we don't distinguish between identifiers and keyword on the scanning stage */
  while ((tok = Scan(&scanner)).type != TOKEN_TYPE_EOF) {
    TEST_ASSERT_EQUAL_INT(tok.type, TOKEN_TYPE_IDENTIFIER);
  }
}

void test_Identifiers(void) {
  const char source[] = 
    "Hello _hello he110";

  struct Scanner scanner;
  int i;
  struct Token tok;

  ScannerInit(&scanner, source, sizeof(source), NULL);

  while ((tok = Scan(&scanner)).type != TOKEN_TYPE_EOF) {
    TEST_ASSERT_EQUAL_INT(tok.type, TOKEN_TYPE_IDENTIFIER);
  }
}

void test_PPNumbers(void) {
  const char source[] = 
    "1 1.2 1.3f 1.4.5 1abcde 1e+1 1e-2 1E-3 1E+4 .123 .1e+1";

  struct Scanner scanner;
  int i;
  struct Token tok;

  ScannerInit(&scanner, source, sizeof(source), NULL);

  while ((tok = Scan(&scanner)).type != TOKEN_TYPE_EOF) {
    TEST_ASSERT_EQUAL_INT(tok.type, TOKEN_TYPE_PPNUMBER);
  }
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_Keywords);
    RUN_TEST(test_Identifiers);
    RUN_TEST(test_PPNumbers);
    return UNITY_END();
}
