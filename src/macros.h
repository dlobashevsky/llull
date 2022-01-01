
__attribute__ ((noreturn))
static inline void error_abort(const char* msg,const char* func,const char* file,int line)
{
  fprintf(stderr,"ERROR %s <%s:%d>\t",func,file,line);
  perror(msg);abort();
}

#define crash(x_)	error_abort(x_,__func__,__FILE__,__LINE__)
#define crash0(s,...)	do { fprintf(stderr,s "\n", ##__VA_ARGS__); exit(1); } while(0);


#ifdef DEBUG
#define logger(s,...)	fprintf(stderr,"# %s <%s:%u>:\t" s "\n", __func__,__FILE__,__LINE__, ##__VA_ARGS__)
#else
#define logger(s,...)	fprintf(stderr,s "\n", ##__VA_ARGS__)
#endif

#define md_malloc(x)	({ void* tmp__=malloc(x); if(!tmp__) crash("memory error"); tmp__; })
#define md_free		free
#define md_calloc(x)	({ void* tmp__=calloc(x,1); if(!tmp__) crash("memory error"); tmp__; })
#define md_realloc(x,y)	({ void* tmp__=realloc(x,y); if(!tmp__) crash("memory error"); tmp__; })
#define md_asprintf(x,...) ({ char *bf__=0; if(asprintf(&bf__, x, ##__VA_ARGS__)<0) crash("memory_error"); bf__; })
static inline char* md_strdup(const char* x)  { if(!x) return 0; void* ret =strdup(x); if(!ret) crash("memory error"); return ret; }


#define md_tmalloc(x,y)		((x*)md_malloc(sizeof(x)*(y)))
#define md_tcalloc(x,y)		((x*)md_calloc(sizeof(x)*(y)))
#define md_pmalloc(x)		((void*)(md_malloc((x)*sizeof(void*))))
#define md_pcalloc(x)		((void*)(md_calloc((x)*sizeof(void*))))
#define md_new(x)		((typeof(x))md_calloc(sizeof(x[0])))
#define md_anew(x,y)		((typeof(x))md_calloc(sizeof(x[0])*(y)))
