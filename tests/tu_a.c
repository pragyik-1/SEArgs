// TU A: uses seargs as a header-only library (no implementation).
#include "../src/seargs.h"

extern int run_b(void); // TU B provides the implementation

int main(void) {
  const seargs_arg_def_t defs[] = {
      {.name = "flag", .short_name = 'f', .type = SEARGS_TYPE_BOOL},
      {.name = "num", .type = SEARGS_TYPE_INT},
  };
  seargs_parser_t *p =
      seargs_parse_args(4, (char *const[]){ "prog", "--flag", "--num", "9" }, defs,
                        sizeof(defs) / sizeof(defs[0]));
  if (!p || p->error.code != SEARGS_OK)
    return 1;
  const int ok = seargs_get_bool(p, "flag") == true && seargs_get_int(p, "num") == 9 &&
                 seargs_is_found(p, "flag") && seargs_get_state(p, "num") != NULL;
  seargs_free(p);
  if (!ok)
    return 2;
  return run_b();
}