#ifndef builtin_h
#define builtin_h

#include "hzval.h"

HzValue* builtin(HzValue* value,char* func);
HzValue* builtin_op(HzValue* value,char* op);
HzValue* builtin_list(HzValue* value);
HzValue* builtin_head(HzValue* value);
HzValue* builtin_tail(HzValue* value);
HzValue* builtin_eval(HzValue* value);
HzValue* builtin_join(HzValue* value);


#endif