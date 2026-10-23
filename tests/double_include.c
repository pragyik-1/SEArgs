// Including the implementation section twice in one translation unit must not
// redefine the implementation symbols.
#define SEARGS_IMPLEMENTATION
#include "../src/seargs.h"

int main(void) {
  const seargs_arg_def_t defs[] = {
      SEARGS_OPTIONAL_BOOL_ARG("flag", 'f', NULL, false),
  };
  seargs_parser_t *p = seargs_parse_args_ex(2, (char *const[]){"prog", "--flag"}, defs,
                                            sizeof(defs) / sizeof(defs[0]), false);
  if (!p || p->error.code != SEARGS_OK || !seargs_get_bool(p, "flag")) {
    seargs_free(p);
    return 1;
  }
  seargs_free(p);
  return 0;
}
