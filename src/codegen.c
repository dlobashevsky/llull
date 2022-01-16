#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include <ctype.h>
#include <math.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "macros.h"
#include "llull.h"
#include "codegen.h"

#define DEF_CODON	"uint16_t"

#define DEPTH_INFIITY	-2
#define DEPTH_NOTINIT	-1


typedef struct codegen_func_t
{
  char* name;
  char* signature;
  char* ret;
  UT_hash_handle hh;
} codegen_func_t;


static grammar_atom_t* atoms_reverse(grammar_atom_t* p)
{
  if(!p) return 0;
  grammar_atom_t *cur,*prev=0,*next;

  for(cur=p;cur;)
  {
    next=cur->next;
    cur->next=prev;
    prev=cur;
    cur=next;
  }

  return prev;
}

static void hat(FILE* f,grammar_t* g)
{
  time_t tm=time(0);
  fprintf(f,"/*\n  Generated at:\t%s  Grammar name:\t%s\n  Grammar UUID:\t%s\n*/\n\n",
    ctime(&tm),g->name,g->uuid);
}

static void update_case(grammar_t* g,grammar_case_t* c,const char* type,const char* codon)
{
  c->args_count=0;
  if(c->atoms)
  {
    c->args=md_tcalloc(grammar_arg_t,c->atoms->order+1);
    c->atoms=atoms_reverse(c->atoms);
    for(grammar_atom_t *atom=c->atoms;atom;atom=atom->next)
      if(atom->type==grammar_atom_id)
      {
        c->args[c->args_count].no=c->args_count;
        c->args[c->args_count].name=atom->val;

        grammar_var_hash_t* v=0;
        HASH_FIND(hh,g->vars,atom->val,strlen(atom->val),v);
        if(v)
        {
          atom->var=c->args[c->args_count].var=1;
          atom->varno=c->args[c->args_count].varno=v->order;
          c->args[c->args_count].type=codon;
        }
        else
        {
          grammar_rules_hash_t* r=0;
          HASH_FIND(hh,g->rules,atom->val,strlen(atom->val),r);
          if(!r)
          {
            logger("cant found reference <%s>",atom->val);
            crash("");
          }
          c->args[c->args_count].type=r->type;
          c->args[c->args_count].var=0;
        }

        c->args_count++;
      }
  }
// update signature
  c->signature=0;

  size_t allocated=0;
  FILE *f=open_memstream(&c->signature,&allocated);
  if(!f)  crash("internal memory allocation error");

  fprintf(f,"%s _ctx",g->context);

  if(c->var)
  {
    grammar_var_hash_t* v=0;
    HASH_FIND(hh,g->vars,c->var,strlen(c->var),v);
    if(!v)  crash("repeat variable not found");
    c->varno=v->order;
    fprintf(f,", const %s__codon_t _arg0",g->name);
  }
  for(size_t i=0;i<c->args_count;i++)
    fprintf(f,", %s%s _arg%zu",c->args[i].type,c->var ? "*" : "",i+1);

  fclose(f);
}


typedef struct atom_depth_hash_t
{
  uint32_t point[3];
  int32_t depth[2];
  int visited;
  UT_hash_handle hh;
} atom_depth_hash_t;

atom_depth_hash_t* atom_depth_hash=0;




static int update_case_depth(grammar_rules_hash_t* r,grammar_case_t* c)
{
  if(c->depth[0]!=DEPTH_NOTINIT && c->depth[1]!=DEPTH_NOTINIT)  return 0;

  int32_t depth[2]={0,0};

  for(grammar_atom_t *a=c->atoms;a;a=a->next)
  {
    uint32_t p[3]={r->no,c->order,a->order};
    if(a->type!=grammar_atom_id) continue;

    atom_depth_hash_t* l=0;
    HASH_FIND(hh,atom_depth_hash,p,sizeof(p),l);
    if(!l) crash("cant find reference");

    if(l->depth[0]==DEPTH_NOTINIT || l->depth[1]==DEPTH_NOTINIT)
    {
    // update rule
//      if(l->visited) l->depth[1]=DEPTH_INIFINITY;
      
    }
    else
    {
//      if(l->depth
    }

  }

  return 0;
}

static int update_depth(grammar_t* g)
{
  // build reverse graph

/*
typedef struct graph_nodeinfo_t
{
  uint32_t rfrom;
  uint32_t cfrom;
  uint32_t rto;
  uint32_t cto;
} graph_nodeinfo_t;

typedef struct graph_node_t
{
  graph_nodeinfo_t key;
  int32_t min;
  int32_t max;
  uint8_t visited;
  UT_hash_handle hh;
} graph_node_t;

//fill graph

*/
  for(grammar_rules_hash_t* rules=g->rules;rules;rules=rules->hh.next)
  {
logger("rule <%s>:<%s> %zd",rules->name,rules->type,rules->no);
    rules->depth[0]=rules->depth[1]=DEPTH_NOTINIT;
    for(grammar_case_t* c=rules->cases;c;c=c->next)
    {
logger("\tcase %zd,%zd <%s> %s",c->order,c->varno,c->var,c->ref);
      c->depth[0]=c->depth[1]=DEPTH_NOTINIT;
      for(grammar_atom_t *atom=c->atoms;atom;atom=atom->next)
      {

logger("\t\tatom %zd,%zd <%s>",atom->order,atom->offset,atom->val);
        atom_depth_hash_t* l=md_new(l);
        l->point[0]=rules->no;
        l->point[1]=c->order;
        l->point[2]=atom->order;

        if(atom->type==grammar_atom_id)
        {
          l->depth[0]=l->depth[1]=DEPTH_NOTINIT;
          if(atom->var) l->depth[0]=l->depth[1]=1;
/*
          if(atom->var)
          {
            if(minmax[0]>=0)  minmax[0]++;
            continue;
          }
*/
        }
        HASH_ADD_KEYPTR(hh,atom_depth_hash,l->point,sizeof(l->point),l);
      }
    }
  }

  for(grammar_rules_hash_t* r=g->rules;r;r=r->hh.next)
    for(grammar_case_t* c=r->cases;c;c=c->next)
      update_case_depth(r,c);


  while(atom_depth_hash)
  {
    atom_depth_hash_t* l=atom_depth_hash;
    HASH_DEL(atom_depth_hash,l);
    md_free(l);
  }

  return 0;
}

/*
generate
*/

int grammar_codegen(grammar_t* g,const char* out,uint32_t flags)
{
  if(!g || !out)  return -1;

  if(mkdir(out,0777)==-1 && errno!=EEXIST)  crash("can not create output folder");

  int proto=(flags&CODEGEN_FLAG_PROTOTYPE);
  if(!g->codon) g->codon=md_strdup(DEF_CODON);

  codegen_func_t* funcs=0;
  char* codon=md_asprintf("%s__codon_t",g->name);

  for(grammar_rules_hash_t* rules=g->rules;rules;rules=rules->hh.next)
    for(grammar_case_t* c=rules->cases;c;c=c->next)
    {
      update_case(g,c,rules->type,codon);
// prepare signature and args

      {
        if(c->flags!=GRAMMAR_CASE_FLAG_FUNC)  continue;
        codegen_func_t* r=0;
// generate signature
        char* sign=c->signature;
        HASH_FIND(hh,funcs,c->ref,strlen(c->ref),r);
        if(r)			// check signature
        {
          if(strcmp(sign,r->signature) || strcmp(rules->type,r->ret))
          {
            logger("signatures not compatible for function %s",c->ref);
            crash("leave");
          }
        }
        else
        {
          r=md_new(r);
          r->name=c->ref;
          r->signature=sign;
          r->ret=rules->type;
          HASH_ADD_KEYPTR(hh,funcs,r->name,strlen(r->name),r);
        }
      }
    }

    update_depth(g);

    if(proto && !funcs) proto=0;

    if(proto)
    {
      char* fn=md_asprintf("%s/%s__proto.h",out,g->name);
      FILE* fproto=fopen(fn,"w");
      if(!fproto)  crash("cant create proto file");
      md_free(fn);
      hat(fproto,g);
      fprintf(fproto,"\n\n");
      for(codegen_func_t* r=funcs;r;r=r->hh.next)
        fprintf(fproto,"%s %s__%s(%s,int* _err);\n",r->ret,g->name,r->name,r->signature);

      fprintf(fproto,"\n\n");
      fclose(fproto);
    }

  char* fn=md_asprintf("%s/%s.h",out,g->name);
  FILE* fh=fopen(fn,"w");
  md_free(fn);
  fn=md_asprintf("%s/%s.c",out,g->name);
  FILE* fc=fopen(fn,"w");
  md_free(fn);
  if(!fh || !fc)
    crash("cant create target files");

  hat(fh,g);

// TODO: place custom allocators code

  size_t total_vars=0;
  for(grammar_var_hash_t* v=g->vars;v;v=v->hh.next)  total_vars++;

  char* upper=md_strdup(g->name);
  for(char* p=upper;*p;p++)
    *p=toupper(*p);

  fprintf(fh,"\n\n#define %s__VARS\t%zu\n\ntypedef %s %s__codon_t;\ntypedef %s %s__fitness_t;\n\n",
             upper,total_vars,g->codon,g->name,g->fitness ?: "float",g->name);


  if(total_vars)
  {
    fprintf(fh,"typedef enum %s__vars_t\n{\n",g->name);
    for(grammar_var_hash_t* v=g->vars;v;v=v->hh.next)
      fprintf(fh,"  %s__vars__%s=%zu,\n",g->name,v->name,v->order);

    fprintf(fh,"} %s__vars_t;\n\n",g->name);
  }

  {
    fprintf(fh,"typedef enum %s__nodes_t\n{\n",g->name);
    size_t order=0;
    for(grammar_rules_hash_t* rules=g->rules;rules;rules=rules->hh.next)
    {
      fprintf(fh,"  %s__nodes__%s=%zu,\t// node type <%s>\n",g->name,rules->name,order++,rules->type);
    }
    fprintf(fh,"} %s__nodes_t;\n\n",g->name);
  }

  const char* final=0;
  {
    grammar_rules_hash_t* r=0;
    HASH_FIND(hh,g->rules,g->init,strlen(g->init),r);
    if(!r)
      crash("init rule not found");
    final=r->type;
  }


  fprintf(fh,"struct %1$s__def_seq_t;\nstruct %1$s__def_rule_t;\n\ntypedef struct %1$s__chromosome_t\n{\n"
             "  size_t len;\n  %1$s__fitness_t fitness;\n  %1$s__codon_t data[0];\n} %1$s__chromosome_t;\n\n"
             "typedef struct %1$s__t\n{\n  const char* uuid;\n  const char* name;\n  size_t vars_count;\n  %1$s__codon_t* vars_min;\n"
             "  %1$s__codon_t* vars_max;\n  double* vars_mdl;\n  uint32_t wrap;\n  const struct %1$s__def_seq_t* seq;\n"
             "  const struct %1$s__def_rule_t* rules;\n} %1$s__t;\n\n"
             "//! %1$s chromosome constructor, all positions are zeroed\n%1$s__chromosome_t* %1$s__chromosome_init0(const %1$s__t*,size_t sz);\n"
             "//! %1$s chromosome constructor, filled by random data, rng safe to NULL\n%1$s__chromosome_t* %1$s__chromosome_init(const %1$s__t*,size_t sz,uint32_t (*rng)(void*),void* rng_state);\n"
             "//! %1$s chromosome destructor\nvoid %1$s__chromosome_free(%1$s__chromosome_t*);\n\n"
             "//! return 0 if chromosome may be successfully parsed, wrap_count should be>0\n"
             "int %1$s__chromosome_check(const %1$s__t*,const %1$s__chromosome_t*);\n\n"
             "//! dump to string, returned string must be freed by caller\nchar* %1$s__chromosome_dumps(const %1$s__t*,const %1$s__chromosome_t*);\n"
             "//! dump to FILE\nint %1$s__chromosome_dump(const %1$s__t*,const %1$s__chromosome_t*,FILE*);\n\n"
             "//! serialize data, return blob size\nint32_t %1$s__chromosome_save(const %1$s__t*,const %1$s__chromosome_t*,FILE*);\n"
             "//!< return size of serializable data\nssize_t %1$s__chromosome_save_size(const %1$s__t*,const %1$s__chromosome_t*);\n"
             "//!< deserialize\n%1$s__chromosome_t* %1$s__chromosome_load(const %1$s__t*,FILE*);\n\n"
             "//! run chromosome computation, return value of root\n"
             "int %1$s__chromosome_compute(const %1$s__t*,const %1$s__chromosome_t*,%2$s ctx,%3$s* res);\n"
             "//! return sum of log(bit size) for MDL computation or parsimony pressure, -1 if error\n"
             "double %1$s__chromosome_bits(const %1$s__t*,const %1$s__chromosome_t*);\n\n"
             "//! mutate of single codon, validity not guaranteed\n"
             "%1$s__chromosome_t* %1$s__chromosome_mutate(const %1$s__chromosome_t*,size_t offset,%1$s__codon_t new);\n"
             "//! inplace random mutate of single codon, validity not guaranteed\n"
             "%1$s__chromosome_t* %1$s__chromosome_mutate_inplace(%1$s__chromosome_t*,size_t offset,%1$s__codon_t new);\n"
             "//! uniform crossover, validity of childs not guaranteed\n"
             "int %1$s__chromosome_uniform(const %1$s__chromosome_t*,const %1$s__chromosome_t*,size_t offset,%1$s__chromosome_t**,%1$s__chromosome_t**);\n"
             "//! inplace uniform crossover, resulting chromosomes may be broken\n"
             "int %1$s__chromosome_uniform_inplace(%1$s__chromosome_t*,%1$s__chromosome_t*,size_t offset);\n\n"
             "//! init definition structure\n%1$s__t* %1$s__init(size_t wrap);\n"
             "//! free definition structure\nvoid %1$s__free(%1$s__t*);\n\n"
             "//! set variable limits\nint %1$s__setvar(%1$s__t*,size_t varidx,uintmax_t min,uintmax_t max);\n\n"
             "//! dump definition structure to string\nchar* %1$s__dumps(const %1$s__t*);\n"
             "//! dump definition structure to stream\nint %1$s__dump(const %1$s__t*,FILE*);\n"
             "//! serialize definition structure in stream\nint32_t %1$s__save(const %1$s__t*,FILE*);\n"
             "//! compute size of serialized definition structure\nint32_t %1$s__save_size(const %1$s__t*);\n"
             "//! deserialize %1$s definition\n%1$s__t* %1$s__load(FILE*);\n\n"
             "//! get uuid\nconst char* %1$s__sys_uuid(void);\n"
             "//! get name\nconst char* %1$s__sys_name(void);\n\n",
             g->name,g->context,final);

  fclose(fh);

  fprintf(fc,"#include <stdio.h>\n#include <stdlib.h>\n#include <stdint.h>\n#include <stdlib.h>\n#include <string.h>\n#include <math.h>\n\n");
  hat(fc,g);
  fprintf(fc,"%s\n",grammar_string_plain(g->code));
  if(proto)  fprintf(fc,"#include \"%s__proto.h\"\n\n",g->name);

/*
  fprintf(fc,"static inline uint32_t local_hash(uint32_t x)\n{\n  x=((x >> 16)^ x)*0x45d9f3bU;"
             "  x=((x >> 16)^x)*0x45d9f3bU;\n  x=(x >> 16)^x;\n  return x;\n}\n\n");
*/

  fprintf(fc,"static const char* %1$s__uuid=\"%3$s\";\nstatic const char* %1$s__name=\"%1$s\";\n\n"
             "typedef int %1$s__chromosome_check_t(const %1$s__t* def,const %1$s__chromosome_t* c,uint32_t* p);\n"
             "typedef int %1$s__chromosome_dump_t(const %1$s__t* def,const %1$s__chromosome_t* c,uint32_t* p,FILE *f);\n"
             "typedef double %1$s__chromosome_bits_t(const %1$s__t* def,const %1$s__chromosome_t* c,uint32_t* p);\n"
             "typedef int %1$s__chromosome_val_t(const %1$s__t* def,const %1$s__chromosome_t* c,%2$s ctx,uint32_t* p,void *res);\n\n",
             g->name,g->context,g->uuid);


  fprintf(fc,"static int %1$s__chromosome_varcheck(const %1$s__t* def,const %1$s__chromosome_t* c,uint32_t* p);\n",g->name);
  fprintf(fc,"static int %1$s__chromosome_vardump(const %1$s__t* def,const %1$s__chromosome_t* c,uint32_t* p,size_t v,FILE* f);\n",g->name);
  fprintf(fc,"static %1$s__codon_t %1$s__chromosome_varbits(const %1$s__t* def,const %1$s__chromosome_t* c,uint32_t* p,size_t v);\n",g->name);
  fprintf(fc,"static int %1$s__chromosome_varget(const %1$s__t* def,const %1$s__chromosome_t* c,uint32_t* p,size_t v,intmax_t* arg);\n",g->name);

// check routines
  for(grammar_rules_hash_t* rules=g->rules;rules;rules=rules->hh.next)
  {
    fprintf(fc,"\nstatic int %1$s__chromosome_check__%2$s(const %1$s__t* def,const %1$s__chromosome_t* c,uint32_t* p);\n",g->name,rules->name);

    for(grammar_case_t* c=rules->cases;c;c=c->next)
      fprintf(fc,"static int %1$s__chromosome_check__%2$s__%3$zu(const %1$s__t* def,const %1$s__chromosome_t* c,uint32_t* p);\n",g->name,rules->name,c->order+1);
  }

// dump routines

  for(grammar_rules_hash_t* rules=g->rules;rules;rules=rules->hh.next)
  {
    fprintf(fc,"\nstatic int %1$s__chromosome_dump__%2$s(const %1$s__t* def,const %1$s__chromosome_t* c,uint32_t* p,FILE* f);\n",g->name,rules->name);

    for(grammar_case_t* c=rules->cases;c;c=c->next)
      fprintf(fc,"static int %1$s__chromosome_dump__%2$s__%3$zu(const %1$s__t* def,const %1$s__chromosome_t* c,uint32_t* p,FILE* f);\n",g->name,rules->name,c->order+1);
  }

// bits routines
  for(grammar_rules_hash_t* rules=g->rules;rules;rules=rules->hh.next)
  {
    fprintf(fc,"\nstatic double %1$s__chromosome_bits__%2$s(const %1$s__t* def,const %1$s__chromosome_t* c,uint32_t* p);\n",g->name,rules->name);

    for(grammar_case_t* c=rules->cases;c;c=c->next)
      fprintf(fc,"static double %1$s__chromosome_bits__%2$s__%3$zu(const %1$s__t* def,const %1$s__chromosome_t* c,uint32_t* p);\n",g->name,rules->name,c->order+1);
  }

// val routines

  for(grammar_rules_hash_t* rules=g->rules;rules;rules=rules->hh.next)
  {
    fprintf(fc,"\nstatic int %1$s__chromosome_val__%2$s(const %1$s__t* def,const %1$s__chromosome_t* c,%3$s ctx,uint32_t* p,%4$s* arg);\n",g->name,rules->name,g->context,rules->type);

    for(grammar_case_t* c=rules->cases;c;c=c->next)
      fprintf(fc,"static int %1$s__chromosome_val__%2$s__%3$zu(const %1$s__t* def,const %1$s__chromosome_t* c,%4$s ctx,uint32_t* p,void* arg);\n",g->name,rules->name,c->order+1,g->context);
  }
/*
  fprintf(fc,"\n\ntypedef struct %1$s__def_seq_t\n{\n  uint32_t arity;\n  %1$s__chromosome_check_t* check;\n  %1$s__chromosome_dump_t* dump;\n"
             "  %1$s__chromosome_bits_t* bits;\n  %1$s__chromosome_val_t* func;\n\n  int32_t depth[2];\n\n  const int32_t* args;\n} %1$s__def_seq_t;\n\n",
             g->name);

  fprintf(fc,"typedef struct %1$s__def_rule_t\n{\n  uint32_t choices;\n  uint32_t off;\n  int32_t depth[2];\n  double mdl;\n} %1$s__def_rule_t;\n\n",
             g->name);

*/
  fprintf(fc,"\n\ntypedef struct %1$s__def_seq_t\n{\n  uint32_t arity;\n  %1$s__chromosome_check_t* check;\n  %1$s__chromosome_dump_t* dump;\n"
             "  %1$s__chromosome_bits_t* bits;\n  %1$s__chromosome_val_t* func;\n\n  int32_t depth[2];\n} %1$s__def_seq_t;\n\n",
             g->name);
  fprintf(fc,"typedef struct %1$s__def_rule_t\n{\n  uint32_t choices;\n  uint32_t off;\n  int32_t depth[2];\n  double mdl;\n} %1$s__def_rule_t;\n\n",
             g->name);

  fprintf(fc,"static const %1$s__def_seq_t %1$s_seq[]=\n{\n",
             g->name);

//TODO: update depth, calculate graph cycles

  for(grammar_rules_hash_t* rules=g->rules;rules;rules=rules->hh.next)
  {
    fprintf(fc,"// '%s' rule type '%s', \n",rules->name,rules->type);

    for(grammar_case_t* c=rules->cases;c;c=c->next)
      fprintf(fc,"  {%4$zu,%1$s__chromosome_check__%2$s__%3$zu,%1$s__chromosome_dump__%2$s__%3$zu,%1$s__chromosome_bits__%2$s__%3$zu,"
                 "%1$s__chromosome_val__%2$s__%3$zu,{%5$d,%6$d}},\n",
                  g->name,rules->name,c->order+1,c->args_count,c->depth[0],c->depth[1]);
  }

  fprintf(fc,"};\n\nstatic const %1$s__def_rule_t %1$s__rules[]=\n{\n",g->name);

  size_t off=0;
  for(grammar_rules_hash_t* rules=g->rules;rules;rules=rules->hh.next)
  {
    size_t choices=rules->cases->order+1;
    double mdl=log2(choices);
    int32_t depth[2]={INT32_MAX,0};
    for(grammar_case_t* c=rules->cases;c;c=c->next)
    {
      if(c->depth[0]<depth[0])  depth[0]=c->depth[0];
      if(depth[1]<0)  continue;
      if(c->depth[1]<0)  depth[1]=-1;
      else
        if(c->depth[1]>depth[1])  depth[1]=c->depth[1];
    }

    fprintf(fc,"  {%zu,%zu,{%d,%d},%g},\n",choices,off,depth[0],depth[1],mdl);
    off+=choices;
  }

  fprintf(fc,"};\n\n");

  fprintf(fc,"%1$s__t* %1$s__init(size_t wrap)\n{\n  if(!wrap)  return 0;\n  %1$s__t* rv=md_new(rv);\n  rv->uuid=%1$s__uuid;\n"
             "  rv->name=%1$s__name;\n  rv->vars_count=%2$s__VARS;\n  rv->seq=%1$s_seq;\n  rv->rules=%1$s__rules;\n  rv->wrap=wrap;\n\n"
             "  rv->vars_min=md_tcalloc(%1$s__codon_t,rv->vars_count);\n  rv->vars_max=md_tcalloc(%1$s__codon_t,rv->vars_count);"
             "  rv->vars_mdl=md_tcalloc(double,rv->vars_count);\n\n//  set default values for vars\n\n",
             g->name,upper);

  size_t i=0;
  for(grammar_var_hash_t* v=g->vars;v;v=v->hh.next,i++)
    fprintf(fc,"  rv->vars_min[%1$zu]=%2$ju;\n  rv->vars_max[%1$zu]=%3$ju;\n"
               "  if(rv->vars_max[%1$zu]!=rv->vars_min[%1$zu]) rv->vars_mdl[%1$zu]=log2(fabs(rv->vars_max[%1$zu]-rv->vars_min[%1$zu]));\n",
               v->order,v->min,v->max);

  fprintf(fc,"  return rv;\n}\n\nvoid %1$s__free(%1$s__t* def)\n{\n  if(!def)  return;\n  free(def->vars_min);\n  free(def->vars_max);\n"
             "  free(def->vars_mdl);\n  free(def);\n}\n\n",
             g->name);


  fprintf(fc,"int %1$s__setvar(%1$s__t* def,size_t varidx,uintmax_t min,uintmax_t max)\n{\n\n"
             "  if(!def || varidx>=def->vars_count || max<min)  return -1;\n"
             "  def->vars_min[varidx]=min;\n  def->vars_max[varidx]=max;\n"
             "  def->vars_mdl[varidx]= (max==min) ? 0 : log2(max-min);\n  return 0;\n}\n\n"
             "char* %1$s_dumps(const %1$s__t* def)\n{\n  if(!def)  return 0;\n  size_t sz=0;\n"
             "  char* rv=0;\n  FILE* f=open_memstream(&rv,&sz);\n  if(%1$s__dump(def,f))\n  {\n    fclose(f);\n    free(rv);"
             "    return 0;\n  }\n  fclose(f);\n  return rv;\n}\n\n",
             g->name);


  fprintf(fc,"int %1$s__dump(const %1$s__t* def,FILE* f)\n{\n  if(!def || !f)  return -1;\n  fprintf(f,\"name: %%s\\nuuid: %%s\\n\",def->name,def->uuid);\n"
             "  fprintf(f,\"wrap: %%u\\n\",def->wrap);\n  fprintf(f,\"variables: %%zu\\n\",def->vars_count);\n  for(size_t i=0;i<def->vars_count;i++)\n"
             "    fprintf(f,\" %%zd\\t%%jd : %%jd\\t%%g\\n\",i,(intmax_t)(def->vars_min[i]),(intmax_t)(def->vars_max[i]),def->vars_mdl[i]);\n\n"
             "  return 0;\n}\n\nint32_t %1$s__save_size(const %1$s__t* def)\n{\n  if(!def)  return -1;\n  int32_t ret=strlen(def->name)+1;\n"
             "  ret+=strlen(def->uuid)+1;\n  ret+=4*sizeof(uint32_t);\n  ret+=(sizeof(%1$s__codon_t)*2+sizeof(double))*def->vars_count;\n\n"
             "  return ret;\n}\n\nint32_t %1$s__save(const %1$s__t* def,FILE* f)\n{\n  if(!def)  return -1;\n  int32_t rv=%1$s__save_size(def);\n"
             "  uint32_t l=strlen(def->name)+1;\n  fwrite(&l,sizeof(l),1,f);\n  fwrite(def->name,l,1,f);\n  l=strlen(def->uuid)+1;\n"
             "  fwrite(&l,sizeof(l),1,f);  fwrite(def->uuid,l,1,f);\n  uint32_t cnt=def->vars_count;\n  fwrite(&cnt,sizeof(cnt),1,f);\n"
             "  cnt=def->wrap;\n  fwrite(&cnt,sizeof(cnt),1,f);\n\n  fwrite(def->vars_min,sizeof(def->vars_min[0])*def->vars_count,1,f);\n"
             "  fwrite(def->vars_max,sizeof(def->vars_max[0])*def->vars_count,1,f);\n  fwrite(def->vars_mdl,sizeof(def->vars_mdl[0])*def->vars_count,1,f);"
             "\n\n  return rv;\n}\n\n",
             g->name);

  fprintf(fc,"%1$s__t* %1$s__load(FILE* f)\n{\n  if(!f)  return 0;\n  uint32_t l=0;\n  fread(&l,sizeof(l),1,f);\n"
             "  char* tmp=calloc(1,l);\n  fread(tmp,l,1,f);\n  if(tmp[l-1] || strcmp(tmp,%1$s__name))\n  {\n"
             "    free(tmp);    return 0;\n  }\n  free(tmp);\n  fread(&l,sizeof(l),1,f);\n  tmp=calloc(1,l);\n"
             "  fread(tmp,l,1,f);\n  if(tmp[l-1] || strcmp(tmp,%1$s__uuid))\n  {\n    free(tmp);\n    return 0;\n"
             "  }\n\n  free(tmp);\n  fread(&l,sizeof(l),1,f);\n  if(l!=%2$s__VARS)  return 0;\n  fread(&l,sizeof(l),1,f);\n"
             "  %1$s__t* rv=%1$s__init(l);\n  if(l)\n  {\n    fread(rv->vars_min,sizeof(rv->vars_min[0])*rv->vars_count,1,f);\n"
             "    fread(rv->vars_max,sizeof(rv->vars_max[0])*rv->vars_count,1,f);\n    fread(rv->vars_mdl,sizeof(rv->vars_mdl[0])*rv->vars_count,1,f);\n"
             "  }\n  return rv;\n}\n\nvoid %1$s__chromosome_free(%1$s__chromosome_t* c)\n{\n  md_free(c);\n}\n\n"
             "%1$s__chromosome_t* %1$s__chromosome_init0(const %1$s__t* def,size_t sz)\n"
             "{\n  if(!def || !sz)  return 0;\n  %1$s__chromosome_t* rv=md_calloc(sizeof(*rv)+sz*sizeof(%1$s__codon_t));\n"
             "  rv->len=sz;\n\n  return rv;\n}\n\n"
             "%1$s__chromosome_t* %1$s__chromosome_init(const %1$s__t* def,size_t sz,uint32_t (*rng)(void*),void* rng_state)\n"
             "{\n  if(!def || !sz)  return 0;\n  %1$s__chromosome_t* rv=md_calloc(sizeof(*rv)+sz*sizeof(%1$s__codon_t));\n"
             "  rv->len=sz;\n\n",
             g->name,upper);//,g->rng_ctx ? "," : "",g->rng_ctx ?: "",g->rng_ctx ? " r": "");

/*
  if(g->rng)
    fprintf(fc,"  %s(%srv->data,sz*sizeof(%s__codon_t));\n",g->rng,g->rng_ctx ? "r," : "",g->name);
  else
    fprintf(fc,"  for(size_t i=0;i<rv->len*sizeof(%s__codon_t);i++)  ((uint8_t*)((void*)rv->data))[i]=rand()&0xff;\n",g->name);
*/

  fprintf(fc,"  if(!rng)\n    for(size_t i=0;i<rv->len*sizeof(%1$s__codon_t);i++)  ((uint8_t*)((void*)rv->data))[i]=rand()&0xff;\n"
             "  else\n    for(size_t i=0;i<rv->len*sizeof(%1$s__codon_t);i++)  ((uint8_t*)((void*)rv->data))[i]=(*rng)(rng_state)&0xff;\n",
             g->name);


  fprintf(fc,"  if(!%1$s__chromosome_check(def,rv))  return rv;\n  %1$s__chromosome_free(rv);\n  return 0;\n}\n\n"
             "static intmax_t getnext(const %1$s__chromosome_t* c,uint32_t* p,size_t wrap)\n"
             "{\n  if(p[0]>=c->len*wrap)  return -1;\n\n  return c->data[(p[0]++)%%c->len];\n}\n\n",
             g->name);


// check recursion
  off=0;
  for(grammar_rules_hash_t* rules=g->rules;rules;rules=rules->hh.next)
  {
    fprintf(fc,"static int %1$s__chromosome_check__%2$s(const %1$s__t* def,const %1$s__chromosome_t* c,uint32_t* p)\n"
               "{\n  intmax_t n=getnext(c,p,def->wrap);\n  if(n<0) return -1;\n  return def->seq[%4$zu+(n%%%3$zu)].check(def,c,p);\n}\n\n",
               g->name,rules->name,rules->cases->order+1,off);

    for(grammar_case_t* c=rules->cases;c;c=c->next)
    {
      fprintf(fc,"static int %1$s__chromosome_check__%2$s__%3$zu(const %1$s__t* def,const %1$s__chromosome_t* c,uint32_t* p)\n{\n",
                  g->name,rules->name,c->order+1);

      if(c->var)
      {
        fprintf(fc,"  intmax_t _a0=0;\n  if(%1$s__chromosome_varget(def,c,p,%2$zu,&_a0))  return -1;\n  _a0-=def->vars_min[%2$zu];\n"
                "  for(ssize_t i=0;i<_a0;i++)\n  {\n",
                g->name,c->varno);
        for(size_t i=0;i<c->args_count;i++)
        {
          if(c->args[i].var)
            fprintf(fc,"    if(%1$s__chromosome_varcheck(def,c,p))  return -1;\n",g->name);
          else
            fprintf(fc,"    if(%1$s__chromosome_check__%2$s(def,c,p))  return -1;\n",g->name,c->args[i].name);
        }
        fprintf(fc,"  }\n");
      }
      else
        for(size_t i=0;i<c->args_count;i++)
        {
          if(c->args[i].var)
            fprintf(fc,"  if(%1$s__chromosome_varcheck(def,c,p))  return -1;\n",g->name);
          else
            fprintf(fc,"  if(%1$s__chromosome_check__%2$s(def,c,p))  return -1;\n",g->name,c->args[i].name);
        }

      fprintf(fc,"  return 0;\n}\n\n");
    }
    off+=rules->cases->order+1;
  }

// bits recursion

  off=0;
  for(grammar_rules_hash_t* rules=g->rules;rules;rules=rules->hh.next)
  {
    fprintf(fc,"static double %1$s__chromosome_bits__%2$s(const %1$s__t* def,const %1$s__chromosome_t* c,uint32_t* p)\n"
               "{\n  intmax_t n=getnext(c,p,def->wrap);\n  if(n<0) return -1;\n"
               "  return def->rules[0].mdl+def->seq[%4$zu+(n%%%3$zu)].bits(def,c,p);\n}\n\n",
               g->name,rules->name,rules->cases->order+1,off);

    for(grammar_case_t* c=rules->cases;c;c=c->next)
    {
      if(c->var)
      {
        fprintf(fc,"static double %1$s__chromosome_bits__%2$s__%3$zu(const %1$s__t* def,const %1$s__chromosome_t* c,uint32_t* p)\n{\n"
                   "  intmax_t _a0=0;\n  if(%1$s__chromosome_varget(def,c,p,%4$zu,&_a0))  return -1;\n  _a0-=def->vars_min[%4$zu];\n"
                   "  double rv=0;\n  double b;\n\n  for(ssize_t i=0;i<_a0;i++)\n  {\n",
                    g->name,rules->name,c->order+1,c->varno);

        for(size_t i=0;i<c->args_count;i++)
        {
          if(c->args[i].var)
            fprintf(fc,"    if((b=%1$s__chromosome_varbits(def,c,p,%2$zu))<0)  return -1;\n    rv+=b;\n",g->name,c->args[i].varno);
          else
            fprintf(fc,"    if((b=%1$s__chromosome_bits__%2$s(def,c,p))<0)  return -1;\n    rv+=b;\n",g->name,c->args[i].name);
        }

        fprintf(fc,"  }\n");
      }
      else
      {
        if(!c->args_count)
        {
          fprintf(fc,"static double %1$s__chromosome_bits__%2$s__%3$zu(const %1$s__t* def,const %1$s__chromosome_t* c,uint32_t* p)\n"
                     "{\n  return 0;\n}\n\n",g->name,rules->name,c->order+1);
          continue;
        }
        fprintf(fc,"static double %1$s__chromosome_bits__%2$s__%3$zu(const %1$s__t* def,const %1$s__chromosome_t* c,uint32_t* p)\n"
                   "{\n  double rv=0;\n  double b;\n\n",
                   g->name,rules->name,c->order+1);


        for(size_t i=0;i<c->args_count;i++)
        {
          if(c->args[i].var)
            fprintf(fc,"  if((b=%1$s__chromosome_varbits(def,c,p,%2$zu))<0)  return -1;\n  rv+=b;\n",g->name,c->args[i].varno);
          else
            fprintf(fc,"  if((b=%1$s__chromosome_bits__%2$s(def,c,p))<0)  return -1;\n  rv+=b;\n",g->name,c->args[i].name);
        }
      }
      fprintf(fc,"\n  return rv;\n}\n\n");
    }
    off+=rules->cases->order+1;
  }


// dump recursion

  off=0;
  for(grammar_rules_hash_t* rules=g->rules;rules;rules=rules->hh.next)
  {
    fprintf(fc,"static int %1$s__chromosome_dump__%2$s(const %1$s__t* def,const %1$s__chromosome_t* c,uint32_t* p,FILE *f)\n"
               "{\n  intmax_t n=getnext(c,p,def->wrap);\n  if(n<0) return -1;\n"
               "  return def->seq[%4$zu+(n%%%3$zu)].dump(def,c,p,f);\n}\n\n",
               g->name,rules->name,rules->cases->order+1,off);

    for(grammar_case_t* c=rules->cases;c;c=c->next)
    {
      fprintf(fc,"static int %1$s__chromosome_dump__%2$s__%3$zu(const %1$s__t* def,const %1$s__chromosome_t* c,uint32_t* p,FILE* f)\n{\n",
                 g->name,rules->name,c->order+1);

      const char* indent="";
      if(c->var)
      {
        fprintf(fc,"  intmax_t _a0=0;\n  if(%1$s__chromosome_varget(def,c,p,%2$zu,&_a0))  return -1;\n"
                   "  _a0-=def->vars_min[%2$zu];\n  for(ssize_t i=0;i<_a0;i++)\n  {\n",
                    g->name,c->varno);

        indent="  ";
      }

      for(grammar_atom_t* atom=c->atoms;atom;atom=atom->next)
        switch(atom->type)
        {
            case grammar_atom_string:
              fprintf(fc,"%s  fprintf(f,\"%%s\",\"%s\");\n",indent,atom->val);
              continue;
            case grammar_atom_id:
              if(atom->var)
                fprintf(fc,"%s  if(%s__chromosome_vardump(def,c,p,%zu,f))  return -1;\n",indent,g->name,atom->varno);
              else
                fprintf(fc,"%s  if(%s__chromosome_dump__%s(def,c,p,f))  return -1;\n",indent,g->name,atom->val);
              continue;
        }

      if(c->var)  fprintf(fc,"  }\n");
      fprintf(fc,"  return 0;\n}\n\n");
    }
    off+=rules->cases->order+1;
  }


// compute recursion


  off=0;
  for(grammar_rules_hash_t* rules=g->rules;rules;rules=rules->hh.next)
  {
    fprintf(fc,"static int %1$s__chromosome_val__%2$s(const %1$s__t* def,const %1$s__chromosome_t* c,%5$s ctx,uint32_t* p,%6$s* arg)\n"
               "{\n  intmax_t n=getnext(c,p,def->wrap);\n  if(n<0) return -1;\n"
               "  return def->seq[%4$zu+(n%%%3$zu)].func(def,c,ctx,p,arg);\n}\n\n",
               g->name,rules->name,rules->cases->order+1,off,g->context,rules->type);


    for(grammar_case_t* c=rules->cases;c;c=c->next)
    {
      fprintf(fc,"static int %1$s__chromosome_val__%2$s__%3$zu(const %1$s__t* _def,const %1$s__chromosome_t* _c,%4$s _ctx,uint32_t* _p,void* _arg)\n"
                 "{\n  int _rv=0;\n\n",
                 g->name,rules->name,c->order+1,g->context);

      if(c->var)
      {
        fprintf(fc,"  intmax_t _a0=0;\n\n  if(%1$s__chromosome_varget(_def,_c,_p,%2$zu,&_a0))  return -1;\n  _a0-=_def->vars_min[%2$zu];\n\n",
                   g->name,c->varno);

        for(size_t i=0;i<c->args_count;i++)
          fprintf(fc,"  %s* _a%zu=md_tcalloc(%s,_a0);\n",c->args[i].var ? "intmax_t" : c->args[i].type,i+1,c->args[i].var ? "intmax_t" : c->args[i].type);
        fprintf(fc,"\n  for(ssize_t i=0;i<_a0;i++)\n  {\n");

        for(size_t i=0;i<c->args_count;i++)
          if(c->args[i].var)
            fprintf(fc,"    if(%s__chromosome_varget(_def,_c,_p,%zu,_a%zu+i))  goto _err;\n",g->name,c->args[i].varno,i+1);
          else
            fprintf(fc,"    if(%s__chromosome_val__%s(_def,_c,_ctx,_p,_a%zu+i)<0)  goto _err;\n",g->name,c->args[i].name,i+1);

        fprintf(fc,"  }\n\n");

        if(c->flags==GRAMMAR_CASE_FLAG_FUNC)
        {
          fprintf(fc,"  %s _r=%s__%s(_ctx,_a0,",rules->type,g->name,c->ref);
          for(size_t i=0;i<c->args_count;i++)
            fprintf(fc,"_a%zu,",i+1);

          fprintf(fc,"&_rv);\n  if(!_rv) *((%s*)_arg)=_r;\n\n",rules->type);
        }
        else
        {
          fprintf(fc,"  { %s }\n\n",c->ref);
          fprintf(fc,"  if(!_rv) *((%s*)_arg)=_r;\n\n",rules->type);
        }


        for(size_t i=0;i<c->args_count;i++)
          fprintf(fc,"  md_free(_a%zu);\n",i+1);

        fprintf(fc,"  return _rv;\n\n_err:\n");

        for(size_t i=0;i<c->args_count;i++)
          fprintf(fc,"  md_free(_a%zu);\n",i+1);

        fprintf(fc,"  return -1;\n}\n\n");
      }
      else
      {
        if(c->args_count)
        {
          for(size_t i=0;i<c->args_count;i++)
            fprintf(fc,"  %s _a%zu;\n",c->args[i].var ? "intmax_t" : c->args[i].type,i+1);
          fprintf(fc,"\n");

          for(size_t i=0;i<c->args_count;i++)
            if(c->args[i].var)
              fprintf(fc,"  if(%s__chromosome_varget(_def,_c,_p,%zu,&_a%zu))  return -1;\n",g->name,c->args[i].varno,i+1);
            else
              fprintf(fc,"  if(%s__chromosome_val__%s(_def,_c,_ctx,_p,&_a%zu)<0)  return -1;\n",g->name,c->args[i].name,i+1);
          fprintf(fc,"\n");
        }

        if(c->flags==GRAMMAR_CASE_FLAG_FUNC)
        {
          fprintf(fc,"  %s _r=%s__%s(_ctx,",rules->type,g->name,c->ref);
          for(size_t i=0;i<c->args_count;i++)
            fprintf(fc,"_a%zu,",i+1);

          fprintf(fc,"&_rv);\n  if(!_rv) *((%s*)_arg)=_r;\n",rules->type);
        }
        else
        {
          fprintf(fc,"  %s _r;\n  {",rules->type);
          fprintf(fc,"%s",c->ref);
          fprintf(fc,"}\n  if(!_rv) *((%s*)_arg)=_r;\n",rules->type);
        }
        fprintf(fc,"  return _rv;\n}\n\n");
      }
    }
    off+=rules->cases->order+1;
  }

// final section

  fprintf(fc,
    "static int %1$s__chromosome_varcheck(const %1$s__t* def,const %1$s__chromosome_t* c,uint32_t* p)\n"
    "{\n  return getnext(c,p,def->wrap)<0 ? -1 : 0;\n}\n\n"
    "static int %1$s__chromosome_vardump(const %1$s__t* def,const %1$s__chromosome_t* c,uint32_t* p,size_t v,FILE* f)\n"
    "{\n  intmax_t n=getnext(c,p,def->wrap);\n  if(n<0) return -1;\n  intmax_t min=def->vars_min[v];\n  intmax_t max=def->vars_max[v];\n"
    "  fprintf(f,\"%%jd\",min==max ? min : min+n%%(max-min));\n  return 0;\n}\n\n"
    "static %1$s__codon_t %1$s__chromosome_varbits(const %1$s__t* def,const %1$s__chromosome_t* c,uint32_t* p,size_t v)\n"
    "{\n  intmax_t n=getnext(c,p,def->wrap);\n  if(n<0) return -1;\n  intmax_t min=def->vars_min[v];\n  intmax_t max=def->vars_max[v];\n"
    "  if(max==min)  return 0;\n  return log2(max-min);\n}\n\n"
    "static int %1$s__chromosome_varget(const %1$s__t* def,const %1$s__chromosome_t* c,uint32_t* p,size_t v,intmax_t* arg)\n"
    "{\n  intmax_t n=getnext(c,p,def->wrap);\n  if(n<0) return -1;\n  intmax_t min=def->vars_min[v];\n  intmax_t max=def->vars_max[v];\n"
    "  *arg=(min==max) ? min : (min+n%%(max-min));\n  return 0;\n}\n\n"
    "int %1$s__chromosome_compute(const %1$s__t* def,const %1$s__chromosome_t* c,struct %1$s_ctx_t* ctx,%3$s* ret)\n"
    "{\n  if(!def || !c || %1$s__chromosome_check(def,c))  return -1;\n  uint32_t p=0;\n"
    "  return %1$s__chromosome_val__%2$s(def,c,ctx,&p,ret);\n}\n\n"
    "int %1$s__chromosome_check(const %1$s__t* def,const %1$s__chromosome_t* c)\n{\n  if(!def || !c)  return -1;\n\n"
    "  uint32_t p=0;\n  return %1$s__chromosome_check__%2$s(def,c,&p);\n}\n\n"
    "double %1$s__chromosome_bits(const %1$s__t* def,const %1$s__chromosome_t* c)\n{\n  if(!def || !c)  return -1;\n\n"
    "  uint32_t p=0;\n  return %1$s__chromosome_bits__%2$s(def,c,&p);\n}\n\n"
    "char* %1$s__chromosome_dumps(const %1$s__t* def,const %1$s__chromosome_t* c)\n{\n  if(!def || !c)  return 0;\n"
    "  size_t sz=0;\n  char* rv=0;\n  FILE* f=open_memstream(&rv,&sz);\n  if(%1$s__chromosome_dump(def,c,f))\n"
    "  {\n    fclose(f);\n    free(rv);\n    return 0;\n  }\n  fclose(f);\n  return rv;\n}\n\n"
    "int %1$s__chromosome_dump(const %1$s__t* def,const %1$s__chromosome_t* c,FILE* f)\n{\n"
    "  if(!def || !c || !f || %1$s__chromosome_check(def,c))  return -1;\n  uint32_t p=0;\n  return %1$s__chromosome_dump__%2$s(def,c,&p,f);\n"
    "}\n\nint32_t %1$s__chromosome_save(const %1$s__t* def,const %1$s__chromosome_t* c,FILE* f)\n{\n"
    "  if(!def || !c || !f)  return -1;\n  uint32_t l=c->len;\n  if(fwrite(&l,sizeof(l),1,f)!=1)  return -1;\n"
    "  if(fwrite(c->data,sizeof(c->data[0])*l,1,f)!=1)  return -1;\n  return sizeof(uint32_t)+c->len*sizeof(%1$s__codon_t);\n}\n\n"
    "ssize_t %1$s__chromosome_save_size(const %1$s__t* def,const %1$s__chromosome_t* c)\n{\n  if(!def || !c)  return -1;\n"
    "  return sizeof(uint32_t)+c->len*sizeof(%1$s__codon_t);\n}\n\n"
    "%1$s__chromosome_t* %1$s__chromosome_load(const %1$s__t* def,FILE* f)\n{\n  if(!def || !f)  return 0;\n"
    "  uint32_t l=0;\n  if(fread(&l,sizeof(l),1,f)!=1)  return 0;\n\n  %1$s__chromosome_t* rv=md_calloc(sizeof(*rv)+sizeof(rv->data[0])*l);\n"
    "  rv->len=l;\n  if(fread(rv->data,sizeof(rv->data[0]),l,f)==l)  return rv;\n"
    "  %1$s__chromosome_free(rv);\n  return 0;\n}\n\n"
    "const char* %1$s__sys_uuid(void)\n{\n  return %1$s__uuid;\n}\n\nconst char* %1$s__sys_name(void)\n{\n  return %1$s__name;\n}\n\n",
    g->name,g->init,final);


  fclose(fc);
  md_free(codon);
  md_free(upper);

  while(funcs)
  {
    codegen_func_t* r=funcs;
    HASH_DELETE(hh,funcs,r);
    md_free(r);
  }

  return 0;
}


int grammar_check(const struct grammar_t* g)
{
  if(!g) crash0("grammar is zero pointer\n");
  if(!g->name) crash0("grammar name not defined\n");
  if(!g->uuid) crash0("grammar UUID not defined\n");
  if(!g->init) crash0("initial rule not defined\n");
  if(!g->codon) crash0("codon size not defined\n");
  if(!g->fitness) crash0("fitness type not defined\n");
  if(!g->context) crash0("chromosome context not defined\n");

  {
    grammar_rules_hash_t* r=0;
    HASH_FIND(hh,g->rules,g->init,strlen(g->init),r);
    if(!r)  crash0("initial rule <%s> can not be resolved",g->init);
  }

  for(grammar_rules_hash_t* r=g->rules;r;r=r->hh.next)
  {
    grammar_var_hash_t* v=0;
    HASH_FIND(hh,g->vars,r->name,strlen(r->name),v);
    if(v)  crash0("rule and variable have same name <%s>",r->name);
  }

  for(grammar_var_hash_t* r=g->vars;r;r=r->hh.next)
  {
    grammar_rules_hash_t* v=0;
    HASH_FIND(hh,g->rules,r->name,strlen(r->name),v);
    if(v)  crash0("rule and variable have same name <%s>",r->name);
  }

//  uint8_t* vars=g->vars ? md_calloc(g->vars->order+1) : 0;

  for(grammar_rules_hash_t* r=g->rules;r;r=r->hh.next)
    for(grammar_case_t* c=r->cases;c;c=c->next)
      for(grammar_atom_t *atom=c->atoms;atom;atom=atom->next)
        if(atom->type==grammar_atom_id)
        {
          grammar_var_hash_t* v=0;
          HASH_FIND(hh,g->vars,atom->val,strlen(atom->val),v);
          if(v)
          {
//            vars[v->order]++;
            continue;
          }

          grammar_rules_hash_t* u=0;
          HASH_FIND(hh,g->rules,atom->val,strlen(atom->val),u);
          if(!u)  crash0("orphaned term <%s>",atom->val);
        }

  return 0;
}


// TODO: backtracking and ramp-half init, init with fixed size

