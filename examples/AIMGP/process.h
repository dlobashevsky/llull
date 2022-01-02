

typedef struct aimgp_ctx_t
{
  size_t regs_count;
  uint32_t* regs;
  size_t runs;
} aimgp_ctx_t;

aimgp_ctx_t* aimgp_ctx_init(size_t regs,size_t runs);
void aimgp_ctx_free(aimgp_ctx_t*);

int aimgp_ctx_reset(aimgp_ctx_t*);

typedef enum op_t
{
  OP_NEG,
  OP_INV,
  OP_ADD,
  OP_SUB,
//  OP_MUL,
//  OP_DIV,
  OP_AND,
  OP_OR,
  OP_XOR,
  OP_RSHIFT,
  OP_LSHIFT,
} op_t;
