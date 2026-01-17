# SEArgs (Simple Easy Arguments)

SEArgs is a lightweight C library for parsing command-line arguments with ease.

## Installation

Simply include seargs.h and seargs.c in your project, or add them to your Makefile/CMake targets.

## Usage

The following is a minimal example to show how the library could be used showing most but not all features of it:

```c
#include "seargs.h"
#include <stdio.h>

int main(int argc, const char *argv[]) {
  const arg_def_t valid_args[] = {
      REQUIRED_STRING_ARG("input", 'i', "Input file path."),
      OPTIONAL_STRING_ARG("output", 'o', "Output directory.", "./"),
      OPTIONAL_INT_ARG("somecount", 's', "just an int", 3),
      FLAG_ARG("someflag", 'f', "just a flag"), FLAG_ARG("help", 'h', "help") // Flags are inherently optional and default to false
    };
  // Make sure valid_args is an array and not decayed to a pointer for PARSE_ARGS
  // else use the parse_args() function which expects length of array to also be passed in.
  parser_t *parser = PARSE_ARGS(argc, argv, valid_args);
  if (!parser) {
    return 1;
  }

  const char *arg_input_val = GET_STRING_ARG(parser, "input");
  // using the get_arg_val() for value retrieval, this returrns a void pointer
  // to the value, you may have to cast it and properly use the pointer. usually
  // you shouldnt have to use get_arg_val() except for certain exceptional cases
  // and isnt recommended
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

  // Use the args...

  free_parser(&parser);
  return 0;
}
```

Lets walk through this step-by-step,

```c
const arg_def_t valid_args[] = {
      REQUIRED_STRING_ARG("input", 'i', "Input file path."),
      OPTIONAL_STRING_ARG("output", 'o', "Output directory.", "./"),
      OPTIONAL_INT_ARG("somecount", 's', "just an int", 3),
      FLAG_ARG("someflag", 'f', "just a flag"), FLAG_ARG("help", 'h', "help")
    };
```
You would want an array of arg_def_t which provide the definitions to every argument you want to parse.

Now,
```c
parser_t *parser = PARSE_ARGS(argc, argv, valid_args);
if (!parser) {
    return 1;
}
```

This quite simply just parses the args and returns the parser, if the parser encounters a fatal error it returns NULL which is being checked here.
In any non-fatal error case, the parser immediately halts parsing and populates the `error` field accessed by `parser->error`

`parser->error` contains a few notable fields:
```c
parser->error.code // contains an enum of error_codes with 0 being ok. This field will always be populated
parser->error.msg // contains an error message, it might not always be populated i.e NULL
parser->error.arg_name // contains the name of the arg that caused the error, it might not always be populated.
```

Also note that errors can be checked broadly using the macros `seargs_ok(parser)` and `seargs_err(parser)`
as seen in 
```c
if (seargs_err(parser)) {
    return 1;
}
```

The `seargs_try(parser, expr)` macro also exists which resets the errors and attempts to execute a code block and if an error is encountered then parser->error is populated once again.

```c
  const char *arg_input_val = GET_STRING_ARG(parser, "input");
  int arg_somecount_val = GET_INT_ARG(parser, "somecount");
  bool arg_someflag_val = GET_FLAG_ARG(parser, "someflag");
  bool arg_help_val = GET_FLAG_ARG(parser, "help");
```

Every arg can be gotten with their respective types using their respective getter macros.
The getter macros will populate the `parser->error` field on failure which can be checked with `seargs_err(parser)` or `seargs_ok(parser)`.

After usage the parser should be freed using the `free_parser()` function to avoid memory leaks.
```c
free_parser(&parser);
```

# Seargs API Reference

`seargs` is a macro-based C library designed for simple and declarative command-line argument parsing.

## 1. Argument Definition

These macros are used to register expected arguments within your parser configuration.

### Core Macros

Use these for generic definitions. When providing a `DEFAULT` for `OPTIONAL_ARG`, you must wrap the value in a type-specific macro (e.g., `INT_VAL(10)` or `STR_VAL("default")`).

| Macro | Description | 
| ----- | ----- | 
| `REQUIRED_ARG(LONG_NAME, SHORT_NAME, TYPE, DESCRIPTION)` | Parsing fails if this argument is not present in the command line. | 
| `OPTIONAL_ARG(LONG_NAME, SHORT_NAME, TYPE, DESCRIPTION, DEFAULT)` | Defaults to the provided value if the user does not specify it. | 

### Typed Convenience Macros

These shorthand macros simplify definitions by removing the need for explicit type enums or value-wrapping macros.

| Type | Required Variant | Optional Variant | 
| ----- | ----- | ----- | 
| **Integer** | `REQUIRED_INT_ARG(LONG, SHORT, DESC)` | `OPTIONAL_INT_ARG(LONG, SHORT, DESC, DEFAULT)` | 
| **Double** | `REQUIRED_DOUBLE_ARG(LONG, SHORT, DESC)` | `OPTIONAL_DOUBLE_ARG(LONG, SHORT, DESC, DEFAULT)` | 
| **String** | `REQUIRED_STRING_ARG(LONG, SHORT, DESC)` | `OPTIONAL_STRING_ARG(LONG, SHORT, DESC, DEFAULT)` | 
| **Boolean** | — | `FLAG_ARG(LONG, SHORT, DESC)` | 

> **Note on Flags:** `FLAG_ARG` acts as a boolean switch. It requires no value from the user and defaults to `false` if absent.

## 2. Parameter Glossary

| Parameter | Type | Description | 
| ----- | ----- | ----- | 
| `LONG_NAME` | `string` | The primary identifier for the argument (e.g., `"output"`). | 
| `SHORT_NAME` | `char` | A single character alias (e.g., `'o'`). Use `0` if no short name is desired. | 
| `TYPE` | `macro` | The expected data type (e.g., `ARG_INT`, `ARG_STRING`). | 
| `DESCRIPTION` | `string` | A help string for the argument. Use `""` for no description. | 
| `DEFAULT` | `mixed` | The fallback value used when an optional argument is missing. | 

## 3. Value Retrieval & Error Handling

Use these macros to access parsed values. These macros automatically clear the previous error state before evaluation.

### Type-Specific Getters

* `GET_INT_ARG(parser, name)` — Retrieves the value as an `int`.
* `GET_DOUBLE_ARG(parser, name)` — Retrieves the value as a `double`.
* `GET_FLOAT_ARG(parser, name)` — Retrieves the value as a `float`.
* `GET_STRING_ARG(parser, name)` — Retrieves the value as a `char*`.
* `GET_FLAG_ARG(parser, name)` — Retrieves the boolean state of a flag.

### Error Checking Utilities

You can check the state of the parser using these boolean macros:

* `seargs_ok(parser)` — Returns true if the parser is in a valid state.
* `seargs_err(parser)` — Returns true if an error occurred during parsing or retrieval.