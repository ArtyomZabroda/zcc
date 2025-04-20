#include "scanner.h"
#include "diag.h"
#include "token.h"
#include <stddef.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>
void ScannerInit(struct Scanner *scanner, const char *source, int source_size, struct DiagEngine *diag_engine) {
  scanner->source = source;
  scanner->source_size = source_size;
  scanner->diag_engine = diag_engine;
  scanner->current = source;
  scanner->start = scanner->current;
  scanner->line = 1;
  scanner->is_at_start_of_line = 0;
  scanner->pp_mode = 0;
  scanner->angled_include = 0;
}

char Advance(struct Scanner *scanner) {
  char c = *scanner->current++;
  return c;
}

void Rollback(struct Scanner *scanner, int n) {
  int i;
  for (i = 0; i < n && scanner->current != scanner->source; ++i) {
    --scanner->current;
  }
}

const char *CurPtr(struct Scanner *scanner) {
  return scanner->current;
}

struct Token MakeToken(struct Scanner *scanner, enum TokenType type) {
  struct Token tok;
  tok.type = type;
  tok.data = scanner->start;
  tok.line = scanner->line;
  tok.length = (int)(CurPtr(scanner) - scanner->start);
  return tok;
}

/* 
  C89: 6.1.1 Keywords
  C89: 6.1.2 Identifiers
*/
struct Token ScanIdentifier(struct Scanner *scanner) {
  char c = Advance(scanner);
  while (isalnum(c) || c == '_') {
    c = Advance(scanner);
  }
  Rollback(scanner, 1);
  return MakeToken(scanner, TOKEN_TYPE_IDENTIFIER);
}

/* 
  C89: 6.1.8 Preprocessing numbers 
  C89: 6.1.3.1 Floating constants
  C89: 6.1.3.2 Integer constants

  Rules for preprocessing numbers are more relaxed than for floats or ints.
  This token will be categorized more strictly on later stages.
*/
struct Token ScanNumber(struct Scanner *scanner) {
  char c = Advance(scanner);
  while (isalnum(c) || c == '_' || c == '.') {
    if (c == 'e' || c == 'E') {
      c = Advance(scanner);
      if (c == '+' || c == '-') c = Advance(scanner);
      else Rollback(scanner, 1);
    }
    c = Advance(scanner);
  }
  Rollback(scanner, 1);
  return MakeToken(scanner, TOKEN_TYPE_PPNUMBER);
}

struct Token ScanCharConst(struct Scanner *scanner) {
  char c = Advance(scanner);
  if (c == '\'') {
    DiagReport(scanner->diag_engine, CurPtr(scanner), DIAG_TYPE_EMPTY_CHAR_CONST);
    return MakeToken(scanner, TOKEN_TYPE_UNKNOWN);
  }
  while (c != '\'') {
    if (c == '\\') {
      c = Advance(scanner);
      if (c >= '0' && c < '8') {
        c = Advance(scanner);
        while (c >= '0' && c < '8') {
          c = Advance(scanner);
        }
        Rollback(scanner, 1);
      } else if (c == 'x') {
        c = Advance(scanner);
        if (!isxdigit(c)) {
          Rollback(scanner, 1);
          DiagReport(scanner->diag_engine, CurPtr(scanner),
                     DIAG_TYPE_INCORRECT_HEX_ESCAPE_SEQUENCE);
          return MakeToken(scanner, TOKEN_TYPE_UNKNOWN);
        }
        while (c >= '0' && c <= '9' && c >= 'a' && c <= 'f') {
          c = Advance(scanner);
        }
        Rollback(scanner, 1);
      } else if (c != '\'' && c != '\"' && c != '?' && c != '\\' && c != 'a' &&
                 c != 'b' && c != 'f' && c != 'n' && c != 'r' && c != 't' &&
                 c != 'v') {
        Rollback(scanner, 1);
        DiagReport(scanner->diag_engine, CurPtr(scanner),
                   DIAG_TYPE_INCORRECT_ESCAPE_SEQUENCE);
        return MakeToken(scanner, TOKEN_TYPE_UNKNOWN);
      }
    } else if (c == '\n' || c == '\0') {
      Rollback(scanner, 1);
      DiagReport(scanner->diag_engine, CurPtr(scanner), DIAG_TYPE_UNTERMINATED_CHAR_CONST);
      return MakeToken(scanner, TOKEN_TYPE_UNKNOWN);
    }
    c = Advance(scanner);
  }
  return MakeToken(scanner, TOKEN_TYPE_CHAR_CONST);
}

struct Token ScanStringLiteral(struct Scanner *scanner) {
  char c = Advance(scanner);
  char terminator = scanner->angled_include ? '>' : '\"';
  while (c != terminator) {
    if (c == '\\') {
      c = Advance(scanner);
      if (c >= '0' && c < '8') {
        c = Advance(scanner);
        while (c >= '0' && c < '8') {
          c = Advance(scanner);
        }
        Rollback(scanner, 1);
      } else if (c == 'x') {
        c = Advance(scanner);
        if (!isxdigit(c)) {
          Rollback(scanner, 1);
          DiagReport(scanner->diag_engine, CurPtr(scanner),
                     DIAG_TYPE_INCORRECT_HEX_ESCAPE_SEQUENCE);
          return MakeToken(scanner, TOKEN_TYPE_UNKNOWN);
        }
        while (c >= '0' && c <= '9' && c >= 'a' && c <= 'f') {
          c = Advance(scanner);
        }
        Rollback(scanner, 1);
      } else if (c != '\'' && c != '\"' && c != '?' && c != '\\' && c != 'a' &&
                 c != 'b' && c != 'f' && c != 'n' && c != 'r' && c != 't' &&
                 c != 'v') {
        Rollback(scanner, 1);
        DiagReport(scanner->diag_engine, CurPtr(scanner),
                   DIAG_TYPE_INCORRECT_ESCAPE_SEQUENCE);
        return MakeToken(scanner, TOKEN_TYPE_UNKNOWN);
      }
    } else if (c == '\n' || c == '\0') {
      Rollback(scanner, 1);
      DiagReport(scanner->diag_engine, CurPtr(scanner), DIAG_TYPE_UNTERMINATED_STRING_LITERAL);
      return MakeToken(scanner, TOKEN_TYPE_UNKNOWN);
    }
    c = Advance(scanner);
  }
  if (scanner->angled_include) {
    return MakeToken(scanner, TOKEN_TYPE_HEADER_NAME);
  } else {
    return MakeToken(scanner, TOKEN_TYPE_STRING_LITERAL);
  }
}

void SkipBlockComment(struct Scanner *scanner) {
  for (;;) {
    char c = Advance(scanner);
    if (c == '*') {
      c = Advance(scanner);
      if (c == '/') {
        break;
      } else Rollback(scanner, 2);
    }
    else if (c == '\n') {
      ++scanner->line;
    }

    if (CurPtr(scanner) == scanner->source + scanner->source_size) {
      DiagReport(scanner->diag_engine, CurPtr(scanner), DIAG_TYPE_UNTERMINATED_BLOCK_COMMENT);
      break;
    }
  }
  Rollback(scanner, 1);
  scanner->start = CurPtr(scanner);
}

struct Token HandleDirective(struct Scanner *scanner) {
  struct Token tok = Scan(scanner);
  if (tok.type = TOKEN_TYPE_IDENTIFIER && memcmp(tok.data, "include", 7 * sizeof(char))) {
    tok = Scan(scanner);
    if (tok.type == TOKEN_TYPE_STRING_LITERAL) {
      return tok;
    }
    else if (tok.type == TOKEN_TYPE_LESS) {
      scanner->angled_include = 1;
      tok = Scan(scanner);
      if (tok.type == TOKEN_TYPE_HEADER_NAME) {
        return tok;
      }
    }
    DiagReport(scanner->diag_engine, CurPtr(scanner), DIAG_TYPE_NO_FILE_NAME);
    return MakeToken(scanner, TOKEN_TYPE_UNKNOWN);
  }
}

struct Token Scan(struct Scanner *scanner) {
  struct Token tok;
  char c;
  int is_at_start_of_line = scanner->is_at_start_of_line;

  scanner->start = CurPtr(scanner);

  c = Advance(scanner);
  scanner->is_at_start_of_line = 0;

  for (;;) {
    switch (c) {
      case 0:
        /* Hit the end of file? */
        if (CurPtr(scanner) == scanner->source + scanner->source_size) {
          return MakeToken(scanner, TOKEN_TYPE_EOF);
        }
        else {
          DiagReport(scanner->diag_engine, CurPtr(scanner), DIAG_TYPE_NULL_IN_FILE);
          scanner->start = CurPtr(scanner);
          c = Advance(scanner);
        }
        break;
      case ' ':
      case '\r':
      case '\t':
        scanner->start = CurPtr(scanner);
        c = Advance(scanner);
        break;
      case '\n':
        ++scanner->line;
        scanner->start = CurPtr(scanner);
        c = Advance(scanner);
        scanner->pp_mode = 0;
        break;
      case 'A': case 'B': case 'C': case 'D': case 'E': case 'F': case 'G': case 'H': case 'I': case 'J': case 'K': /* case 'L':*/ case 'M':
      case 'N': case 'O': case 'P': case 'Q': case 'R': case 'S': case 'T': case 'U': case 'V': case 'W': case 'X': case 'Y': case 'Z':
      case 'a': case 'b': case 'c': case 'd': case 'e': case 'f': case 'g': case 'h': case 'i': case 'j': case 'k': case 'l': case 'm':
      case 'n': case 'o': case 'p': case 'q': case 'r': case 's': case 't': case 'u': case 'v': case 'w': case 'x': case 'y': case 'z':
      case '_':
        return ScanIdentifier(scanner);
      case '0': case '1': case '2': case '3': case '4':
      case '5': case '6': case '7': case '8': case '9':
        return ScanNumber(scanner);
      case '.':
        c = Advance(scanner);
        if (c == '.') {
          c = Advance(scanner);
          if (c == '.') {
            return MakeToken(scanner, TOKEN_TYPE_ELLIPSIS);
          } else {
            Rollback(scanner, 2);
          }
        }
        else if (isdigit(c)) {
          return ScanNumber(scanner);
        } else {
          Rollback(scanner, 1);
          return MakeToken(scanner, TOKEN_TYPE_PERIOD);
        }
      case '\'':
        return ScanCharConst(scanner);
      case 'L':
        c = Advance(scanner);
        if (c == '\'') {
          DiagReport(scanner->diag_engine, CurPtr(scanner), DIAG_TYPE_WIDE_CHAR_CONSTANTS_UNSUPPORTED);
          while (c != '\'') {
            c = Advance(scanner);
          }
          Rollback(scanner, 1);
          return MakeToken(scanner, TOKEN_TYPE_UNKNOWN);
        } else if (c == '\"') {
          DiagReport(scanner->diag_engine, CurPtr(scanner), DIAG_TYPE_WIDE_STRING_LITERALS_UNSUPPORTED);
          while (c != '\"') {
            c = Advance(scanner);
          }
          Rollback(scanner, 1);
          return MakeToken(scanner, TOKEN_TYPE_UNKNOWN);
        } else return ScanIdentifier(scanner);
        break;
      case '\"':
        return ScanStringLiteral(scanner);
      case '[':
        return MakeToken(scanner, TOKEN_TYPE_LSQUARE);
      case ']':
       return MakeToken(scanner, TOKEN_TYPE_RSQUARE);
      case '(':
        return MakeToken(scanner, TOKEN_TYPE_LPAREN);
      case ')':
        return MakeToken(scanner, TOKEN_TYPE_RPAREN);
      case '-':
        c = Advance(scanner);
        switch (c) {
          case '>':
            return MakeToken(scanner, TOKEN_TYPE_ARROW);
          case '-':
            return MakeToken(scanner, TOKEN_TYPE_MINUS_MINUS);
          case '=':
            return MakeToken(scanner, TOKEN_TYPE_MINUS_EQUAL);
          default:
            Rollback(scanner, 1);
            return MakeToken(scanner, TOKEN_TYPE_MINUS);
        }
      case '+':
        c = Advance(scanner);
        switch (c) {
          case '+':
            return MakeToken(scanner, TOKEN_TYPE_PLUS_PLUS);
          case '=':
            return MakeToken(scanner, TOKEN_TYPE_PLUS_EQUAL);
          default:
            Rollback(scanner, 1);
            return MakeToken(scanner, TOKEN_TYPE_PLUS);
        }
      case '&':
        c = Advance(scanner);
        switch (c) {
          case '&':
            return MakeToken(scanner, TOKEN_TYPE_AMP_AMP);
          case '=':
            return MakeToken(scanner, TOKEN_TYPE_AMP_EQUAL);
          default:
            Rollback(scanner, 1);
            return MakeToken(scanner, TOKEN_TYPE_AMP);
        }
      case '*':
        c = Advance(scanner);
        if (c == '=') {
          return MakeToken(scanner, TOKEN_TYPE_STAR_EQUAL);
        } else {
          Rollback(scanner, 1);
          return MakeToken(scanner, TOKEN_TYPE_STAR);
        }
      case '~':
        return MakeToken(scanner, TOKEN_TYPE_TILDE);
      case '!':
        c = Advance(scanner);
        if (c == '=') {
          return MakeToken(scanner, TOKEN_TYPE_EXCLAIM_EQUAL);
        } else {
          Rollback(scanner, 1);
          return MakeToken(scanner, TOKEN_TYPE_EXCLAIM);
        }
      case '/':
        c = Advance(scanner);
        switch (c) {
          case '*':
            SkipBlockComment(scanner);
          case '=':
            return MakeToken(scanner, TOKEN_TYPE_SLASH_EQUAL);
          default:
            Rollback(scanner, 1);
            return MakeToken(scanner, TOKEN_TYPE_SLASH);
        }
      case '%':
        c = Advance(scanner);
        if (c == '=') {
          return MakeToken(scanner, TOKEN_TYPE_PERCENT_EQUAL);
        } else {
          Rollback(scanner, 1);
          return MakeToken(scanner, TOKEN_TYPE_PERCENT);
        }
      case '<':
        c = Advance(scanner);
        switch (c) {
          case '<':
            c = Advance(scanner);
            if (c == '=') {
              return MakeToken(scanner, TOKEN_TYPE_LESS_LESS_EQUAL);
            } else {
              Rollback(scanner, 1);
              return MakeToken(scanner, TOKEN_TYPE_LESS_LESS);
            }
          case '=':
            return MakeToken(scanner, TOKEN_TYPE_LESS_EQUAL);
          default:
            return MakeToken(scanner, TOKEN_TYPE_LESS);
        }
        case '>':
        c = Advance(scanner);
        switch (c) {
          case '>':
            c = Advance(scanner);
            if (c == '=') {
              return MakeToken(scanner, TOKEN_TYPE_GREATER_GREATER_EQUAL);
            } else {
              Rollback(scanner, 1);
              return MakeToken(scanner, TOKEN_TYPE_GREATER_GREATER);
            }
          case '=':
            return MakeToken(scanner, TOKEN_TYPE_GREATER_EQUAL);
          default:
            return MakeToken(scanner, TOKEN_TYPE_GREATER);
        }
      case '=':
        c = Advance(scanner);
        if (c == '=') {
          return MakeToken(scanner, TOKEN_TYPE_EQUAL_EQUAL);
        } else {
          Rollback(scanner, 1);
          return MakeToken(scanner, TOKEN_TYPE_EQUAL);
        }
      case '^':
        c = Advance(scanner);
        if (c == '=') {
          return MakeToken(scanner, TOKEN_TYPE_CARET_EQUAL);
        } else {
          Rollback(scanner, 1);
          return MakeToken(scanner, TOKEN_TYPE_CARET);
        }
        case '|':
        c = Advance(scanner);
        switch (c) {
          case '|':
            return MakeToken(scanner, TOKEN_TYPE_PIPE_PIPE);
          case '=':
            return MakeToken(scanner, TOKEN_TYPE_PIPE_EQUAL);
          default:
            Rollback(scanner, 1);
            return MakeToken(scanner, TOKEN_TYPE_PIPE);
        }
      case '?':
        return MakeToken(scanner, TOKEN_TYPE_QUESTION);
      case ':':
        return MakeToken(scanner, TOKEN_TYPE_COLON);
      case ',':
        return MakeToken(scanner, TOKEN_TYPE_COMMA);
      case '#':
        /* Check if this is preprocessor directive */
        if (is_at_start_of_line) {
          scanner->pp_mode = 1;
        }
        c = Advance(scanner);
        if (c == '#') {
          return MakeToken(scanner, TOKEN_TYPE_HASH_HASH);
        } else {
          Rollback(scanner, 1);
          return MakeToken(scanner, TOKEN_TYPE_HASH);
        }
      case '{':
        return MakeToken(scanner, TOKEN_TYPE_LBRACE);
      case '}':
        return MakeToken(scanner, TOKEN_TYPE_RBRACE);
      case ';':
        return MakeToken(scanner, TOKEN_TYPE_SEMICOLON);
      default:
        return MakeToken(scanner, TOKEN_TYPE_UNKNOWN);
    }
  }
}

void PrintScannerOutput(struct Scanner *scanner) {
  struct Token tok;
  int line = -1;
  printf("line\ttype\tvalue\n");
  while ((tok = Scan(scanner)).type != TOKEN_TYPE_EOF) {
    if (line != tok.line) {
      line = tok.line;
      printf("%d", tok.line);
    }
    else putc('|', stdout);
    putc('\t', stdout);
    printf("%d\t", tok.type);
    printf("%.*s\t\n", tok.length, tok.data);
  }
}
