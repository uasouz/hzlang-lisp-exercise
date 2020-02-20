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

HzValue* builtin_op(HzValue* value,char* op){
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

HzValue* builtin_head(HzValue* value){
    LASSERT(value,value->count == 1,"Function 'head' passed too many arguments.");
    LASSERT(value,value->cell[0]->type == HZVAL_QEXPR,"Function 'head' passed incorrect type.");
    LASSERT(value,value->cell[0]->count != 0,"Function 'head' passed '{}'.");

    HzValue* head = hzval_take(value,0);

    while(head->count>1) {hzval_del(hzval_pop(head,1));}
    hzval_details_println(head);
    return head;
}

HzValue* builtin_tail(HzValue* value){
    LASSERT(value,value->count == 1,"Function 'tail' passed too many arguments.");
    LASSERT(value,value->cell[0]->type == HZVAL_QEXPR,"Function 'tail' passed incorrect type.");
    LASSERT(value,value->cell[0]->count != 0,"Function 'tail' passed '{}'.");

    HzValue* tail = hzval_take(value,0);

    hzval_del(hzval_pop(tail,0));
    
    return tail;
}

HzValue* builtin_list(HzValue* value){
    value->type = HZVAL_QEXPR;
    return value;
}

HzValue* builtin_eval(HzValue* value){
    LASSERT(value,value->count == 1,"Function 'eval' passed too many arguments.");
    LASSERT(value,value->cell[0]->type == HZVAL_QEXPR,"Function 'eval' passed incorrect type.");

    HzValue* evaluable = hzval_take(value,0);
    evaluable->type = HZVAL_SEXPR;
    return hzval_eval(evaluable);
}

HzValue* builtin_join(HzValue* value){

    for(int i = 0;i < value->count;i++){
        LASSERT(value,value->cell[i]->type == HZVAL_QEXPR,"Function 'join' passed incorrect type.");
    }
    hzval_details_println(value);
    HzValue* accumulator = hzval_pop(value,0);

    while (value->count)
    {
        accumulator  = hzval_join(accumulator,hzval_pop(value,0));
    }
    
    hzval_del(value);
    return accumulator;
}

HzValue* builtin_cons(HzValue* value){
    LASSERT(value,value->count == 2,"Function 'cons' passed too many arguments.");
    LASSERT(value,value->cell[1]->type == HZVAL_QEXPR,"Function 'cons' passed incorrect type for second value,must be a Q-Expression.");
    
    if(!(value->cell[0]->type==HZVAL_QEXPR || value->cell[0]->type==HZVAL_SEXPR)){
        HzValue* parent = hzval_qexpression();
        value->cell[0] = hzval_add(parent,value->cell[0]);
    }

    return builtin_join(value);
}

HzValue* builtin_len(HzValue* value){
    LASSERT(value,value->count == 1,"Function 'eval' passed too many arguments.");
    LASSERT(value,value->cell[0]->type == HZVAL_QEXPR,"Function 'eval' passed incorrect type.");

    return hzval_num(value->cell[0]->count);
}

HzValue* builtin(HzValue* value,char* func){
  if (strcmp("list", func) == 0) { return builtin_list(value); }
  if (strcmp("head", func) == 0) { return builtin_head(value); }
  if (strcmp("tail", func) == 0) { return builtin_tail(value); }
  if (strcmp("join", func) == 0) { return builtin_join(value); }
  if (strcmp("eval", func) == 0) { return builtin_eval(value); }
  if (strcmp("cons", func) == 0) { return builtin_cons(value); }
  if (strcmp("len", func) == 0)  { return builtin_len(value);  }
  if (strstr("+-/*^", func)) { return builtin_op(value, func); }
  hzval_del(value);
  return hzval_err("Unknown Function!");
}