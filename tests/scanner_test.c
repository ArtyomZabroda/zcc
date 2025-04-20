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
  
  struct Token tok;

  expect_function_call(__wrap_DiagReport);
  expect_value(__wrap_DiagReport, diag_type, DIAG_TYPE_UNTERMINATED_CHAR_CONST);
  ScannerInit(&teststate->scanner, source, sizeof(source), &teststate->diag);
  tok = Scan(&teststate->scanner);
  assert_int_equal(tok.type, TOKEN_TYPE_UNKNOWN);
}

void TestSingleBackslashCharConst(void **state) {
  struct TestState *teststate = *state;
  const char source[] = "'\\'";
  struct Token tok;

  expect_function_call(__wrap_DiagReport);
  expect_value(__wrap_DiagReport, diag_type, DIAG_TYPE_UNTERMINATED_CHAR_CONST);
  ScannerInit(&teststate->scanner, source, sizeof(source), &teststate->diag);
  tok = Scan(&teststate->scanner);
  assert_int_equal(tok.type, TOKEN_TYPE_UNKNOWN);
}

void TestEmptyCharConst(void **state) {
  struct TestState *teststate = *state;
  const char source[] = "''";
  struct Token tok;

  expect_function_call(__wrap_DiagReport);
  expect_value(__wrap_DiagReport, diag_type, DIAG_TYPE_EMPTY_CHAR_CONST);
  ScannerInit(&teststate->scanner, source, sizeof(source), &teststate->diag);
  tok = Scan(&teststate->scanner);
  assert_int_equal(tok.type, TOKEN_TYPE_UNKNOWN);
}

void TestInvalidEscapeSequenceInCharConst(void **state) {
  struct TestState *teststate = *state;
  const char source[] = "'\\w'";
  struct Token tok;
  expect_function_call(__wrap_DiagReport);
  expect_value(__wrap_DiagReport, diag_type, DIAG_TYPE_INCORRECT_ESCAPE_SEQUENCE);
  ScannerInit(&teststate->scanner, source, sizeof(source), &teststate->diag); 
  tok = Scan(&teststate->scanner);
  assert_int_equal(tok.type, TOKEN_TYPE_UNKNOWN);
}

void TestInvalidHexEscapeSequenceInCharConst(void **state) {
  struct TestState *teststate = *state;
  const char source[] = "'\\x'";
  struct Token tok;
  expect_function_call(__wrap_DiagReport);
  expect_value(__wrap_DiagReport, diag_type, DIAG_TYPE_INCORRECT_HEX_ESCAPE_SEQUENCE);
  ScannerInit(&teststate->scanner, source, sizeof(source), &teststate->diag); 
  tok = Scan(&teststate->scanner);
  assert_int_equal(tok.type, TOKEN_TYPE_UNKNOWN);
}

void TestWideCharUnsupportedInCharConst(void **state) {
  struct TestState *teststate = *state;
  const char source[] = "L''";
  struct Token tok;
  expect_function_call(__wrap_DiagReport);
  expect_value(__wrap_DiagReport, diag_type, DIAG_TYPE_WIDE_CHAR_CONSTANTS_UNSUPPORTED);
  ScannerInit(&teststate->scanner, source, sizeof(source), &teststate->diag); 
  tok = Scan(&teststate->scanner);
  assert_int_equal(tok.type, TOKEN_TYPE_UNKNOWN);
}

void TestIdentifierThatStartsWithL(void **state) {
  struct TestState *teststate = *state;
  const char source[] = "La'";
  struct Token tok;
  ScannerInit(&teststate->scanner, source, sizeof(source), &teststate->diag); 
  tok = Scan(&teststate->scanner);
  assert_int_equal(tok.type, TOKEN_TYPE_IDENTIFIER);
}

void TestNullByteInFileIsIgnoredWithWarning(void **state) {
  struct TestState *teststate = *state;
  const char source[] = "abc\0def";
  struct Token tok;
  expect_function_call(__wrap_DiagReport);
  expect_value(__wrap_DiagReport, diag_type, DIAG_TYPE_NULL_IN_FILE);
  ScannerInit(&teststate->scanner, source, sizeof(source), &teststate->diag); 

  tok = Scan(&teststate->scanner);
  assert_int_equal(tok.type, TOKEN_TYPE_IDENTIFIER);
  tok = Scan(&teststate->scanner);
  assert_int_equal(tok.type, TOKEN_TYPE_IDENTIFIER);
}

void TestNullByteInCharConst(void **state) {
  struct TestState *teststate = *state;
  const char source[] = "\'\0\'";
  struct Token tok;
  expect_function_call(__wrap_DiagReport);
  expect_value(__wrap_DiagReport, diag_type, DIAG_TYPE_UNTERMINATED_CHAR_CONST);

  ScannerInit(&teststate->scanner, source, sizeof(source), &teststate->diag); 

  tok = Scan(&teststate->scanner);
  assert_int_equal(tok.type, TOKEN_TYPE_UNKNOWN);
  expect_function_calls(__wrap_DiagReport, 2);
  expect_value(__wrap_DiagReport, diag_type, DIAG_TYPE_NULL_IN_FILE);
  expect_value(__wrap_DiagReport, diag_type, DIAG_TYPE_UNTERMINATED_CHAR_CONST);
  tok = Scan(&teststate->scanner);
  assert_int_equal(tok.type, TOKEN_TYPE_UNKNOWN);
}

void TestNullByteInStringLiteral(void **state) {
  struct TestState *teststate = *state;
  const char source[] = "\"\0\"";
  struct Token tok;
  expect_function_call(__wrap_DiagReport);
  expect_value(__wrap_DiagReport, diag_type, DIAG_TYPE_UNTERMINATED_STRING_LITERAL);

  ScannerInit(&teststate->scanner, source, sizeof(source), &teststate->diag); 

  tok = Scan(&teststate->scanner);
  assert_int_equal(tok.type, TOKEN_TYPE_UNKNOWN);
  expect_function_calls(__wrap_DiagReport, 2);
  expect_value(__wrap_DiagReport, diag_type, DIAG_TYPE_NULL_IN_FILE);
  expect_value(__wrap_DiagReport, diag_type, DIAG_TYPE_UNTERMINATED_STRING_LITERAL);
  tok = Scan(&teststate->scanner);
  assert_int_equal(tok.type, TOKEN_TYPE_UNKNOWN);
}

void TestValidStringLiterals(void **state) {
  struct TestState *teststate = *state;
  const char source[] = "\"\" \"abc\" \"\\'\" \"\\\"\" \"\\\\\" \"\\a\" \"\\b\" \"\\f\" \"\\n\" \"\\r\" \"\\t\" \"\\v\" \"\\x1\" \"\\x12\" \"\\0\" \"'\"";
  struct Token tok;
  ScannerInit(&teststate->scanner, source, sizeof(source), &teststate->diag); 
  tok = Scan(&teststate->scanner);
  while ((tok = Scan(&teststate->scanner)).type != TOKEN_TYPE_EOF) {
    assert_int_equal(tok.type, TOKEN_TYPE_STRING_LITERAL);
  }
}

void TestSingleBackSlashInStringLiteral(void **state) {
  struct TestState *teststate = *state;
  const char source[] = "\"\\\"";
  struct Token tok;
  expect_function_call(__wrap_DiagReport);
  expect_value(__wrap_DiagReport, diag_type, DIAG_TYPE_UNTERMINATED_STRING_LITERAL);
  ScannerInit(&teststate->scanner, source, sizeof(source), &teststate->diag); 
  tok = Scan(&teststate->scanner);
  assert_int_equal(tok.type, TOKEN_TYPE_UNKNOWN);
}

void TestInvalidHexEscapeSequenceInStringLiteral(void **state) {
  struct TestState *teststate = *state;
  const char source[] = "\"\\x\"";
  struct Token tok;
  expect_function_call(__wrap_DiagReport);
  expect_value(__wrap_DiagReport, diag_type, DIAG_TYPE_INCORRECT_HEX_ESCAPE_SEQUENCE);
  ScannerInit(&teststate->scanner, source, sizeof(source), &teststate->diag); 
  tok = Scan(&teststate->scanner);
  assert_int_equal(tok.type, TOKEN_TYPE_UNKNOWN);
}

void TestInvalidEscapeSequenceInStringLiteral(void **state) {
  struct TestState *teststate = *state;
  const char source[] = "\"\\w\"";
  struct Token tok;
  expect_function_call(__wrap_DiagReport);
  expect_value(__wrap_DiagReport, diag_type, DIAG_TYPE_INCORRECT_ESCAPE_SEQUENCE);
  ScannerInit(&teststate->scanner, source, sizeof(source), &teststate->diag); 
  tok = Scan(&teststate->scanner);
  assert_int_equal(tok.type, TOKEN_TYPE_UNKNOWN);
}

void TestWideCharUnsupportedStringLiteral(void **state) {
  struct TestState *teststate = *state;
  const char source[] = "L\"abc\"";
  struct Token tok;
  expect_function_call(__wrap_DiagReport);
  expect_value(__wrap_DiagReport, diag_type, DIAG_TYPE_WIDE_STRING_LITERALS_UNSUPPORTED);
  ScannerInit(&teststate->scanner, source, sizeof(source), &teststate->diag); 
  tok = Scan(&teststate->scanner);
  assert_int_equal(tok.type, TOKEN_TYPE_UNKNOWN);
}

void TestValidOperators(void **state) {
  struct TestState *teststate = *state;
  const char source[] = 
  "[ ] ( ) . ->"
  "++ -- & * + - ~ ! sizeof"
  "/ % << >> < > <= >= == != ^ | && || ? :"
  "= *= /= %= += -= <<= >>= &= ^= |= , # ##";

  int i;
  struct Token tok;
  ScannerInit(&teststate->scanner, source, sizeof(source), &teststate->diag);

  tok = Scan(&teststate->scanner);
  assert_int_equal(tok.type, TOKEN_TYPE_LSQUARE);
  tok = Scan(&teststate->scanner);
  assert_int_equal(tok.type, TOKEN_TYPE_RSQUARE);
  tok = Scan(&teststate->scanner);
  assert_int_equal(tok.type, TOKEN_TYPE_LPAREN);
  tok = Scan(&teststate->scanner);
  assert_int_equal(tok.type, TOKEN_TYPE_RPAREN);
  tok = Scan(&teststate->scanner);
  assert_int_equal(tok.type, TOKEN_TYPE_PERIOD);
  tok = Scan(&teststate->scanner);
  assert_int_equal(tok.type, TOKEN_TYPE_ARROW);
  tok = Scan(&teststate->scanner);
  assert_int_equal(tok.type, TOKEN_TYPE_PLUS_PLUS);
  tok = Scan(&teststate->scanner);
  assert_int_equal(tok.type, TOKEN_TYPE_MINUS_MINUS);
  tok = Scan(&teststate->scanner);
  assert_int_equal(tok.type, TOKEN_TYPE_AMP);
  tok = Scan(&teststate->scanner);
  assert_int_equal(tok.type, TOKEN_TYPE_STAR);
  tok = Scan(&teststate->scanner);
  assert_int_equal(tok.type, TOKEN_TYPE_PLUS);
  tok = Scan(&teststate->scanner);
  assert_int_equal(tok.type, TOKEN_TYPE_MINUS);
  tok = Scan(&teststate->scanner);
  assert_int_equal(tok.type, TOKEN_TYPE_TILDE);
  tok = Scan(&teststate->scanner);
  assert_int_equal(tok.type, TOKEN_TYPE_EXCLAIM);
  tok = Scan(&teststate->scanner);
  assert_int_equal(tok.type, TOKEN_TYPE_IDENTIFIER); /* treat sizeof as keyword */ 
  tok = Scan(&teststate->scanner);
  assert_int_equal(tok.type, TOKEN_TYPE_SLASH);
  tok = Scan(&teststate->scanner);
  assert_int_equal(tok.type, TOKEN_TYPE_PERCENT);
  tok = Scan(&teststate->scanner);
  assert_int_equal(tok.type, TOKEN_TYPE_LESS_LESS);
  tok = Scan(&teststate->scanner);
  assert_int_equal(tok.type, TOKEN_TYPE_GREATER_GREATER);
  tok = Scan(&teststate->scanner);
  assert_int_equal(tok.type, TOKEN_TYPE_LESS);
  tok = Scan(&teststate->scanner);
  assert_int_equal(tok.type, TOKEN_TYPE_GREATER);
  tok = Scan(&teststate->scanner);
  assert_int_equal(tok.type, TOKEN_TYPE_LESS_EQUAL);
  tok = Scan(&teststate->scanner);
  assert_int_equal(tok.type, TOKEN_TYPE_GREATER_EQUAL);
  tok = Scan(&teststate->scanner);
  assert_int_equal(tok.type, TOKEN_TYPE_EQUAL_EQUAL);
  tok = Scan(&teststate->scanner);
  assert_int_equal(tok.type, TOKEN_TYPE_EXCLAIM_EQUAL);
  tok = Scan(&teststate->scanner);
  assert_int_equal(tok.type, TOKEN_TYPE_CARET);
  tok = Scan(&teststate->scanner);
  assert_int_equal(tok.type, TOKEN_TYPE_PIPE);
  tok = Scan(&teststate->scanner);
  assert_int_equal(tok.type, TOKEN_TYPE_AMP_AMP);
  tok = Scan(&teststate->scanner);
  assert_int_equal(tok.type, TOKEN_TYPE_PIPE_PIPE);
  tok = Scan(&teststate->scanner);
  assert_int_equal(tok.type, TOKEN_TYPE_QUESTION);
  tok = Scan(&teststate->scanner);
  assert_int_equal(tok.type, TOKEN_TYPE_COLON);
  tok = Scan(&teststate->scanner);
  assert_int_equal(tok.type, TOKEN_TYPE_EQUAL);
  tok = Scan(&teststate->scanner);
  assert_int_equal(tok.type, TOKEN_TYPE_STAR_EQUAL);
  tok = Scan(&teststate->scanner);
  assert_int_equal(tok.type, TOKEN_TYPE_SLASH_EQUAL);
  tok = Scan(&teststate->scanner);
  assert_int_equal(tok.type, TOKEN_TYPE_PERCENT_EQUAL);
  tok = Scan(&teststate->scanner);
  assert_int_equal(tok.type, TOKEN_TYPE_PLUS_EQUAL);
  tok = Scan(&teststate->scanner);
  assert_int_equal(tok.type, TOKEN_TYPE_MINUS_EQUAL);
  tok = Scan(&teststate->scanner);
  assert_int_equal(tok.type, TOKEN_TYPE_LESS_LESS_EQUAL);
  tok = Scan(&teststate->scanner);
  assert_int_equal(tok.type, TOKEN_TYPE_GREATER_GREATER_EQUAL);
  tok = Scan(&teststate->scanner);
  assert_int_equal(tok.type, TOKEN_TYPE_AMP_EQUAL);
  tok = Scan(&teststate->scanner);
  assert_int_equal(tok.type, TOKEN_TYPE_CARET_EQUAL);
  tok = Scan(&teststate->scanner);
  assert_int_equal(tok.type, TOKEN_TYPE_PIPE_EQUAL);
  tok = Scan(&teststate->scanner);
  assert_int_equal(tok.type, TOKEN_TYPE_COMMA);
  tok = Scan(&teststate->scanner);
  assert_int_equal(tok.type, TOKEN_TYPE_HASH);
  tok = Scan(&teststate->scanner);
  assert_int_equal(tok.type, TOKEN_TYPE_HASH_HASH);
  tok = Scan(&teststate->scanner);
  assert_int_equal(tok.type, TOKEN_TYPE_EOF);
}

void TestValidPunctuators(void **state) {
  struct TestState *teststate = *state;
  const char source[] = "[ ] ( ) { } * , : = ; ... #";
  int i;
  struct Token tok;
  ScannerInit(&teststate->scanner, source, sizeof(source), &teststate->diag);
  tok = Scan(&teststate->scanner);
  assert_int_equal(tok.type, TOKEN_TYPE_LSQUARE);
  tok = Scan(&teststate->scanner);
  assert_int_equal(tok.type, TOKEN_TYPE_RSQUARE);
  tok = Scan(&teststate->scanner);
  assert_int_equal(tok.type, TOKEN_TYPE_LPAREN);
  tok = Scan(&teststate->scanner);
  assert_int_equal(tok.type, TOKEN_TYPE_RPAREN);
  tok = Scan(&teststate->scanner);
  assert_int_equal(tok.type, TOKEN_TYPE_LBRACE);
  tok = Scan(&teststate->scanner);
  assert_int_equal(tok.type, TOKEN_TYPE_RBRACE);
  tok = Scan(&teststate->scanner);
  assert_int_equal(tok.type, TOKEN_TYPE_STAR);
  tok = Scan(&teststate->scanner);
  assert_int_equal(tok.type, TOKEN_TYPE_COMMA);
  tok = Scan(&teststate->scanner);
  assert_int_equal(tok.type, TOKEN_TYPE_COLON);
  tok = Scan(&teststate->scanner);
  assert_int_equal(tok.type, TOKEN_TYPE_EQUAL);
  tok = Scan(&teststate->scanner);
  assert_int_equal(tok.type, TOKEN_TYPE_SEMICOLON);
  tok = Scan(&teststate->scanner);
  assert_int_equal(tok.type, TOKEN_TYPE_ELLIPSIS);
  tok = Scan(&teststate->scanner);
  assert_int_equal(tok.type, TOKEN_TYPE_HASH);
  tok = Scan(&teststate->scanner);
  assert_int_equal(tok.type, TOKEN_TYPE_EOF);
}

void TestValidComments(void **state) {
  struct TestState *teststate = *state;
  const char source[] = "/*Hello, world!*/i"; /* notice identifier i after comment */
  char expected[] = "i";
  struct Token tok;

  ScannerInit(&teststate->scanner, source, sizeof(source), &teststate->diag);
  tok = Scan(&teststate->scanner);

  /* check that the comment was skipped */
  assert_int_equal(tok.type, TOKEN_TYPE_IDENTIFIER);
  assert_int_equal(tok.length, 1);
  assert_memory_equal(tok.data, expected, 1);
}

void TestCommentInStringIsNotSkipped(void **state) {
  struct TestState *teststate = *state;
  const char source[] = "\"/*hello*/\"";
  struct Token tok;

  ScannerInit(&teststate->scanner, source, sizeof(source), &teststate->diag);
  tok = Scan(&teststate->scanner);
  /* check that the "comment" is not skipped and used by the string literal */
  assert_int_equal(tok.type, TOKEN_TYPE_IDENTIFIER);
  assert_int_equal(tok.length, 12);
  assert_memory_equal(tok.data, source, 12);
}

void TestValidHeaderNames(void **state) {
  struct TestState *teststate = *state;
  const char source[] = "#include <stdio.h>\n#include \"hello.h\"";
  int i;
  struct Token tok;

  ScannerInit(&teststate->scanner, source, sizeof(source), &teststate->diag);
  tok = Scan(&teststate->scanner);
  assert_int_equal(tok.type, TOKEN_TYPE_HASH);
  tok = Scan(&teststate->scanner);
  assert_int_equal(tok.type, TOKEN_TYPE_IDENTIFIER);
  assert_memory_equal(tok.data, "include", 7);
  tok = Scan(&teststate->scanner);
  assert_int_equal(tok.type, TOKEN_TYPE_HEADER_NAME);
  assert_memory_equal(tok.data, "<stdio.h>", 9);
  tok = Scan(&teststate->scanner);
  assert_int_equal(tok.type, TOKEN_TYPE_HASH);
  tok = Scan(&teststate->scanner);
  assert_int_equal(tok.type, TOKEN_TYPE_IDENTIFIER);
  assert_memory_equal(tok.data, "include", 7);
  tok = Scan(&teststate->scanner);
  assert_int_equal(tok.type, TOKEN_TYPE_IDENTIFIER);
  assert_memory_equal(tok.data, "\"hello.h\"", 9);
}

int main(void) {
  const struct CMUnitTest scanner_tests[] = {
      cmocka_unit_test(TestValidKeywords),
      cmocka_unit_test(TestValidIdentifiers),
      cmocka_unit_test(TestValidPPNumbers),
      cmocka_unit_test(TestValidCharConsts),
      cmocka_unit_test(TestUnterminatedCharConst),
      cmocka_unit_test(TestSingleBackslashCharConst),
      cmocka_unit_test(TestEmptyCharConst),
      cmocka_unit_test(TestInvalidEscapeSequenceInCharConst),
      cmocka_unit_test(TestInvalidHexEscapeSequenceInCharConst),
      cmocka_unit_test(TestWideCharUnsupportedInCharConst),
      cmocka_unit_test(TestIdentifierThatStartsWithL),
      cmocka_unit_test(TestNullByteInFileIsIgnoredWithWarning),
      cmocka_unit_test(TestNullByteInCharConst),
      cmocka_unit_test(TestNullByteInStringLiteral),
      cmocka_unit_test(TestValidStringLiterals),
      cmocka_unit_test(TestSingleBackSlashInStringLiteral),
      cmocka_unit_test(TestInvalidHexEscapeSequenceInStringLiteral),
      cmocka_unit_test(TestInvalidEscapeSequenceInStringLiteral),
      cmocka_unit_test(TestWideCharUnsupportedStringLiteral),
      cmocka_unit_test(TestValidOperators),
      cmocka_unit_test(TestValidPunctuators),
      cmocka_unit_test(TestValidComments),
      cmocka_unit_test(TestCommentInStringIsNotSkipped),
      cmocka_unit_test(TestValidHeaderNames)
  };
  return cmocka_run_group_tests(scanner_tests, ScannerGroupSetup, ScannerGroupTeardown);
}
