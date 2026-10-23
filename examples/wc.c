#include <ctype.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SEARGS_IMPLEMENTATION
#include "../src/seargs.h"

static const seargs_arg_def_t defs[] = {
    SEARGS_OPTIONAL_BOOL_ARG("lines", 'l', "count lines", false),
    SEARGS_OPTIONAL_BOOL_ARG("words", 'w', "count words", false),
    SEARGS_OPTIONAL_BOOL_ARG("bytes", 'c', "count bytes", false),
    SEARGS_POSITIONAL_OPTIONAL_STRING_ARG("file", "input file (stdin when absent)", NULL),
};

typedef struct {
  uintmax_t lines;
  uintmax_t words;
  uintmax_t bytes;
} counts_t;

static counts_t count_stream(FILE *f) {
  counts_t c = {0, 0, 0};
  bool in_word = false;
  int ch;
  while ((ch = fgetc(f)) != EOF) {
    c.bytes++;
    if (ch == '\n')
      c.lines++;
    if (isspace((unsigned char)ch)) {
      in_word = false;
    } else if (!in_word) {
      in_word = true;
      c.words++;
    }
  }
  return c;
}

int main(int argc, char *const argv[]) {
  // Real wc reads stdin when given no arguments, so opt out of the automatic
  // argc == 1 help listing.
  seargs_parser_t *p =
      seargs_parse_args_ex(argc, argv, defs, sizeof(defs) / sizeof(defs[0]), false);
  if (!p) {
    fprintf(stderr, "wc: out of memory\n");
    return EXIT_FAILURE;
  }
  if (p->error.code != SEARGS_OK) {
    fprintf(stderr, "wc: ");
    if (p->error.arg)
      fprintf(stderr, "'%s' ", p->error.arg);
    fprintf(stderr, "%s\n", p->error.detail ? p->error.detail : "bad arguments");
    seargs_free(p);
    return EXIT_FAILURE;
  }

  bool show_lines = seargs_get_bool(p, "lines");
  bool show_words = seargs_get_bool(p, "words");
  bool show_bytes = seargs_get_bool(p, "bytes");
  const char *fname = seargs_get_string(p, "file");

  // With no flags, report everything, like the real wc.
  if (!show_lines && !show_words && !show_bytes)
    show_lines = show_words = show_bytes = true;

  FILE *fp = stdin;
  if (fname && strcmp(fname, "-") != 0) {
    fp = fopen(fname, "rb");
    if (!fp) {
      fprintf(stderr, "wc: cannot open '%s'\n", fname);
      seargs_free(p);
      return EXIT_FAILURE;
    }
  }

  const counts_t c = count_stream(fp);
  if (ferror(fp)) {
    fprintf(stderr, "wc: read error on '%s'\n", fname ? fname : "stdin");
    if (fp != stdin)
      fclose(fp);
    seargs_free(p);
    return EXIT_FAILURE;
  }
  if (fp != stdin && fclose(fp) != 0) {
    fprintf(stderr, "wc: error closing '%s'\n", fname);
    seargs_free(p);
    return EXIT_FAILURE;
  }
  seargs_free(p);

  int printed = 0;
  if (show_lines) {
    printf("%" PRIuMAX, c.lines);
    printed++;
  }
  if (show_words) {
    printf("%s%" PRIuMAX, printed ? " " : "", c.words);
    printed++;
  }
  if (show_bytes) {
    printf("%s%" PRIuMAX, printed ? " " : "", c.bytes);
    printed++;
  }
  if (fname)
    printf("%s%s", printed ? " " : "", fname);
  fputc('\n', stdout);
  return EXIT_SUCCESS;
}
