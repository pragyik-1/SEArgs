#include "../include/seargs.h"
#include <stdio.h>

int main(int argc, const char *argv[]) {
  const arg_def_t valid_args[] = {
      REQUIRED_ARG("input", 'i', ARG_STRING, "Input file path."),
      OPTIONAL_ARG("output", 'o', ARG_STRING, "Output directory.",
                   STRING_VAL("./")),
      OPTIONAL_ARG("somecount", 's', ARG_INT, "E", INT_VAL(3)),
      OPTIONAL_ARG("someflag", 'f', ARG_FLAG, "F", FLAG_VAL)};
  int valid_args_length = sizeof(valid_args) / sizeof(arg_def_t);
  args_t *args = PARSE_ARGS(argc, argv, valid_args);
  if (args == NULL) {
    return EXIT_FAILURE;
  }
  const arg_def_t *arg_input_def =
      get_arg_def(valid_args, "input", valid_args_length);
  const arg_def_t *arg_somecount_def =
      get_arg_def(valid_args, "somecount", valid_args_length);
  const arg_def_t *arg_someflag_def =
      get_arg_def(valid_args, "someflag", valid_args_length);
  const arg_def_t *arg_output_def =
      get_arg_def(valid_args, "output", valid_args_length);

  char *arg_input_val = get_arg_val(args, arg_input_def->name);
  int *arg_somecount_val = get_arg_val(args, arg_somecount_def->name);
  bool *arg_someflag_val = get_arg_val(args, arg_someflag_def->name);
  char *arg_output_val = get_arg_val(args, arg_output_def->name);

  printf("Input (desc): %s, value: %s\n", arg_input_def->desc, arg_input_val);
  printf("Somecount (desc): %s, value: %d\n", arg_somecount_def->desc,
         *arg_somecount_val);
  printf("someflag(desc): %s, value: %d\n", arg_someflag_def->desc,
         *arg_someflag_val);
  printf("Output (desc): %s, value: %s\n", arg_output_def->desc,
         arg_output_val);
  free_args(args);
  return EXIT_SUCCESS;
}