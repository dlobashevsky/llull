#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <uuid/uuid.h>

#include "macros.h"
#include "llull.h"
#include "codegen.h"

#include "llulldef.tab.h"

#define DEF_STRING_BLOCK	256
#define LLULL_VERSION		"0.1"
#define DEF_OUTPUT		"./llull.gen/"

static int grammar_atom_list_str2(const grammar_atom_t* list,FILE *f);

static char* typedup(const char* s)
{
  if(!s)  return 0;
  while(*s && (*s==' ' || *s=='\t')) s++;
  if(!s || *s++!='<')  return 0;
  while(*s && (*s==' ' || *s=='\t')) s++;
  if(!s)  return 0;
  const char* t=strrchr(s,'>');
  if(!t--)  return 0;
  while(t>s && (*t==' '|| *t=='\t')) t--;

  char* rv=md_calloc(t-s+2);
  memcpy(rv,s,t-s+1);
  return rv;
}

grammar_t* grammar_init(void)
{
  grammar_t* rv=md_new(rv);
  rv->code=grammar_string_init(0);
  return rv;
}

void grammar_free(grammar_t* g)
{
  if(!g)  return;
  md_free(g->name);
  md_free(g->init);
  md_free(g->uuid);
  md_free(g->context);
  md_free(g->codon);
  md_free(g->fitness);

  grammar_string_free(g->code);

  while(g->vars)
  {
    grammar_var_hash_t* v=g->vars;
    HASH_DELETE(hh,g->vars,v);
    md_free(v->name);
    md_free(v);
  }

  while(g->rules)
  {
    grammar_rules_hash_t* v=g->rules;
    HASH_DELETE(hh,g->rules,v);
    md_free(v->name);
    md_free(v->type);
    grammar_case_free(v->cases);
    md_free(v);
  }

  md_free(g);
}

int grammar_dump(const grammar_t* g,FILE* f)
{
  if(!f || !g)  return -1;
  fprintf(f,"\n%%name %s\n%%init %s\n%%codon <%s>\n%%fitness <%s>\n%%context <%s>\n%%uuid \"%s\"\n\n",
     g->name,g->init,g->codon,g->fitness,g->context,g->uuid);

  for(grammar_var_hash_t* v=g->vars;v;v=v->hh.next)
    if(v->min==v->max)
      fprintf(f,"%%var %s\n",v->name);
    else
      fprintf(f,"%%var %s %jd:%jd\n",v->name,v->min,v->max);

  fprintf(f,"\n\n");

  const char* code=grammar_string_plain(g->code);
  fprintf(f,"%%{%s%%}\n\n%%%%\n\n",code);
//  md_free(code);

  for(grammar_rules_hash_t* v=g->rules;v;v=v->hh.next)
  {
    fprintf(f,"%s<%s>:\t",v->name,v->type);
    int u=0;
    for(grammar_case_t* c=v->cases;c;c=c->next)
    {
      if(u)
      {
        fprintf(f,"\n| ");
        u=1;
      }
      else
        u=1;

      if(c->var)
        fprintf(f," [ %s ]\t",c->var);
      else
        fprintf(f,"\t\t");
      grammar_atom_list_str2(c->atoms,f);
      if((c->flags&1)==GRAMMAR_CASE_FLAG_FUNC)
        fprintf(f," @%s",c->ref);
      else
        fprintf(f," %%{ %s %%}",c->ref);

    }
    fprintf(f,"\n;\n\n");
  }


  fprintf(f,"\n\n");
  return 0;
}


#define SETTER(n)						\
int grammar_set_##n(grammar_t* g,const char* n)			\
{								\
  if(!g)  return -1;						\
  if(g->n) return 1;						\
  g->n=md_strdup(n);						\
  return 0;							\
}

#define SETTER2(n)						\
int grammar_set_##n(grammar_t* g,const char* n)			\
{								\
  if(!g)  return -1;						\
  if(g->n) return 1;						\
  g->n=typedup(n);						\
  return 0;							\
}


SETTER(name)
SETTER(init)
SETTER(uuid)

SETTER2(context)
SETTER2(fitness)

int grammar_set_codon(grammar_t* g,unsigned bytes)
{
  static const char* types[]=
  {
    "uint8_t",
    "uint16_t",
    0,
    "uint32_t",
    0,
    0,
    0,
    "uint64_t"
  };

  if(!g)  return -1;
  if(g->codon) return 1;
  if(!bytes || (bytes%8) || bytes>64 || !types[bytes/8-1]) crash("allowed codon size is 8,16,32,64");
  g->codon=md_strdup(types[bytes/8-1]);
  return 0;
}


int grammar_add_var(grammar_t* g,const char* name,intmax_t min,intmax_t max)
{
  if(!g || !name || !*name)  return -1;
  grammar_var_hash_t* r=0;
  HASH_FIND(hh,g->vars,name,strlen(name),r);
  if(r)  return -1;
  r=md_new(r);
  r->name=md_strdup(name);
  r->min=min;
  r->max=max;
  r->order=g->vars_cnt++;
  HASH_ADD_KEYPTR(hh,g->vars,r->name,strlen(r->name),r);
  return 0;
}

int grammar_add_var2(grammar_t* g,const char* name,grammar_minmax_t m)
{
  return grammar_add_var(g,name,m.min,m.max);
}

int grammar_add_rule(grammar_t* g,const char* id,const char* type,grammar_case_t* c)
{
  if(!g || !id || !type || !*type || !*id || !c)  return -1;
  grammar_rules_hash_t* r=0;

  {
    grammar_var_hash_t* v=0;
    HASH_FIND(hh,g->vars,id,strlen(id),v);
    if(v) crash("rule have same name as variable");
  }

  HASH_FIND(hh,g->rules,id,strlen(id),r);
  if(!r)
  {
    r=md_new(r);
    r->name=md_strdup(id);
    r->type=typedup(type);
    r->no=g->rules_cnt++;
    r->cases_cnt=c ? c->order+1 : 0;
    HASH_ADD_KEYPTR(hh,g->rules,r->name,strlen(r->name),r);
  }
  else
    crash("duplicate rule found");

  r->cases=c;
  return 0;
}


grammar_case_t* grammar_case_init(const char* mult,grammar_atom_t* list,uint32_t flag,const char* str)
{
  grammar_case_t* r=md_new(r);
  r->ref=str ? md_strdup(str) : 0;
  r->flags=flag;
  if(mult)
    r->var=md_strdup(mult);

  r->atoms=list;

  return r;
}


void grammar_case_free(grammar_case_t* c)
{
  while(c)
  {
    grammar_case_t* next=c->next;
    md_free(c->var);
    md_free(c->ref);
    grammar_atom_free(c->atoms);

    md_free(c->signature);
    md_free(c->args);
    md_free(c);
    c=next;
  }
}

void grammar_atom_free(grammar_atom_t* at)
{
  while(at)
  {
    grammar_atom_t* next=at->next;
    md_free(at->val);
    md_free(at);
    at=next;
  }
}

grammar_atom_t* grammar_atom_init(grammar_atom_type_t t,const char* str,grammar_atom_t* list)
{
  grammar_atom_t* r =md_new(r);

  r->val=md_strdup(str);
  r->type=t;
  r->len=strlen(str);
  r->next=list;
  if(list)
  {
    r->order=list->order+1;
    r->offset=list->offset+list->len+1;
  }

  return r;
}


char* grammar_atoms_squeeze(const grammar_atom_t* list)
{
  if(!list)  return md_strdup("");
  char* r=md_calloc(list->offset+list->len+3);
  for(const grammar_atom_t* u=list;u;u=u->next)
  {
    memcpy(r+u->offset,u->val,u->len);
    r[u->offset+u->len]='\n';
  }
  return r;
}


static void print_string_literal(FILE *f,const char* s)
{
  if(!s || !f)  return;

  static const char* pat="abfnrtv";
  static const char* rev="\a\b\f\n\r\t\v";

  fputc('"',f);
  while(*s)
  {
    char* t=strchr(rev,*s);
    if(t)
    {
      fputc('\\',f);
      fputc(pat[t-rev],f);
      s++;
      continue;
    }
    switch(*s)
    {
      case '\\':
      case '"':
        fputc('\\',f);
      default:
        fputc(*s++,f);
    }
  }
  fputc('"',f);
}



static int grammar_atom_list_str2(const grammar_atom_t* list,FILE *f)
{
  if(!list)  return -1;
  const grammar_atom_t** rev=md_pcalloc(list->order+1);

  for(const grammar_atom_t* u=list;u;u=u->next)
    rev[u->order]=u;

  for(size_t i=0;i<=list->order;i++)
    if(rev[i])
      switch(rev[i]->type)
      {
        case grammar_atom_string:
           fputc(' ',f);
           print_string_literal(f,rev[i]->val);
          continue;

        case grammar_atom_id:
          fprintf(f," %s",rev[i]->val);
          continue;
      }
  md_free(rev);
  return 0;
}


int grammar_add_code(grammar_t* g,const char* code)
{
  return (!g || !code) ? -1 : grammar_string_append(g->code,code);
}



grammar_string_t* grammar_string_init(size_t init)
{
  if(!init)  init=DEF_STRING_BLOCK;
  grammar_string_t* rv=md_new(rv);
  rv->buf=md_calloc(init);
  rv->allocated=init;
  return rv;
}

void grammar_string_free(grammar_string_t* s)
{
  if(!s)  return;
  md_free(s->buf);
  md_free(s);
}


const char* grammar_string_plain(grammar_string_t* s)
{
  if(!s)  return 0;
  if(s->len==s->allocated)
  {
    s->allocated+=DEF_STRING_BLOCK;
    s->buf=md_realloc(s->buf,s->allocated);
  }
  s->buf[s->len]=0;
  return s->buf;
}

int grammar_string_append(grammar_string_t* s,const char* d)
{
  if(!s)  return -1;
  if(!d || !*d)  return 0;
  size_t z=strlen(d);

  if(s->len+z>s->allocated)
  {
    s->allocated=(1+(s->len+z)/DEF_STRING_BLOCK)*DEF_STRING_BLOCK;
    s->buf=md_realloc(s->buf,s->allocated);
  }
  memcpy(s->buf+s->len,d,z);
  s->len+=z;
  return 0;
}

int grammar_string_append0(grammar_string_t* s,char c)
{
  if(!s)  return -1;

  if(s->len==s->allocated)
  {
    s->allocated+=DEF_STRING_BLOCK;
    s->buf=md_realloc(s->buf,s->allocated);
  }
  s->buf[s->len++]=c;
  return 0;
}

extern FILE* yyin;

static const char* usage="Usage: llull [options]\nUse llull -h for help\n";

static const char* help=
"llull - code generator designed for grammatical evolution systems\n"
"\t-v\t\tversion\n"
"\t-h\t\tthis help\n"
"\t-i input\tinput grammar file\n"
"\t-o folder\toutput directory for generated code\n"
"\t-n\t\tskip prototype file creation\n"
"\t-u uuid\t\toverride uuid\n"
"\t-d file\t\tdump grammar to specified file\n"
;


int main(int ac,char** av)
{
  opterr=0;
  const char* output=0;
  const char* input=0;
  const char* uuid=0;
  const char* dump=0;
  int proto=1;

  int c;

  while((c=getopt(ac,av,"hvni:o:u:d:"))!=-1)
    switch(c)
    {
      case 'h':
        fputs(help,stderr);
        return 0;

      case 'v':
        fputs("Llull v" LLULL_VERSION "\n",stderr);
        return 0;

      case 'n':
        proto=0;
        continue;

      case 'u':
        uuid=optarg;
        continue;

      case 'o':
        output=optarg;
        continue;

      case 'd':
        dump=optarg;
        continue;

      case 'i':
        input=optarg;
        continue;

      default:
        fprintf(stderr,"unknoun option -%c\n",c);
        fputs(usage,stderr);
        return 1;
    }

  if(!output)
  {
    fputs("output folder not set, using ./llull.gen/\n",stderr);
    output=DEF_OUTPUT;
  }

  yyin= input ? fopen(input, "r") : stdin;

  if(!yyin) crash(input);

  grammar_t* g=grammar_init();
  yyparse(g);

  if(uuid)
  {
    md_free(g->uuid);
    g->uuid=md_strdup(uuid);
  }

  if(!g->uuid)
  {
    char tmp[UUID_STR_LEN]={0,};
    uuid_t u;
    uuid_generate_random(u);
    uuid_unparse_lower(u,tmp);
    g->uuid=md_strdup(tmp);
  }

  if(dump)
  {
    FILE* f=fopen(dump,"w");
    if(f)
    {
      if(grammar_dump(g,f)) crash("Grammar dump failed");
      fclose(f);
    }
  }

  if(grammar_check(g))  crash("invalid grammar structure");
  grammar_codegen(g,output,proto ? CODEGEN_FLAG_PROTOTYPE : 0);

  grammar_free(g);
  if(input) fclose(yyin);
  return 0;
}
