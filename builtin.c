#include "builtin.h"

#define LASSERT(args, cond, err) \
  if (!(cond)) { hzval_del(args); return hzval_err(err); }

HzValue* eval_op(HzValue* x, char* op, HzValue* y) {
    if(x->type==HZVAL_ERR){ return hzval_err(x->err);}
    if(y->type==HZVAL_ERR) { return hzval_err(y->err);}

    if(x->type==HZVAL_DECIMAL || y->type==HZVAL_DECIMAL){
        if(x->type==HZVAL_NUM){
            x->dec = x->num;
        }
        if(y->type==HZVAL_NUM){
            y->dec = y->num;
        }
        
        if (strcmp(op, "+") == 0) { return hzval_decimal(x->dec + y->dec); }
        if (strcmp(op, "-") == 0) { return hzval_decimal(x->dec - y->dec); }
        if (strcmp(op, "*") == 0) { return hzval_decimal(x->dec * y->dec); }
        if (strcmp(op, "/") == 0) {
            if(y->dec==0){
                return hzval_err("Division by zero");
            } else {
                return hzval_decimal(x->dec / y->dec);
            }
            }
        if (strcmp(op, "%") == 0) { return hzval_decimal(fmod(x->dec,y->dec)); }
        if (strcmp(op, "^") == 0) { return hzval_decimal(pow(x->dec,y->dec)); }
    }
  if (strcmp(op, "+") == 0) { return hzval_num(x->num + y->num); }
  if (strcmp(op, "-") == 0) { return hzval_num(x->num - y->num); }
  if (strcmp(op, "*") == 0) { return hzval_num(x->num * y->num); }
  if (strcmp(op, "/") == 0) {
      if(y->num==0){
          return hzval_err("Division by zero");
      } else {
        return hzval_num(x->num / y->num);
      }
    }
  if (strcmp(op, "%") == 0) { return hzval_num(x->num % y->num); }
  if (strcmp(op, "^") == 0) { return hzval_num(pow(x->num,y->num)); }
  return hzval_err("Bad Operation");
}

HzValue* builtin_op(HzEnv* env,HzValue* value,char* op){
    for(int i=0; i< value->count; i++){
        if(!(value->cell[i]->type == HZVAL_NUM || value->cell[i]->type == HZVAL_DECIMAL)){
            hzval_del(value);
            return hzval_err("Cannot operate on non-number");
        }
    }
    HzValue* start = hzval_pop(value,0);

    if ((strcmp(op,"-")==0)&& value->count==0){
        start->num = -start->num;
    }

    while(value->count >0){
        HzValue* next = hzval_pop(value,0);

        start = eval_op(start,op,next);

        hzval_del(next);
    }    
    hzval_del(value);
     return start;
}

HzValue* builtin_head(HzEnv* env,HzValue* value){
    LASSERT(value,value->count == 1,"Function 'head' passed too many arguments.");
    LASSERT(value,value->cell[0]->type == HZVAL_QEXPR,"Function 'head' passed incorrect type.");
    LASSERT(value,value->cell[0]->count != 0,"Function 'head' passed '{}'.");

    HzValue* head = hzval_take(value,0);

    while(head->count>1) {hzval_del(hzval_pop(head,1));}
    // hzval_details_println(head);
    return head;
}

HzValue* builtin_tail(HzEnv* env,HzValue* value){
    LASSERT(value,value->count == 1,"Function 'tail' passed too many arguments.");
    LASSERT(value,value->cell[0]->type == HZVAL_QEXPR,"Function 'tail' passed incorrect type.");
    LASSERT(value,value->cell[0]->count != 0,"Function 'tail' passed '{}'.");

    HzValue* tail = hzval_take(value,0);

    hzval_del(hzval_pop(tail,0));
    
    return tail;
}

HzValue* builtin_list(HzEnv* env,HzValue* value){
    value->type = HZVAL_QEXPR;
    return value;
}

HzValue* builtin_eval(HzEnv* env,HzValue* value){
    LASSERT(value,value->count == 1,"Function 'eval' passed too many arguments.");
    LASSERT(value,value->cell[0]->type == HZVAL_QEXPR,"Function 'eval' passed incorrect type.");

    HzValue* evaluable = hzval_take(value,0);
    evaluable->type = HZVAL_SEXPR;
    return hzval_eval(env,evaluable);
}

HzValue* builtin_join(HzEnv* env,HzValue* value){

    for(int i = 0;i < value->count;i++){
        LASSERT(value,value->cell[i]->type == HZVAL_QEXPR,"Function 'join' passed incorrect type.");
    }
    // hzval_details_println(value);
    HzValue* accumulator = hzval_pop(value,0);

    while (value->count)
    {
        accumulator  = hzval_join(accumulator,hzval_pop(value,0));
    }
    
    hzval_del(value);
    return accumulator;
}

HzValue* builtin_cons(HzEnv* env,HzValue* value){
    LASSERT(value,value->count == 2,"Function 'cons' passed too many arguments.");
    LASSERT(value,value->cell[1]->type == HZVAL_QEXPR,"Function 'cons' passed incorrect type for second value,must be a Q-Expression.");
    
    if(!(value->cell[0]->type==HZVAL_QEXPR || value->cell[0]->type==HZVAL_SEXPR)){
        HzValue* parent = hzval_qexpression();
        value->cell[0] = hzval_add(parent,value->cell[0]);
    }

    return builtin_join(env,value);
}

HzValue* builtin_len(HzEnv* env,HzValue* value){
    LASSERT(value,value->count == 1,"Function 'eval' passed too many arguments.");
    LASSERT(value,value->cell[0]->type == HZVAL_QEXPR,"Function 'eval' passed incorrect type.");

    return hzval_num(value->cell[0]->count);
}


HzValue* builtin_add(HzEnv* env,HzValue* value){
    return builtin_op(env,value,"+");
}

HzValue* builtin_sub(HzEnv* env,HzValue* value){
    return builtin_op(env,value,"-");
}

HzValue* builtin_mul(HzEnv* env,HzValue* value){
    return builtin_op(env,value,"*");
}

HzValue* builtin_div(HzEnv* env,HzValue* value){
    return builtin_op(env,value,"/");
}

HzValue* builtin_pow(HzEnv* env,HzValue* value){
    return builtin_op(env,value,"^");
}

HzValue* builtin_def(HzEnv* env,HzValue* value){
    LASSERT(value,value->cell[0]->type == HZVAL_QEXPR,"Function 'def' passed incorrect type.");

    HzValue* symbols = value->cell[0];

    for(int i =0;i< symbols->count;i++){
        LASSERT(value,symbols->cell[i]->type == HZVAL_SYM,"Function 'def' cannot define non-symbol");
    }

    LASSERT(value,symbols->count == value->count-1,"Function 'def cannot define incorrect "
    "number of values to symbols");

    for(int i=0;i<symbols->count;i++){
        hzenv_put(env,symbols->cell[i],value->cell[i+1]);
    }

    hzval_del(value);
    return hzval_sexpression();
}

/*deprecated(?)*/
HzValue* builtin(HzEnv* env,HzValue* value,char* func){
  if (strcmp("list", func) == 0) { return builtin_list(env,value); }
  if (strcmp("head", func) == 0) { return builtin_head(env,value); }
  if (strcmp("tail", func) == 0) { return builtin_tail(env,value); }
  if (strcmp("join", func) == 0) { return builtin_join(env,value); }
  if (strcmp("eval", func) == 0) { return builtin_eval(env,value); }
  if (strcmp("cons", func) == 0) { return builtin_cons(env,value); }
  if (strcmp("len", func) == 0)  { return builtin_len(env,value);  }
  if (strstr("+-/*^", func)) { return builtin_op(env,value, func); }
  hzval_del(value);
  return hzval_err("Unknown Function!");
}