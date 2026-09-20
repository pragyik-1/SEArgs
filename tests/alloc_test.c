// Verifies the "exactly one heap allocation" guarantee using the linker's
// --wrap: only references compiled into this TU (which includes the seargs
// implementation) are counted, so stdio/glibc allocations cannot pollute the
// counters. Snapshot is taken immediately after seargs_parse_args.
// glibc-specific; run_tests.sh skips it when the link step fails.
#define SEARGS_IMPLEMENTATION
#include "../src/seargs.h"

#include <string.h>

static int live_allocations = 0;
static int peak_allocations = 0;

void *__real_malloc(size_t);
void *__real_calloc(size_t, size_t);
void *__real_realloc(void *, size_t);
void __real_free(void *);

void *__wrap_malloc(size_t n) {
  live_allocations++;
  if (live_allocations > peak_allocations)
    peak_allocations = live_allocations;
  return __real_malloc(n);
}

void *__wrap_calloc(size_t n, size_t s) {
  void *p = __real_calloc(n, s);
  live_allocations++;
  if (live_allocations > peak_allocations)
    peak_allocations = live_allocations;
  return p;
}

void *__wrap_realloc(void *p, size_t n) { return __real_realloc(p, n); }

void __wrap_free(void *p) {
  if (p)
    live_allocations--;
  __real_free(p);
}

static const seargs_arg_def_t defs[] = {
    {.name = "num", .type = SEARGS_TYPE_INT},
    {.name = "str", .type = SEARGS_TYPE_STRING},
};

int main(void) {
  char str_value[] = "hello";
  seargs_parser_t *p;
  int live_at_parse, peak_at_parse;

  p = seargs_parse_args(5, (char *const[]){ "prog", "--str", str_value, "--num", "7" }, defs,
                        sizeof(defs) / sizeof(defs[0]));
  live_at_parse = live_allocations;
  peak_at_parse = peak_allocations;

  if (!p || p->error.code != SEARGS_OK)
    return 1;
  if (live_at_parse != 1 || peak_at_parse != 1)
    return 2;

  // Values alias the caller's argv (documented), so they are valid while argv lives.
  if (seargs_get_int(p, "num") != 7 || strcmp(seargs_get_string(p, "str"), "hello") != 0)
    return 3;
  if (seargs_get_string(p, "str") != str_value)
    return 4;

  seargs_free(p);
  if (live_allocations != 0)
    return 5;

  seargs_free(NULL); // must be a no-op
  if (live_allocations != 0)
    return 6;

  return 0; // all good
}