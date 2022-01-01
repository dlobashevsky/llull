

#define CODEGEN_FLAG_PROTOTYPE		1

struct grammar_t;

int grammar_check(const struct grammar_t* g);
int grammar_codegen(struct grammar_t* g,const char* out,uint32_t flags);

