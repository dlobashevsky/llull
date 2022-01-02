#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "macros.h"
#include "process.h"
#include "aimgp.h"
#include "aimgp__proto.h"


#define RUNS	1000
#define POPULATION	1000
#define LEN	512
#define REGISTERS	4

static inline uint32_t target(uint32_t x)
{
  return ((x<<4)^(x>>5))|(x+0xac);
}

static inline uint32_t bits(uint32_t s)
{
  uint32_t ret=0;
  while(s)
  {
    ret+=s&1;
    s>>=1;
  }
  return ret;
}

aimgp_ctx_t* aimgp_ctx_init(size_t regs,size_t runs)
{
  if(!regs)  return 0;
  aimgp_ctx_t* rv=md_new(rv);
  rv->regs_count=regs;
  rv->runs=runs;
  rv->regs=md_tcalloc(uint32_t,regs);
  return rv;
}

void aimgp_ctx_free(aimgp_ctx_t* r)
{
  if(!r)  return;
  md_free(r->regs);
  md_free(r);
}

int aimgp_ctx_reset(aimgp_ctx_t* r)
{
  if(!r)  return -1;
  memset(r->regs,0,sizeof(r->regs[0])*r->regs_count);
  return 0;
}

aimgp__fitness_t fitness(aimgp__t* def,aimgp__chromosome_t* c,aimgp_ctx_t* r)
{
  if(!c)  return -1;

  size_t success=0;
  for(size_t i=0;i<r->runs;i++)
  {
    aimgp_ctx_reset(r);
    uint32_t src=rand()%0xffffffffU;
    uint32_t dst=target(src);
    r->regs[0]=src;
    uint32_t res=0;

    if(aimgp__chromosome_compute(def,c,r,&res))  continue;
//logger("src %x want %x got %x",src,dst,res);
    success+=bits(~(dst^res));
  }

  return success/(32.*r->runs);
}



uint32_t aimgp__runner(aimgp_ctx_t* ctx, int dummy,int* err)
{
  return ctx->regs[0];
}


int aimgp__sequence(aimgp_ctx_t* ctx, const aimgp__codon_t count, int* dummy,int* err)
{
  return 0;
}

int aimgp__bop(aimgp_ctx_t* ctx, op_t op, aimgp__codon_t r1, aimgp__codon_t r2,int* err)
{
  *err=0;
  switch(op)
  {
    case OP_ADD:
      ctx->regs[r1]+= ctx->regs[r2];
      break;
    case OP_SUB:
      ctx->regs[r1]-= ctx->regs[r2];
      break;
    case OP_AND:
      ctx->regs[r1]&= ctx->regs[r2];
      break;
    case OP_OR:
      ctx->regs[r1]|= ctx->regs[r2];
      break;
    case OP_XOR:
      ctx->regs[r1]^= ctx->regs[r2];
      break;
    case OP_LSHIFT:
      ctx->regs[r1]<<= ctx->regs[r2];
      break;
    case OP_RSHIFT:
      ctx->regs[r1]>>= ctx->regs[r2];
      break;

    default: crash0("invalid double-operand instruction %u",op);
  }
  return 0;
}

int aimgp__sop(aimgp_ctx_t* ctx, op_t op, aimgp__codon_t reg,int* err)
{
  *err=0;
  switch(op)
  {
    case OP_NEG:
      ctx->regs[reg]= ~ctx->regs[reg];
      break;
    case OP_INV:
      ctx->regs[reg]= !ctx->regs[reg];
      break;
    default: crash0("invalid single-operand instruction %u",op);
  }
  return 0;
}

int aimgp__const(aimgp_ctx_t* ctx, aimgp__codon_t reg, aimgp__codon_t val,int* err)
{
  *err=0;
  ctx->regs[reg]=val;
  return 0;
}


int main(void)
{
  srand(time(0));

  aimgp_ctx_t* ctx=aimgp_ctx_init(REGISTERS,RUNS);
  aimgp__t* aimgp=aimgp__init(2);

  aimgp__setvar(aimgp,aimgp__vars__REGNO,0,REGISTERS);
  aimgp__setvar(aimgp,aimgp__vars__EPHEMERAL,0,0xffffffffU);
  size_t success=0;

  aimgp__fitness_t best=-1.;
  double mdl=0;
  aimgp__chromosome_t* result=0;

  for(size_t i=0;i<POPULATION;i++)
  {
    aimgp__chromosome_t* c=aimgp__chromosome_init(aimgp,LEN,0,0);
    if(!c)  continue;
    aimgp__fitness_t fit=fitness(aimgp,c,ctx);
    if(fit>=0)
    {
      success++;
      if(fit>best)
      {
        best=fit;
        mdl=aimgp__chromosome_bits(aimgp,c);
        if(result) aimgp__chromosome_free(result);
        result=c;
      }
      else
          aimgp__chromosome_free(c);
    }
    else
      aimgp__chromosome_free(c);
  }

  logger("non-lethal %.2f%% best fit %g with %g bits",success*100./POPULATION,best,mdl);

  if(result)
  {
    aimgp__fitness_t fit=fitness(aimgp,result,ctx);
    char* dump=aimgp__chromosome_dumps(aimgp,result);
    logger("result: %g\n%s",fit,dump);
    md_free(dump);

    aimgp_ctx_reset(ctx);
    uint32_t src=rand()%0xffffffffU;
    uint32_t dst=target(src);
    ctx->regs[0]=src;
    uint32_t res=0;

    if(aimgp__chromosome_compute(aimgp,result,ctx,&res))  crash("something wrong here");
    logger("pass %x want %x got %x common %x bits %d",src,dst,res,~(dst^res),bits(~(dst^res)));

    aimgp__chromosome_free(result);
  }

  aimgp_ctx_free(ctx);
  aimgp__free(aimgp);
  return 0;
}





