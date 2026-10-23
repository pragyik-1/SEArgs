// TU B: provides the single implementation.
#define SEARGS_IMPLEMENTATION
#include "../src/seargs.h"

#include <string.h>

int run_b(void) {
  const seargs_arg_def_t defs[] = {
      {.name = "str", .type = SEARGS_TYPE_STRING},
      {.name = "dflt", .type = SEARGS_TYPE_STRING, .default_val = {.str_val = "none"}},
  };
  seargs_parser_t *p = seargs_parse_args(3, (char *const[]){"prog", "--str", "hi"}, defs,
                                         sizeof(defs) / sizeof(defs[0]));
  if (!p || p->error.code != SEARGS_OK)
    return 3;

  const int ok = strcmp(seargs_get_string(p, "str"), "hi") == 0 &&
                 strcmp(seargs_get_string(p, "dflt"), "none") == 0; // default applied
  seargs_free(p);
  return ok ? 0 : 4;
}
