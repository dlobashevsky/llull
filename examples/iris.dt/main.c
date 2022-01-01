#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "macros.h"
#include "data.h"
#include "context.h"
#include "irisdt.h"

#define TEST_OUT	"test.out/"

#define MINLEN		10
#define MAXLEN		400
#define LOOPS		5000
#define CROSSES		5000


irisdt__fitness_t fitness(irisdt__t* def,irisdt__chromosome_t* c,data_t* d)
{
  if(!c)  return -1;

  static const char* names[]=
  {
    "Iris Setosa",
    "Iris Versicolour",
    "Iris Virginica"
  };

  irisdt_ctx_t ctx={d,0};

  size_t success=0;

  for(;ctx.line<d->size;ctx.line++)
  {
    const char* res=0;
    if(irisdt__chromosome_compute(def,c,&ctx,&res))  return -1.;
    for(size_t i=0;i<3;i++)
    {
      if(!res || strcmp(res,names[i]))  continue;
      if(d->data[ctx.line].class==i)  success++;
      break;
    }
  }
  return success/(double)d->size;
}


int main(void)
{
  srand(time(0));
  data_t* d=data_init("data/iris.data");
  if(!d)  crash("no data)");

  irisdt__t* iris=irisdt__init(2);
  if(!iris)  crash("init");

  mkdir(TEST_OUT,0777);

  FILE *f=fopen(TEST_OUT "save0","w");
  if(!f)  crash("save");
  logger("saved size %d",irisdt__save(iris,f));
  fclose(f);
  irisdt__free(iris);

  f=fopen(TEST_OUT "save0","r");
  if(!f)  crash("no file");
  iris=irisdt__load(f);
  fclose(f);
  if(!iris) crash("crashed in load");

  size_t max=1+(MAXLEN-MINLEN)/10;
  irisdt__chromosome_t* pool[1+(MAXLEN-MINLEN)/10]={0};
  size_t found=0;

  for(size_t len=MINLEN;len<MAXLEN;len+=10)
  {
    size_t success=0;
    irisdt__fitness_t best=-1.;
    double mdl=0;
    irisdt__chromosome_t* result=0;

    for(size_t i=0;i<LOOPS;i++)
    {
      irisdt__chromosome_t* c=irisdt__chromosome_init(iris,len,0,0);
      if(!c)  continue;
      irisdt__fitness_t fit=fitness(iris,c,d);
      if(fit>=0)
      {
        success++;
        if(fit>best)
        {
          best=fit;
          mdl=irisdt__chromosome_bits(iris,c);
          if(result) irisdt__chromosome_free(result);
          result=c;
        }
        else
          irisdt__chromosome_free(c);
      }
      else
        irisdt__chromosome_free(c);
    }
    logger("length %zd:\tnon-lethal %.2f%% best fit %g with %g bits",len,success*100./LOOPS,best,mdl);
    if(result)
    {
      char* dump=irisdt__chromosome_dumps(iris,result);
      logger("result: <%s>",dump);
      md_free(dump);
      char* name=md_asprintf("%ssaved_%zu.chr",TEST_OUT,len);
      FILE *f=fopen(name,"w");
      irisdt__chromosome_save(iris,result,f);
      fclose(f);
      md_free(name);
      if(mdl>2.)  pool[found++]=result;
      else irisdt__chromosome_free(result);
    }
  }
  logger("found %zu non-trivial chromosomes",found);
  for(size_t i=0;i<found;i++)
  {
    char* dump=irisdt__chromosome_dumps(iris,pool[i]);
    logger("\t%.2f%%\t%g\t%s",100.*fitness(iris,pool[i],d),irisdt__chromosome_bits(iris,pool[i]),dump);
    md_free(dump);
  }

  if(found>2)
    for(size_t i=0;i<CROSSES;i++)
    {
      if(found>=max)  break;
      size_t u1=rand()%found;
      size_t u2=rand()%found;
      if(u1==u2 || pool[u1]->len<4 || pool[u2]->len<4)  continue;
      size_t z1=1+(rand()%(pool[u1]->len-1));
      size_t z2=1+(rand()%(pool[u2]->len-1));
      irisdt__chromosome_t* c1=irisdt__chromosome_init0(iris,z1+pool[u2]->len-z2);
      irisdt__chromosome_t* c2=irisdt__chromosome_init0(iris,z2+pool[u1]->len-z1);

      memcpy(c1->data,pool[u1]->data,z1*sizeof(irisdt__codon_t));
      memcpy(c2->data,pool[u2]->data,z2*sizeof(irisdt__codon_t));
      memcpy(c1->data+z1,pool[u2]->data+z2,(pool[u2]->len-z2)*sizeof(irisdt__codon_t));
      memcpy(c2->data+z2,pool[u1]->data+z1,(pool[u1]->len-z1)*sizeof(irisdt__codon_t));
//      int irisdt__chromosome_uniform(const irisdt__chromosome_t*,const irisdt__chromosome_t*,size_t offset,irisdt__chromosome_t**,irisdt__chromosome_t**);
      if(irisdt__chromosome_check(iris,c1) || found>=max || irisdt__chromosome_bits(iris,c1)<=2.5)
        irisdt__chromosome_free(c1);
      else
        pool[found++]=c1;

      if(irisdt__chromosome_check(iris,c2) || found>=max || irisdt__chromosome_bits(iris,c2)<=2.5)
        irisdt__chromosome_free(c2);
      else
        pool[found++]=c2;
    }

  logger("after crosover found %zu non-trivial chromosomes",found);
  for(size_t i=0;i<found;i++)
  {
    char* dump=irisdt__chromosome_dumps(iris,pool[i]);
    logger("\t%.2f%%\t%g\t%s",100.*fitness(iris,pool[i],d),irisdt__chromosome_bits(iris,pool[i]),dump);
    md_free(dump);
  }

  while(found--) irisdt__chromosome_free(pool[found]);

  irisdt__free(iris);
  data_free(d);

  logger("done");
  return 0;
}

