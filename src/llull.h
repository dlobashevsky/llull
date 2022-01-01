
#ifndef UTHASH_H
#include "uthash.h"
#endif


struct grammar_t;

extern int yylineno;
void yyerror(struct grammar_t* g,const char *s, ...);

//! if set then fuction calling
#define GRAMMAR_CASE_FLAG_FUNC		0
//! if set then fuction calling
#define GRAMMAR_CASE_FLAG_CODE		1



typedef struct grammar_minmax_t
{
  intmax_t min;
  intmax_t max;
} grammar_minmax_t;

typedef struct grammar_string_t
{
  char* buf;
  size_t len;
  size_t allocated;
} grammar_string_t;



typedef enum  grammar_atom_type_t
{
//  grammar_atom_notdef=-1,
  grammar_atom_string,
  grammar_atom_id,
//  grammar_atom_code,
} grammar_atom_type_t;

typedef struct grammar_var_hash_t
{
  char* name;
  size_t order;
  intmax_t min,max;
  UT_hash_handle hh;
} grammar_var_hash_t;


typedef struct grammar_atom_t
{
  grammar_atom_type_t type;
  char* val;
  size_t order;
  size_t len;
  size_t offset;
  uint8_t var;
  size_t varno;
  struct grammar_atom_t* next;
} grammar_atom_t;

typedef struct grammar_arg_t
{
  const char* name;
  const char* type;
  size_t no;
  uint8_t var;
  size_t varno;
} grammar_arg_t;

typedef struct grammar_case_t
{
  uint8_t flags;
  char* ref;
  char* var;
  size_t varno;
  size_t order;
  size_t args_count;
  grammar_arg_t* args;
  char* signature;
  grammar_atom_t* atoms;
  struct grammar_case_t* next;
  int32_t depth[2];
} grammar_case_t;


typedef struct grammar_rules_hash_t
{
  char* name;
  char* type;

  size_t no;
  size_t cases_cnt;
  int32_t depth[2];

  grammar_case_t* cases;
  UT_hash_handle hh;
} grammar_rules_hash_t;


typedef struct grammar_t
{
  char* name;
  char* init;
  char* uuid;
  char* context;
  char* codon;
  char* fitness;

  size_t rules_cnt;
  grammar_string_t* code;
  grammar_var_hash_t* vars;
  grammar_rules_hash_t* rules;
} grammar_t;


grammar_t* grammar_init(void);
void grammar_free(grammar_t*);
int grammar_dump(const grammar_t*,FILE* f);

int grammar_add_var(grammar_t*,const char* name,intmax_t min,intmax_t max);
int grammar_add_var2(grammar_t*,const char* name,grammar_minmax_t m);
int grammar_add_rng(grammar_t*,const char* func,const char* ctx);

int grammar_set_name(grammar_t*,const char* name);
int grammar_set_init(grammar_t*,const char* root);
int grammar_set_uuid(grammar_t*,const char* uuid);
int grammar_set_context(grammar_t*,const char* id);
int grammar_set_codon(grammar_t*,unsigned bytes);
int grammar_set_fitness(grammar_t*,const char* id);

int grammar_add_rule(grammar_t*,const char* id,const char* type,grammar_case_t*);

grammar_atom_t* grammar_atom_init(grammar_atom_type_t t,const char* str,grammar_atom_t* parent);
void grammar_atom_free(grammar_atom_t* at);

grammar_case_t* grammar_case_init(const char* mult,grammar_atom_t* list,uint32_t flag,const char* str);
void grammar_case_free(grammar_case_t* c);

char* grammar_atoms_squeeze(const grammar_atom_t* list);

int grammar_add_code(grammar_t*,const char* str);

grammar_string_t* grammar_string_init(size_t init);
void grammar_string_free(grammar_string_t*);
const char* grammar_string_plain(grammar_string_t*);
int grammar_string_append(grammar_string_t*,const char*);
int grammar_string_append0(grammar_string_t*,char);

