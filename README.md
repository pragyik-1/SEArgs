# SEArgs

Simple Easy Arguments (SEArgs)

A lightweight single-header argument parsing library for C (and C++) including
features such as automatic help text printing.

To include it, just define SEARGS_IMPLEMENTATION in exactly one source file and then include "seargs.h" anywhere else
```c
#define SEARGS_IMPLEMENTATION   /* in exactly one source file */
#include "seargs.h"
```

## Quick start

```c
#define SEARGS_IMPLEMENTATION
#include "seargs.h"

#include <stdio.h>
#include <stdlib.h>

static const seargs_arg_def_t defs[] = {
    SEARGS_REQUIRED_STRING_ARG("text", 't', "the text to repeat"),
    SEARGS_OPTIONAL_INT_ARG("count", 'c', "how many times to print it", 1),
    SEARGS_OPTIONAL_BOOL_ARG("upper", 'u', "print in uppercase", false),
    SEARGS_POSITIONAL_OPTIONAL_STRING_ARG("extra", "optional trailing text", ""),
};

int main(int argc, char *const argv[]) {
    seargs_parser_t *p =
        seargs_parse_args(argc, argv, defs, sizeof(defs) / sizeof(defs[0]));
    if (!p) {                      /* NULL = allocation failure */
        fprintf(stderr, "out of memory\n");
        return EXIT_FAILURE;
    }
    if (seargs_help_requested(p)) { /* argc == 1 printed the help listing */
        seargs_free(p);
        return EXIT_SUCCESS;
    }
    if (!IS_SEARGS_OK(p)) {        /* definition or parse error */
        fprintf(stderr, "error: %s\n",
                p->error.detail ? p->error.detail : "bad arguments");
        seargs_free(p);
        return EXIT_FAILURE;
    }

    const char *text = seargs_get_string(p, "text");
    int count = seargs_get_int(p, "count");
    for (int i = 0; i < count; i++)
        fputs(text, stdout);
    fputc('\n', stdout);

    if (seargs_is_found(p, "extra"))    /* given, not defaulted */
        printf("extra: %s\n", seargs_get_string(p, "extra"));

    seargs_free(p);
    return EXIT_SUCCESS;
}
```

```sh
$ ./myprog --text hi --count 3
hihihi
$ ./myprog --text hi --extra trailing
hi
extra: trailing
$ ./myprog                      # no arguments: prints help, exits 0
Usage:
  --text   (-t)  the text to repeat
  --count  (-c)  how many times to print it
  --upper  (-u)  print in uppercase
  [extra]        optional trailing text
```

## Parsing rules

- `--long`, `--long=value`, `--long value` (space-separated values too)
- `-s`, `-s value`, and clusters of boolean flags: `-abc`; a value-taking
  short option must be last in a cluster (`-abn 5`)
- A value that *looks like* another option must be attached with `=`
  (`--name=-weird`); negative numbers are allowed as separate values
- `--` ends option parsing; everything after it is positional
- A lone `-` fills the next positional slot
  and is an error when no positional slot is open
- Booleans are set by presence. An attached value (`--flag=false`), or a
  following token that is exactly `true`/`false`/`1`/`0` (case-insensitive)
  after a standalone boolean option (`--flag` or `-f`), is used as an explicit
  value. Any other following token is **not** consumed, so `--flag maybe`
  leaves `maybe` to a positional slot; inside a cluster (`-ab`) a trailing
  token is likewise left alone.
- Integers and doubles reject leading or trailing whitespace. Overflow is
  rejected; underflow is accepted. `inf`/`nan` follow `strtod`.
- A value-taking option that appears more than once is an error unless it has
  `SEARGS_FLAG_REPEATABLE` (last value wins). Repeating a boolean is harmless.
- Long names are matched exactly unless `SEARGS_FLAG_CASE_INSENSITIVE` is set.
- Positional arguments fill in declaration order
- With no positional arguments declared, bare tokens are ignored
- `seargs_parse_args` prints the help listing and sets `help_requested` when
  called with no arguments (`argc == 1`); check `seargs_help_requested()` and
  exit successfully. Use `seargs_parse_args_ex(..., false)` when no-argument
  behavior is meaningful (for example, reading stdin).

## API

### Definition types

```c
typedef enum {
    SEARGS_TYPE_BOOL, SEARGS_TYPE_INT, SEARGS_TYPE_DOUBLE, SEARGS_TYPE_STRING
} seargs_arg_type_t;

typedef union {
    bool bool_val; int int_val; const char *str_val; double double_val;
} seargs_arg_val_t;

typedef struct {
    const char *name;          /* long name, e.g. "count"  */
    const char *desc;          /* for help output; may be NULL */
    seargs_arg_val_t default_val;
    seargs_arg_type_t type;
    uint32_t flags;            /* 0 or SEARGS_FLAG_*        */
    char short_name;           /* '\0' = none               */
    bool is_required;
} seargs_arg_def_t;
```

### Constants and flags

| Name | Meaning |
|---|---|
| `SEARGS_NO_SHORT_NAME` | `'\0'` — pass to declare no short option |
| `ARG_NAME_MAX_SIZE` | long-name length limit (64) |
| `ARG_VAL_MAX_SIZE` | value length limit (256) |
| `SEARGS_FLAG_POSITIONAL` | matched by position, never usable as an option |
| `SEARGS_FLAG_CASE_INSENSITIVE` | match the long name case-insensitively |
| `SEARGS_FLAG_REPEATABLE` | allow a value-taking option to appear more than once (last wins) |
| `SEARGS_FLAG_MASK` | all known flag bits (for validation) |

`ARG_NAME_MAX_SIZE` (64) and `ARG_VAL_MAX_SIZE` (256) are exclusive limits:
names and values must be shorter than the limit. Both are `#ifndef`-guarded, so
a limit can be raised by defining it before the header is included — e.g.
`-DARG_VAL_MAX_SIZE=4096` or a project config header. Use the same value in
every translation unit that includes SEArgs.

### The declaration macros

Every optional argument **created through these macros** carries an explicit
default, so an absent argument can never hand you a value you didn't choose. A
raw `seargs_arg_def_t` with no default is zero-initialized. Positional macros
take no short name (they can never collide with an option).

| Macro | Arguments | Notes |
|---|---|---|
| `SEARGS_REQUIRED_BOOL_ARG` | `(name, short, desc)` | |
| `SEARGS_OPTIONAL_BOOL_ARG` | `(name, short, desc, default)` | |
| `SEARGS_REQUIRED_STRING_ARG` | `(name, short, desc)` | |
| `SEARGS_OPTIONAL_STRING_ARG` | `(name, short, desc, default)` | |
| `SEARGS_REQUIRED_INT_ARG` | `(name, short, desc)` | |
| `SEARGS_OPTIONAL_INT_ARG` | `(name, short, desc, default)` | |
| `SEARGS_REQUIRED_DOUBLE_ARG` | `(name, short, desc)` | |
| `SEARGS_OPTIONAL_DOUBLE_ARG` | `(name, short, desc, default)` | |
| `SEARGS_POSITIONAL_REQUIRED_STRING_ARG` | `(name, desc)` | no short name |
| `SEARGS_POSITIONAL_OPTIONAL_STRING_ARG` | `(name, desc, default)` | no short name |
| `SEARGS_POSITIONAL_REQUIRED_INT_ARG` | `(name, desc)` | no short name |
| `SEARGS_POSITIONAL_OPTIONAL_INT_ARG` | `(name, desc, default)` | no short name |
| `SEARGS_POSITIONAL_REQUIRED_DOUBLE_ARG` | `(name, desc)` | no short name |
| `SEARGS_POSITIONAL_OPTIONAL_DOUBLE_ARG` | `(name, desc, default)` | no short name |

The macro set has no boolean positional form, but a raw definition can use
`SEARGS_TYPE_BOOL` with `SEARGS_FLAG_POSITIONAL`. `desc` may be `NULL`; pass
`SEARGS_NO_SHORT_NAME` for no short option. Each macro expands to a braced
initializer, so the elements compose directly into a `defs[]` array.

### Parse and free

```c
seargs_parser_t *seargs_parse_args(int argc, char *const argv[],
                                   const seargs_arg_def_t *arg_defs,
                                   size_t n_arg_defs);
seargs_parser_t *seargs_parse_args_ex(int argc, char *const argv[],
                                      const seargs_arg_def_t *arg_defs,
                                      size_t n_arg_defs,
                                      bool print_help_on_no_args);
bool seargs_help_requested(const seargs_parser_t *p);
void seargs_free(seargs_parser_t *parser);
```

`seargs_parse_args` is `seargs_parse_args_ex(..., true)`. When the
`print_help_on_no_args` argument is true and `argc == 1`, a usage listing is
printed after definition validation and the returned parser has
`error.code == SEARGS_OK`, `help_requested == true`, and no required-argument
checks. Check `seargs_help_requested()` and exit successfully. Pass `false` to
keep no-argument behavior available to the program.

`seargs_parse_args_ex` returns:

- **`NULL`** — allocation failure or an argument count too large for the
  single-block allocation (the only ways to get `NULL`).
- **A parser with `error.code != SEARGS_OK`** — a definition or parse error;
  the error is fully described in `parser->error` and *must still be released*
  with `seargs_free`.
- **A parser with `error.code == SEARGS_OK`** — success (including the automatic
  help path). Read values with the accessors, then `seargs_free`.

### Reading results

```c
bool    seargs_help_requested(const seargs_parser_t *p);
const seargs_arg_state_t *seargs_get_state(const seargs_parser_t *p, const char *name);
bool    seargs_is_found(const seargs_parser_t *p, const char *name);
bool    seargs_get_bool(const seargs_parser_t *p, const char *name);
int     seargs_get_int(const seargs_parser_t *p, const char *name);
double  seargs_get_double(const seargs_parser_t *p, const char *name);
const char *seargs_get_string(const seargs_parser_t *p, const char *name);
```

- `seargs_help_requested` reports whether the automatic no-argument help listing
  was printed.
- Typed accessors return the parsed value, or the definition's default when the
  argument was absent. In debug builds they `assert` that the name exists and
  the type matches; in release builds a missing name or type mismatch returns
  the fallback value: `false`, `0`, `0.0`, or `NULL`.
- `seargs_is_found` distinguishes *"absent (default returned)"* from
  *"explicitly given, even as `false` / `0`"*.
- `seargs_get_state` returns the raw state (def back-pointer, `def_index`,
  parsed value, `is_found`) or `NULL` for an unknown name. It is safe to call
  after a definition-validation error.
- `IS_SEARGS_OK(p)` is shorthand for `((p)->error.code == SEARGS_OK)`.

### Errors

```c
typedef struct {
    seargs_error_t code;
    size_t index;        /* def index that caused the error; SIZE_MAX when N/A */
    const char *arg;     /* argv token that caused it; NULL when N/A          */
    const char *detail;  /* short, static, human-readable message; may be NULL */
} seargs_error_info_t;
```

The first error wins. Parse-time codes: `SEARGS_MISSING_ARG`, `SEARGS_INVALID_VALUE`,
`SEARGS_MISSING_VALUE`, `SEARGS_UNKNOWN_ARG`, `SEARGS_TOO_MANY_ARGS`,
`SEARGS_DUPLICATE_ARG`. Definition-validation codes: `SEARGS_DEF_TABLE_NULL`,
`SEARGS_DEF_NAME_NULL`, `SEARGS_DEF_NAME_EMPTY`, `SEARGS_DEF_NAME_INVALID`,
`SEARGS_DEF_NAME_TOO_LONG`, `SEARGS_DEF_DEFAULT_TOO_LONG`,
`SEARGS_DEF_SHORT_NAME_INVALID`, `SEARGS_DEF_DUPLICATE_NAME`,
`SEARGS_DEF_DUPLICATE_SHORT_NAME`, `SEARGS_DEF_INVALID_TYPE`,
`SEARGS_DEF_INVALID_FLAGS`, `SEARGS_DEF_POSITIONAL_INVALID`.

### Help

```c
void seargs_print_help(const seargs_arg_def_t *defs, size_t n_args);
```

Prints a usage listing of every defined argument to **stdout**. Options are
shown as `--name`; positional arguments are shown as `<name>` when required and
`[name]` when optional. `seargs_parse_args` calls it automatically when the
program is invoked with no arguments (`argc == 1`) and sets `help_requested`.
`seargs_parse_args_ex(..., false)` suppresses that automatic listing. Help is
the library's only output path; otherwise SEArgs never writes to `stdout` or
`stderr`.

## Portability

SEArgs requires C99 or later. The declaration macros use C99 designated
initializers; in C++ they require C++20. Public declarations are wrapped in
`extern "C"` for C++ callers.

## Ownership and lifetimes

- The parser and all argument states live in one aligned allocation, created
  inside `seargs_parse_args` / `seargs_parse_args_ex` and released with
  `seargs_free`
- String values alias the caller's `argv` strings. The library copies
  nothing. The pointers stay valid for as long as the program's `argv` does.
- Definition defaults are also aliased, not copied; declare them as static or
  string-literal storage. `desc` is used only by `seargs_print_help`.
- The definitions table must outlive the parser.

## Examples

- `examples/example.c` has a documented walkthrough: required/optional/positional
  args, help handling, error reporting, `seargs_is_found`, a `--` terminator.
- `examples/wc.c` has a real, minimal word-count tool (`-l`/`-w`/`-c`, file or
  stdin), built with SEArgs. It uses `seargs_parse_args_ex(..., false)` so no
  arguments means "read stdin" rather than "print help".

## Testing

`tests/run_tests.sh` builds and runs the verification suite, including the
help-formatting check. `CC` and `CFLAGS` are overridable via the environment.

```sh
cd tests && ./run_tests.sh
```
