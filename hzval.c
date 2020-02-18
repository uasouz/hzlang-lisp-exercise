#include "hzval.h"

//Utils
char* join_string(char* first,char* second){
    char* buf = malloc(sizeof(char)* 2048);
    snprintf(buf,2047,"%s%s",first,second);
    return buf;
}

char* create_error_message(char* message){
    return join_string("Error: ", message);
}
//End Utils

HzValue* hzval_num(long value){
    HzValue* hzValue = malloc(sizeof(HzValue));
    hzValue->type = HZVAL_NUM;
    hzValue->num = value;
    return hzValue;
}

HzValue* hzval_decimal(double value){
    HzValue* hzValue = malloc(sizeof(HzValue));
    hzValue->type = HZVAL_DECIMAL;
    hzValue->dec = value;
    return hzValue;
}

HzValue* hzval_err(char* err){
    HzValue* hzValue = malloc(sizeof(HzValue));
    hzValue->type = HZVAL_ERR;
    hzValue->err = malloc(strlen(err)+1);
    strcpy(hzValue->err,err);
    return hzValue;
}

HzValue* hzval_sym(char* sym){
    HzValue* hzValue = malloc(sizeof(HzValue));
    hzValue->type = HZVAL_SYM;
    hzValue->sym = malloc(strlen(sym)+1);
    strcpy(hzValue->sym,sym);
    return hzValue;
}

HzValue* hzval_sexpression(void){
    HzValue* hzValue = malloc(sizeof(HzValue));
    hzValue->type = HZVAL_SEXPR;
    hzValue->count= 0;
    hzValue->cell=NULL;
    return hzValue;
}

void hzval_del(HzValue* hzValue){
    switch(hzValue->type){
        case HZVAL_NUM || HZVAL_DECIMAL: break;

        case HZVAL_ERR: free(hzValue->err); break;
        case HZVAL_SYM: free(hzValue->sym); break;

        case HZVAL_SEXPR:
            for(int i =0;i<hzValue->count;i++){
                hzval_del(hzValue->cell[i]);
            }
            free(hzValue->cell);
        break;
    }
    free(hzValue);
}


void hzval_print_type(HzValue* value){
    switch (value->type)
    {
    case HZVAL_NUM:
        printf("HZVAL_NUM");
        break;
    case HZVAL_DECIMAL:
        printf("HZVAL_DECIMAL");
        break;
    case HZVAL_ERR: {
        printf("HZVAL_ERR");
        break;
    }
    case HZVAL_SYM:
        printf("HZVAL_SYM"); break;
    case HZVAL_SEXPR:
        printf("HZVAL_SEXPR"); break;
    }
}

void hzval_print(HzValue* value){
    switch (value->type)
    {
    case HZVAL_NUM:
        printf("%li",value->num);
        break;
    case HZVAL_DECIMAL:
        printf("%f",value->dec);
        break;
    case HZVAL_ERR: {
        char* error = create_error_message(value->err);
        printf("%s",error);
        free(error);
        break;
    }
    case HZVAL_SYM:
        printf("%s",value->sym); break;
    case HZVAL_SEXPR:
        hzval_expr_print(value,'(',')'); break;
    }
}

void hzval_println(HzValue* value) { hzval_print(value); putchar('\n');}
void hzval_details_println(HzValue* value) {
    hzval_print_type(value);
    printf("::");
    hzval_print(value);
    printf("::");
    printf("%d",value->count);
    putchar('\n');
    }


HzValue* hzval_read_num(mpc_ast_t* tree){
        errno = 0;
        long number = strtol(tree->contents,NULL,10);
        return errno != ERANGE ? hzval_num(number) : hzval_err("Invalid Number");
}

HzValue* hzval_read_decimal(mpc_ast_t* tree){
        errno = 0;
        double number = strtod(tree->contents,NULL);
        return errno != ERANGE ? hzval_decimal(number) : hzval_err("Invalid Number");
}

HzValue* hzval_add(HzValue* parent,HzValue* child){
    parent->count++;
    parent->cell = realloc(parent->cell,sizeof(HzValue*) * parent->count);
    parent->cell[parent->count-1] = child;
    return parent;
}

HzValue* hzval_read(mpc_ast_t* tree){
    if(strstr(tree->tag,"number")){
        return hzval_read_num(tree);
    }
    if(strstr(tree->tag,"decimal")){
        return hzval_read_decimal(tree);
    }
    if(strstr(tree->tag,"symbol")){
        return hzval_sym(tree->contents);
    }

    HzValue* read = NULL;
    if(strcmp(tree->tag,">")==0){
        read = hzval_sexpression();
    }
    if(strstr(tree->tag,"sexpression")){
        read = hzval_sexpression();
    }

    for(int i=0;i <tree->children_num; i++){
        if (strcmp(tree->children[i]->contents, "(")==0){ continue;}
        if (strcmp(tree->children[i]->contents, ")")==0){ continue;}
        if (strcmp(tree->children[i]->tag, "regex")==0){ continue;}
        read = hzval_add(read,hzval_read(tree->children[i]));
    }

    return read;
}

void hzval_expr_print(HzValue* hzvalue,char open,char close){
    putchar(open);
    for(int i=0;i<hzvalue->count;i++){
        hzval_print(hzvalue->cell[i]);

        if(i != (hzvalue->count-1)){
            putchar(' ');
        }
    }
    putchar(close);
}

HzValue* hzval_pop(HzValue* value,int position){
    HzValue* item = value->cell[position];

    memmove(&value->cell[position],&value->cell[position+1],
        sizeof(HzValue*) * (value->count-position-1));

    value->count--;

    value->cell = realloc(value->cell,sizeof(HzValue*)*value->count);

    return item;
}

HzValue* hzval_take(HzValue* value,int position){
    HzValue* item = hzval_pop(value,position);
    hzval_del(value);
    return item;
}

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
        if(value->cell[i]->type != HZVAL_NUM){
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

HzValue* hzval_eval_sexpr(HzValue* value){
    for(int i=0;i < value->count;i++){
        value->cell[i] = hzval_eval(value->cell[i]);
    }

    for(int i=0;i < value->count;i++){
        if(value->cell[i]->type == HZVAL_ERR) {return hzval_take(value,i);}
    }

    if(value->count==0){return value;}

    if(value->count==1){return hzval_take(value,0);}

    HzValue* first = hzval_pop(value,0);

    if(first->type != HZVAL_SYM){
        hzval_del(first);hzval_del(value);
        return hzval_err("S-expression Does not start with symbol!");
    }

    HzValue* result = builtin_op(value,first->sym);
    hzval_del(first);
    return result;
}

HzValue* hzval_eval(HzValue* value){
    if(value->type == HZVAL_SEXPR) { return hzval_eval_sexpr(value);}

    return value;
}