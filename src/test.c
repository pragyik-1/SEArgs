#include "seargs.h"

int main(int argc, char *argv[]) {
  const arg_def_t valid_args[] = {
      REQUIRED_ARG("input", 'i', ARG_STRING, "Input file path."),
      OPTIONAL_ARG("output", 'o', ARG_STRING, "Output directory.",
                   STRING_VAL("./")),
      OPTIONAL_ARG("somecount", 's', ARG_INT, "E", INT_VAL(0))};
  args_t *args = PARSE_ARGS(argc, argv, valid_args);
  if (args == NULL) {
    return EXIT_FAILURE;
  }
  printf("Input: %s\n", (char *)get_arg_val(args, "input"));
  printf("somecount: %d\n", *(int *)get_arg_val(args, "somecount"));
  free_args(args);
  return EXIT_SUCCESS;
}