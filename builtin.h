#ifndef builtin_h
#define builtin_h

#include "hzval.h"

HzValue* builtin(HzEnv* env,HzValue* value,char* func);
HzValue *builtin_lambda(HzEnv *env, HzValue *value);

HzValue* builtin_op(HzEnv* env,HzValue* value,char* op);
HzValue* builtin_list(HzEnv* env,HzValue* value);
HzValue* builtin_head(HzEnv* env,HzValue* value);
HzValue* builtin_tail(HzEnv* env,HzValue* value);
HzValue* builtin_eval(HzEnv* env,HzValue* value);
HzValue* builtin_join(HzEnv* env,HzValue* value);
HzValue* builtin_cons(HzEnv* env,HzValue* value);
HzValue *builtin_len(HzEnv *env, HzValue *value);

HzValue* builtin_def(HzEnv* env,HzValue* value);
HzValue *builtin_put(HzEnv *env, HzValue *value);
HzValue *builtin_fun(HzEnv *env, HzValue *value);

//Math Function wrappers
HzValue* builtin_add(HzEnv* env,HzValue* value);
HzValue* builtin_sub(HzEnv* env,HzValue* value);
HzValue* builtin_mul(HzEnv* env,HzValue* value);
HzValue* builtin_div(HzEnv* env,HzValue* value);
HzValue* builtin_pow(HzEnv* env,HzValue* value);

#endif