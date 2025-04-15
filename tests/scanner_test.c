#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <stdlib.h>
#include <cmocka.h>
#include "diag.h"
#include "scanner.h"
#include "token.h"

void __wrap_DiagReport(struct DiagEngine* diag, const char *current_ptr, enum DiagType diag_type) {
  function_called();
  check_expected(diag_type);
}

struct TestState {
  struct Scanner scanner;
  struct DiagEngine diag;
};

static int ScannerGroupSetup (void** state) {
  struct TestState *teststate = calloc(1, sizeof(struct TestState));    
  *state = teststate;
  return 0;
}

static int ScannerGroupTeardown (void** state) {
  struct TestState *teststate = *state;
  free(teststate);
  return 0;
}

void TestValidKeywords(void **state) {
  struct TestState *teststate = *state;

  const char source[] = 
    "auto break case char const continue default do"
    "double else enum extern float for goto if"
    "int long register return short signed sizeof static"
    "struct switch typedef union unsigned void volatile while";
  int i;
  struct Token tok;

  ScannerInit(&teststate->scanner, source, sizeof(source), &teststate->diag);
  /* we don't distinguish between identifiers and keyword on the scanning stage */
  while ((tok = Scan(&teststate->scanner)).type != TOKEN_TYPE_EOF) {
    assert_int_equal(tok.type, TOKEN_TYPE_IDENTIFIER);
  }
}

void TestValidIdentifiers(void **state) {
  struct TestState *teststate = *state;
  const char source[] = 
    "Hello _hello he110";
    
  int i;
  struct Token tok;

  ScannerInit(&teststate->scanner, source, sizeof(source), &teststate->diag);

  while ((tok = Scan(&teststate->scanner)).type != TOKEN_TYPE_EOF) {
    assert_int_equal(tok.type, TOKEN_TYPE_IDENTIFIER);
  }
}

void TestValidPPNumbers(void **state) {
  struct TestState *teststate = *state;
  const char source[] = 
    "1 1.2 1.3f 1.4.5 1abcde 1e+1 1e-2 1E-3 1E+4 .123 .1e+1";

  int i;
  struct Token tok;

  ScannerInit(&teststate->scanner, source, sizeof(source), &teststate->diag);

  while ((tok = Scan(&teststate->scanner)).type != TOKEN_TYPE_EOF) {
    assert_int_equal(tok.type, TOKEN_TYPE_PPNUMBER);
  }
}

void TestValidCharConsts(void **state) {
  struct TestState *teststate = *state;
  const char source[] = 
    "'a' 'ab' '\\'' '\\\"' '\\?' '\\\\' '\\a' '\\b' '\\f' '\\n' '\\r' '\\t' '\\v' '\\1' '\\12' '\\123' '\\x1' '\\x12'";
    
  int i;
  struct Token tok;
  ScannerInit(&teststate->scanner, source, sizeof(source), &teststate->diag);

  while ((tok = Scan(&teststate->scanner)).type != TOKEN_TYPE_EOF) {
    assert_int_equal(tok.type, TOKEN_TYPE_CHAR_CONST);
  }
}

void TestUnterminatedCharConst(void **state) {
  struct TestState *teststate = *state;
  const char source[] = "'";
  
  int i;
  struct Token tok;

  expect_function_call(__wrap_DiagReport);
  expect_value(__wrap_DiagReport, diag_type, DIAG_TYPE_UNTERMINATED_CHAR_CONST);
  ScannerInit(&teststate->scanner, source, sizeof(source), &teststate->diag);
  tok = Scan(&teststate->scanner);
  assert_int_equal(tok.type, TOKEN_TYPE_UNKNOWN);
}

int main(void) {
  const struct CMUnitTest scanner_tests[] = {
      cmocka_unit_test(TestValidKeywords),
      cmocka_unit_test(TestValidIdentifiers),
      cmocka_unit_test(TestValidPPNumbers),
      cmocka_unit_test(TestValidCharConsts),
      cmocka_unit_test(TestUnterminatedCharConst),
  };
  return cmocka_run_group_tests(scanner_tests, ScannerGroupSetup, ScannerGroupTeardown);
}
