#include "../src/seargs.h"

#include <stdio.h>

int main(int argc, const char *argv[]) {
  const arg_def_t valid_args[] = {
      REQUIRED_STRING_ARG("input", 'i', "Input file path."),
      OPTIONAL_STRING_ARG("output", 'o', "Output directory.", "./"),
      OPTIONAL_INT_ARG("somecount", 's', "just an int", 3),
      FLAG_ARG("someflag", 'f', "just a flag"), FLAG_ARG("help", 'h', "help")};
  // Make sure valid_args is an array and not decayed to a pointer for
  // PARSE_ARGS else use the parse_args() function
  parser_t *parser = PARSE_ARGS(argc, argv, valid_args);
  if (!parser) {
    return 1;
  }

  // Using the helper functions from helper.h for cleaner value retrieval.
  const char *arg_input_val = GET_STRING_ARG(parser, "input");
  // using the get_arg_val() for value retrieval, this returrns a void pointer
  // to the value, you may have to cast it and properly use the pointer. usually
  // you shouldnt have to use get_arg_val() except for certain exceptional cases
  // and isnt reccomended
  const char **arg_output_val = (const char **)get_arg_val(parser, "output");
  int arg_somecount_val = GET_INT_ARG(parser, "somecount");
  bool arg_someflag_val = GET_FLAG_ARG(parser, "someflag");
  bool arg_help_val = GET_FLAG_ARG(parser, "help");

  if (seargs_err(parser)) {
    return 1;
  }

  printf("Input value: %s\n", arg_input_val);
  printf("Output value: %s\n", *arg_output_val);
  printf("Somecount value: %d\n", arg_somecount_val);
  printf("Someflag value: %s\n", arg_someflag_val ? "true" : "false");
  printf("Help value: %s\n", arg_help_val ? "true" : "false");

  if (arg_help_val) {
    print_help(valid_args, sizeof(valid_args) / sizeof(valid_args[0]));
  }

  free_parser(&parser);
  return 0;
}