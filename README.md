# llull
Static sentence generator in C with yacc-like syntax

Part of Grammatical Evolution engine (Hegel - https://github.com/dlobashevsky/hegel.git), also useful in fuzzer, generating QA sequences etc.

## Conceptions
Production grammar described in yacc-style manner with direct including parts of C code

## Use
llull options input_grammar output_folder

### Options

* `-v` print version
* `-h` short help
* `-i filename` input grammar file name
* `-o foldername` output directory for generated code
* `-n` skip prototype file creation
* `-u uuid` override uuid
* `-d filename` dump grammar to specified file (for debug)

## Grammar

### Options

* `%name alphanumeric_identifier`
>Commmon prefix for all generated entities in C language.
>For example `%name irisdt` will lead to generate types like  `irisdt__chromosome_t` and methods like `irisdt__chromosome_bits`

* `%codon len`
>Codon length in bits (now only 8,16,32). Default 32. Example `%codon 16`

* `%fitness <type>`
> Type of fitness value, type may be any C type returned by fuction, i.e. `%fitness <float>`.

* `%context <type>`
> Type of context - user-data structure transparently passed to user defined functions. Example: `%context <irisdt_context_t*>`

* `%init alphanumeric_identifier`
> Start production rule name'

* `%uuid ...`
> uuid generated in generation phase, may be used to version control or dll management. If not set then generated.

* `%var alphanumeric_identifier diapason`
> Define sort of ephemeral variable, diapason may be empty. Example `%var ARRAY_SIZE 1:255`. If diapason not set then take as min/max value of `%codon`
> Another example:
```
%var REGISTER  0:15
...

asm__binary_instruction: "\tADD " operand "," operand "\n" {* $$=$1+$2; *}
| ...

operand: "REG[" REGISTER "]" {* $$=register_content[$1]; *}
| ...

```

* `%random <type>`
> Random generator context type

* `%pool`
> Enable gene pooling by append single variant like `| "_nodename_pool[" index "]";` to every production rule. Array of pool may be filled by building blocks dynamically.

### Raw code

Raw C code enclosed in `{*` and `*}` brackets and included as is.
>Example:
```
{*
#include "context_definitions.h"
*}
```

### Production rules

`rule_name <type_name> : variant_definition function_call | variant_definition function_call .... ;`

`type_name` is a C type assotiated with given rule, all functions in `function_All` should return this type.
`variant_definition` is a list of strings, rule names or variable names.
`function_call` may be `{* raw C code *}` block or reference to externally defined function in form `@function_name`

### Resulting code structure

Three files are generated: name.h, name.c and name__proto.h where `name` is string defined in `%name` directive
Last file define prototypes for functions found in @-part of production rules.
name.* files contain definition and implementation of generated functions.

### Generated types and methods

Because names and signatures of functions depends from source grammar we well use in this documents `name` as `%name name` synonym, `context` as `%context context` ans so on

#### types

* `typedef uint16_t name__codon_t;` define codon type (in given case for `%codon 16`)
* `typedef float name__fitness_t;` define fitness type (in given case for `%fitness <float>`)
* Single chromosome definition:
```
typedef struct name__chromosome_t
{
  size_t len;
  name__fitness_t fitness;
  name__codon_t data[0];
} name__chromosome_t;
```

#### Methods

For functions returning `int` zero return assumed as success, non-zero as error
For functions returning pointer zero return assumed as error, non-zero as success
For functions returning `ssize_t` error marked as negative return

##### Global subsystem methods

* `name_t* name_init(size_t wrap);`  Initialize subsystem
* `void name_free(name_t*);` Deinitialize subsystem
* `char* name_dumps(const name_t*);` Dump definition structure to string
* `int name_dump(const name_t*,FILE*);` Dump definition structure to stream
* `int name__setvar(name_t*,size_t varidx,uintmax_t min,uintmax_t max);` Set varriable diapason dynamically.

##### Single chromosome methods

* `name__chromosome_t* name__chromosome_init0(const name_t*,size_t sz);` chromosome constructor, all positions are zeroed
* `name__chromosome_t* name__chromosome_init(const name_t*,size_t sz);` chromosome constructor, filled by random data
* `void name__chromosome_free(name__chromosome_t*);` chromosome destructor
* `int name__chromosome_check(const name_t*,const name__chromosome_t*);` return 0 if chromosome may be successfully parsed, wrap_count should be>0
* `char* name__chromosome_dumps(const name_t*,const name__chromosome_t*);` dump chromosome to string, returned string must be freed by caller
* `int name__chromosome_dump(const name_t*,const name__chromosome_t*,FILE*);` dump chromosome to FILE
* `int32_t name__chromosome_save(const name_t*,const name__chromosome_t*,FILE*);` serialize data, return blob size or <0 if error
* `ssize_t name__chromosome_save_size(const name_t*,const name__chromosome_t*);` return size of serializable data
* `name__chromosome_t* name__chromosome_load(const name_t*,FILE*);` deserialize
* `int name__chromosome_compute(const name_t*,const name__chromosome_t*,context_t* ctx,const char** res);` run chromosome computation, return value of root
* `double name__chromosome_bits(const name_t*,const name__chromosome_t*);` return sum of log(bit size) for MDL computation or parsimony pressure, -1 if error
* `name__chromosome_t* name__chromosome_mutate(const name__chromosome_t*,size_t offset,name__codon_t new);` mutate of single codon, validity not guaranteed
* `name__chromosome_t* name__chromosome_mutate_inplace(name__chromosome_t*,size_t offset,name__codon_t new);` inplace random mutate of single codon, validity not guaranteed
* `int name__chromosome_uniform(const name__chromosome_t*,const name__chromosome_t*,size_t offset,name__chromosome_t**,name__chromosome_t**);` uniform crossover, validity of childs not guaranteed
* `int name__chromosome_uniform_inplace(name__chromosome_t*,name__chromosome_t*,size_t offset);` inplace uniform crossover, resulting chromosomes may be broken

TODO

chromosome_fix (by backtracking)
chromosome_init_valid (by dynamically rules probability change)


