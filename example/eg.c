#include "../headers/macros.h"
#include "../headers/seargs.h"

#include <stdio.h>

int main(int argc, const char *argv[]) {
  const arg_def_t valid_args[] = {
      REQUIRED_ARG("input", 'i', ARG_STRING, "Input file path."),
      OPTIONAL_ARG("output", 'o', ARG_STRING, "Output directory.",
                   STRING_VAL("./")),
      OPTIONAL_ARG("somecount", 's', ARG_INT, "E", INT_VAL(3)),
      OPTIONAL_ARG("someflag", 'f', ARG_FLAG, "F", FLAG_VAL)};
  args_t *args = PARSE_ARGS(argc, argv, valid_args);
  if (args == NULL) {
    return 1;
  }

  // Using the helper macros from macros.h for cleaner value retrieval.
  const char *arg_input_val = GET_STRING_ARG(args, "input");
  // using the function for value retrieval, this returrns a void pointer to the
  // value, you may have to cast it and properly use the pointer
  const char **arg_output_val = (const char **)get_arg_val(args, "output");
  int arg_somecount_val = GET_INT_ARG(args, "somecount");
  bool arg_someflag_val = GET_FLAG_ARG(args, "someflag");

  printf("Input value: %s\n", arg_input_val);
  printf("Output value: %s\n", *arg_output_val);
  printf("Somecount value: %d\n", arg_somecount_val);
  printf("Someflag value: %s\n", arg_someflag_val ? "true" : "false");

  free_args(&args);
  return 0;
}