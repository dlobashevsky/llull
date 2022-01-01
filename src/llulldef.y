%{
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdarg.h>

#include "llull.h"
//#include "llulldef.tab.h"
#include "macros.h"

void yyerror(grammar_t* g,const char *s, ...)
{
  va_list ap;
  va_start(ap, s);

  fprintf(stderr, "Parsing error at line %d; ", yylineno);
  vfprintf(stderr, s, ap);
  fprintf(stderr, "\n");
}

extern int yylex(void);

%}


%union {
  char* s;
  uint32_t n;
  grammar_atom_t* atom;
  grammar_case_t* sub;
  struct grammar_minmax_t minmax;
/*  int t;*/
}

/* declare tokens */
%token <n> TOKEN_NUMBER
%token <s> TOKEN_ID TOKEN_STRING FREE_CODE TOKEN_CTYPE
%token TOKEN_INIT TOKEN_NAME  TOKEN_VAR  TOKEN_UUID  TOKEN_CONTEXT  TOKEN_BLOCK TOKEN_CODON TOKEN_FITNESS TOKEN_RNG
%token TOKEN_EOF TOKEN_SEMICOLON TOKEN_RULE  TOKEN_VARIANT  TOKEN_AT TOKEN_SBRA TOKEN_SKET

%type <s> mult
%type <atom> atom subrule
%type <sub> sub rlist
%type <minmax> mod

%start root

%parse-param {struct grammar_t* grammar}

%%

root:  prefix_section TOKEN_BLOCK  main_section
|	prefix_section TOKEN_BLOCK  main_section TOKEN_BLOCK
;

prefix_section:
|	prefix_directive prefix_section 
;


prefix_directive:	TOKEN_NAME TOKEN_ID		{  if(grammar_set_name(grammar,$2)) crash("%%name double definition"); md_free($2); }
|			TOKEN_INIT TOKEN_ID		{  if(grammar_set_init(grammar,$2)) crash("%%init double definition"); md_free($2); }
|			TOKEN_CONTEXT TOKEN_CTYPE	{  if(grammar_set_context(grammar,$2)) crash("%%context double definition"); md_free($2); }
|			TOKEN_UUID TOKEN_STRING		{  if(grammar_set_uuid(grammar,$2)) crash("%%uuid double definition"); md_free($2);}
|			TOKEN_CODON TOKEN_NUMBER	{  if(grammar_set_codon(grammar,$2)) crash("%%codon double definition"); }
|			TOKEN_FITNESS TOKEN_CTYPE	{  if(grammar_set_fitness(grammar,$2)) crash("%%fitness double definition"); md_free($2); }
|			TOKEN_VAR TOKEN_ID mod		{  if(grammar_add_var2(grammar,$2,$3)) crash0("%%var double definition <%s>",$2); md_free($2); }
|			TOKEN_RNG TOKEN_CTYPE		{  grammar_add_rng(grammar,$2,0); md_free($2); }
|			TOKEN_RNG TOKEN_CTYPE TOKEN_CTYPE	{
								 grammar_add_rng(grammar,$2,$3); md_free($2); md_free($3);
								}
|			FREE_CODE 			{  grammar_add_code(grammar,$1); md_free($1); }
;

mod:			TOKEN_NUMBER TOKEN_RULE TOKEN_NUMBER	{  $$= (grammar_minmax_t){$1,$3}; }
|			TOKEN_NUMBER				{  $$= (grammar_minmax_t){0,$1};}
|								{  $$= (grammar_minmax_t){0,0};}
;

mult:								{ $$=0;}
|			TOKEN_SBRA TOKEN_ID TOKEN_SKET		{ $$=$2;}
;

main_section:		rule
|			main_section rule
;

rule:		TOKEN_ID TOKEN_CTYPE TOKEN_RULE rlist TOKEN_SEMICOLON	{
									  if(grammar_add_rule(grammar,$1,$2,$4))  crash("double defined rule");
									  md_free($1);md_free($2);
									}
;


rlist:		sub					{  $$=$1; }
|		rlist TOKEN_VARIANT sub 		{  $3->next=$1; $3->order=$1->order+1;  $$=$3; }
;

sub:		mult subrule TOKEN_AT TOKEN_ID		{ $$=grammar_case_init($1,$2,GRAMMAR_CASE_FLAG_FUNC,$4);md_free($1);md_free($4);}
|		mult subrule FREE_CODE 			{
							   $$=grammar_case_init($1,$2,GRAMMAR_CASE_FLAG_CODE,$3);
							   md_free($1);md_free($3);
							}
;

subrule:						{ $$ = 0;}
|		subrule atom				{ $$ = grammar_atom_init($2->type,$2->val,$1); grammar_atom_free($2); }
;

atom:		TOKEN_STRING				{  $$ = grammar_atom_init(grammar_atom_string,$1,0); md_free($1);}
|		TOKEN_ID				{  $$ = grammar_atom_init(grammar_atom_id,$1,0); md_free($1);}
;


%%
