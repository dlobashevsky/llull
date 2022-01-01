#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "macros.h"
#include "data.h"


static const char* names[]=
{
  "Iris-setosa",
  "Iris-versicolor",
  "Iris-virginica"
};


const char* iris_name(iris_class_t class)
{
  switch(class)
  {
    case 0 ... 2 : return names[class];
    default: return 0;
  }
  return 0;
}


iris_class_t iris_class(const char* name)
{
  if(!name || !*name)  return iris_unknown;
  for(size_t i=0;i<sizeof(names)/sizeof(*names);i++)
    if(!strcmp(names[i],name))  return i;
  return iris_unknown;
}


#define GETATTR(l,off)	((l[off]-'0')*10U+(l[off+2]-'0'))

data_t* data_init(const char* fn)
{
  if(!fn || !*fn)  return 0;
  FILE* f=fopen(fn,"r");
  if(!f)  return 0;

  size_t z=0;
  char* line=0;
  size_t lines=0;
  ssize_t s=0;
  while((s=getline(&line,&z,f))>=0)
    if(*line && *line!=' ' && s>18)
      lines++;

  rewind(f);

  data_t* rv=md_new(rv);
  rv->data=md_tmalloc(data_record_t,lines);

  while((s=getline(&line,&z,f))>=0)
    if(*line && *line!=' ' && s>18)
    {
      char* t=strchr(line,'\n'); if(t) *t=0;
      data_record_t* r=rv->data+rv->size++;
      line[3]=line[7]=line[11]=line[15]=0;
      r->class=iris_class(line+16);
      if(r->class==iris_unknown) crash(line);
      r->attrs[0]=GETATTR(line,0);
      r->attrs[1]=GETATTR(line,4);
      r->attrs[2]=GETATTR(line,8);
      r->attrs[3]=GETATTR(line,12);
    }
  fclose(f);
  free(line);
  return rv;
}

void data_free(data_t* d)
{
  if(d)
  {
    md_free(d->data);
    md_free(d);
  }
}


int data_dump(const data_t* d,const char* fn)
{
  if(!d || !fn || !*fn)  return -1;
  FILE *f=fopen(fn,"w");
  if(!f)  return -1;
  for(size_t i=0;i<d->size;i++)
    fprintf(f,"%.1f,%.1f,%.1f,%.1f,%s\n",d->data[i].attrs[0]/10.,d->data[i].attrs[1]/10.,d->data[i].attrs[2]/10.,d->data[i].attrs[3]/10.,
            iris_name(d->data[i].class));

  fclose(f);
  return 0;
}
