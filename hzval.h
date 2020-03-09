#ifndef hzval_h
#define hzval_h

#include "mpc.h"

//
struct HzValue;
struct HzEnv;
typedef struct HzValue HzValue;
typedef struct HzEnv HzEnv;

//Evaluation Types Enum
enum {HZVAL_NUM,HZVAL_DECIMAL,HZVAL_SYM,HZVAL_FUN,HZVAL_SEXPR,HZVAL_QEXPR,HZVAL_ERR};

typedef HzValue*(*HzFunction)(HzEnv*,HzValue*);

typedef struct HzValue {
  int type;
  long num;
  double dec;
  /*Function Type*/
  HzFunction function;
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
HzValue* hzval_qexpression(void);

//HzValue Methods
HzValue* hzval_copy(HzValue* value);
HzValue* hzval_add(HzValue* parent,HzValue* child);
HzValue* hzval_pop(HzValue* value,int position);
HzValue* hzval_take(HzValue* value,int position);
HzValue* hzval_join(HzValue* first,HzValue* second);
void hzval_del(HzValue* hzValue);

void hzval_print(HzValue* value);
void hzval_println(HzValue* value);
void hzval_details_println(HzValue* value);
void hzval_expr_print(HzValue* hzvalue,char open,char close);

HzValue* hzval_read(mpc_ast_t* tree);
HzValue* hzval_read_num(mpc_ast_t* tree);
HzValue* hzval_read_decimal(mpc_ast_t* tree);

HzValue* hzval_eval_sexpr(HzEnv* env,HzValue* value);
HzValue* hzval_eval(HzEnv*env,HzValue* value);


/*Environment */
HzEnv* hzenv_new(void);
void hzenv_del(HzEnv* env);
void hzenv_add_builtins(HzEnv* env);
#endif