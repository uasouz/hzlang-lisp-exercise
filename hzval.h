#ifndef hzval_h
#define hzval_h

#include "mpc.h"

//Evaluation Types Enum
enum {HZVAL_NUM,HZVAL_DECIMAL,HZVAL_SYM,HZVAL_SEXPR,HZVAL_ERR};


typedef struct HzValue {
  int type;
  long num;
  double dec;
  /* Error and Symbol types have some string data */
  char* err;
  char* sym;
  /* Count and Pointer to a list of "HzValue*" */
  int count;
  struct HzValue** cell;
} HzValue;

//HzValue types constructors
HzValue* hzval_num(long value);
HzValue* hzval_decimal(double value);
HzValue* hzval_err(char* err);
HzValue* hzval_sym(char* sym);
HzValue* hzval_sexpression(void);

//HzValue Methods
HzValue* hzval_add(HzValue* parent,HzValue* child);
HzValue* hzval_pop(HzValue* value,int position);
HzValue* hzval_take(HzValue* value,int position);
void hzval_del(HzValue* hzValue);

void hzval_print(HzValue* value);
void hzval_println(HzValue* value);
void hzval_details_println(HzValue* value);
void hzval_expr_print(HzValue* hzvalue,char open,char close);

HzValue* hzval_read(mpc_ast_t* tree);
HzValue* hzval_read_num(mpc_ast_t* tree);
HzValue* hzval_read_decimal(mpc_ast_t* tree);

HzValue* builtin_op(HzValue* value,char* op);
HzValue* hzval_eval_sexpr(HzValue* value);
HzValue* hzval_eval(HzValue* value);

#endif