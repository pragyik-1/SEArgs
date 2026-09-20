#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>

#define SEARGS_IMPLEMENTATION
#include "../src/seargs.h"

static const seargs_arg_def_t defs[] = {
    SEARGS_OPTIONAL_BOOL_ARG("lines", 'l', "count lines", false),
    SEARGS_OPTIONAL_BOOL_ARG("words", 'w', "count words", false),
    SEARGS_OPTIONAL_BOOL_ARG("bytes", 'c', "count bytes", false),
    SEARGS_POSITIONAL_OPTIONAL_STRING_ARG("file", "input file (stdin when absent)", NULL),
};

typedef struct {
  long lines;
  long words;
  long bytes;
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
  seargs_parser_t *p = seargs_parse_args(argc, argv, defs, sizeof(defs) / sizeof(defs[0]));
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
  if (fname) {
    fp = fopen(fname, "rb");
    if (!fp) {
      fprintf(stderr, "wc: cannot open '%s'\n", fname);
      seargs_free(p);
      return EXIT_FAILURE;
    }
  }

  counts_t c = count_stream(fp);
  if (fp != stdin)
    fclose(fp);
  seargs_free(p);

  int printed = 0;
  if (show_lines) {
    printf("%ld", c.lines);
    printed++;
  }
  if (show_words) {
    printf("%s%ld", printed ? " " : "", c.words);
    printed++;
  }
  if (show_bytes) {
    printf("%s%ld", printed ? " " : "", c.bytes);
    printed++;
  }
  if (fname)
    printf("%s%s", printed ? " " : "", fname);
  fputc('\n', stdout);
  return EXIT_SUCCESS;
}