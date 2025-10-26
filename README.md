# SEArgs.h (Simple Easy Arguments)

## Installation
Just copy `seargs.h` into your project and include it and thats it.

## Usage
To interact with the library you will usually be using the macros it provides

These two are important macros which you will be using to define the args for your program

```c
REQUIRED_ARG(LONG_NAME, SHORT_NAME, TYPE, DESCRIPTION)
/* 
The default value expects you to use one of the builtin macros to input a default value depending on the type.
Such as if you wanted an int as the default you would do INT_VAL(20). the type of default and the type you put in must be the same.
*/ 
OPTIONAL_ARG(LONG_NAME, SHORT_NAME, TYPE, DESCRIPTION, DEFAULT) 
```
Here is a brief description of all of the parameters
 - `LONG_NAME:` It is the actual name of the agrument such as "input"
 - `SHORT_NAME`: You can optionally provide a short name such as "i" for arguments, you would use 0 to disable this.
 - `TYPE`: It is the type of value you expect this arg to contain. eg: ./prog --input "./file", here the type of the arg is a string (char *)
 - `DESCRIPTION`: An optional description you can provide for your arg, leave as empty string to disable this.
 
 This is how you will usually be defining the arguments of your program.
```c
const args_def_t valid_args[] = {
    REQUIRED_ARG("input", 'i', ARG_STRING, "Input file path."),
    OPTIONAL_ARG("output", 'o', ARG_STRING, "Output file path.", STRING_VAL("./output.txt")),
    OPTIONAL_ARG("verbose", 'v', ARG_FLAG, "Enable verbose output.", FLAG_VAL)
}
```

You would get and use the arguments using the following
```c
args_t *args = PARSE_ARGS(argc, argv, valid_args);
```

Putting it all together

```c
#include <seargs.h>

int main(int argc, char *argv[]) {
    const args_def_t valid_args[] = {
        REQUIRED_ARG("input", 'i', ARG_STRING, "Input file path."),
        OPTIONAL_ARG("output", 'o', ARG_STRING, "Output file path.", STRING_VAL("./output.txt")),
        OPTIONAL_ARG("verbose", 'v', ARG_FLAG, "Enable verbose output.", FLAG_VAL)
    }
    args_t *args = PARSE_ARGS(argc, argv, valid_args);
}
```

You can access the values of the arguments using the following
```c
char **input_arg_val = get_arg_val(args, "input");
char **output_arg_val = get_arg_val(args, "output");
// Note: get_arg_val returns a void pointer to the specified type, you may have to cast it.
```

Using these you would be able to do something like
```c
#include <seargs.h>

int main(int argc, char *argv[]) {
    const args_def_t valid_args[] = {
        REQUIRED_ARG("input", 'i', ARG_STRING, "Input file path."),
        OPTIONAL_ARG("output", 'o', ARG_STRING, "Output file path.", STRING_VAL("./output.txt")),
        OPTIONAL_ARG("verbose", 'v', ARG_FLAG, "Enable verbose output.", FLAG_VAL)
    }
    args_t *args = PARSE_ARGS(argc, argv, valid_args);
    char **input_arg_val = get_arg_val(args, "input");
    char **output_arg_val = get_arg_val(args, "output");
}
``` 

### Helpers
These are some helpers
```c
args_def_t *input_arg_def = GET_DEF(args, "input"); // Gives you the definition of the arg such as name, description...
free_args(&args) // lets you cleanly free the args preventing memory leaks.
```