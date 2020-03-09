#include "hzval.h"
#include "builtin.h"
#include "builtin/hashmap.h"
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

//Types Constructors
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

HzValue* hzval_function(HzFunction function){
    HzValue* hzValue = malloc(sizeof(HzValue));
    hzValue->type = HZVAL_FUN;
    hzValue->function = function;
    return hzValue;
}

HzValue* hzval_sexpression(void){
    HzValue* hzValue = malloc(sizeof(HzValue));
    hzValue->type = HZVAL_SEXPR;
    hzValue->count= 0;
    hzValue->cell=NULL;
    return hzValue;
}

HzValue* hzval_qexpression(void){
    HzValue* hzValue = malloc(sizeof(HzValue));
    hzValue->type = HZVAL_QEXPR;
    hzValue->count= 0;
    hzValue->cell=NULL;
    return hzValue;
}
//End Types Constructors

//Environment
struct HzEnv  {
    struct hashmap store;
};

typedef struct HzEnvMapEntry{
    struct hashmap_entry ent;
    HzValue* value;
} HzEnvMapEntry;

HzEnvMapEntry* hzenv_map_entry_new(HzValue* key,HzValue* value){
    HzEnvMapEntry *envEntry = malloc(sizeof(HzEnvMapEntry));
    hashmap_entry_init(envEntry,hash(key->sym,sizeof(key->sym)));
    envEntry->value = hzval_copy(value);
    return envEntry;
}

HzValue* hzenv_get(HzEnv* env,HzValue* key){
    struct HzEnvMapEntry keyEntry;
    hashmap_entry_init(&keyEntry,hash(key->sym,sizeof(key->sym)));
    struct HzEnvMapEntry* entry = hashmap_get(&env->store,&keyEntry,NULL);
    if(entry != NULL){
        return hzval_copy(entry->value);
    }
    return hzval_err("unbound symbol!");
}

void hzenv_put(HzEnv* env,HzValue* key,HzValue* value){
    HzEnvMapEntry *entry = hzenv_map_entry_new(key,value);
    hashmap_put(&env->store,entry);
};

HzEnv* hzenv_new(void){
    HzEnv* env = malloc(sizeof(HzEnv));
    hashmap_init(&env->store,NULL,NULL,0);
    return env;
}

void hzenv_del(HzEnv* env){
    hashmap_free(&env->store,1);
    free(env);
}

void hzenv_add_function(HzEnv* env,char* name,HzFunction func){
    HzValue* key = hzval_sym(name);
    HzValue* value = hzval_function(func);
    hzenv_put(env,key,value);
    hzval_del(key);hzval_del(value);
}

void hzenv_add_builtins(HzEnv* env){
     /* List Functions */
  hzenv_add_function(env, "list", builtin_list);
  hzenv_add_function(env, "head", builtin_head);
  hzenv_add_function(env, "tail", builtin_tail);
  hzenv_add_function(env, "eval", builtin_eval);
  hzenv_add_function(env, "join", builtin_join);
  hzenv_add_function(env, "cons", builtin_cons);

  /* Mathematical Functions */
  hzenv_add_function(env, "+", builtin_add);
  hzenv_add_function(env, "-", builtin_sub);
  hzenv_add_function(env, "*", builtin_mul);
  hzenv_add_function(env, "/", builtin_div);
  hzenv_add_function(env, "^", builtin_pow);
}
//End Enviroment

void hzval_del(HzValue* hzValue){
    switch(hzValue->type){
        case HZVAL_NUM || HZVAL_DECIMAL: break;
        case HZVAL_FUN: break;
        case HZVAL_ERR: free(hzValue->err); break;
        case HZVAL_SYM: free(hzValue->sym); break;

        case HZVAL_QEXPR:
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
    case HZVAL_QEXPR:
        printf("HZVAL_QEXPR"); break;
    case HZVAL_FUN:
        printf("HZVAL_FUN"); break;
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
    case HZVAL_QEXPR:
        hzval_expr_print(value,'{','}'); break;
    case HZVAL_FUN:
        printf("<function>"); break;
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
    if(strstr(tree->tag,"qexpression")){
        read = hzval_qexpression();
    }

    for(int i=0;i <tree->children_num; i++){
        if (strcmp(tree->children[i]->contents, "(")==0){ continue;}
        if (strcmp(tree->children[i]->contents, ")")==0){ continue;}
        if (strcmp(tree->children[i]->contents, "{")==0){ continue;}
        if (strcmp(tree->children[i]->contents, "}")==0){ continue;}
        if (strcmp(tree->children[i]->tag, "regex")==0){ continue;}
        read = hzval_add(read,hzval_read(tree->children[i]));
    }

    return read;
}

HzValue* hzval_copy(HzValue* value){
    HzValue* copy = malloc(sizeof(HzValue));
    copy->type = value->type;

    switch(value->type){
        case HZVAL_NUM:
            copy->num = value->num; break;
        case HZVAL_DECIMAL:
            copy->dec = value->dec; break;
        case HZVAL_FUN:
            copy->function = value->function; break;

        case HZVAL_ERR:
            copy->err = malloc(strlen(value->err)+1);
            strcpy(copy->err,value->err); break;
        case HZVAL_SYM:
            copy->sym = malloc(strlen(value->sym)+1);
            strcpy(copy->sym,value->sym); break;

        case HZVAL_SEXPR:
        case HZVAL_QEXPR:
            copy->count = value->count;
            copy->cell = malloc(sizeof(HzValue*) * value->count);
            for (int i =0;i < copy->count;i++){
                copy->cell[i] = hzval_copy(value->cell[i]);
            }
        break;
    }

    return copy;
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

HzValue* hzval_join(HzValue* first,HzValue* second){
    while(second->count){
        first = hzval_add(first,hzval_pop(second,0));
    }

    hzval_del(second);
    return first;
}

HzValue* hzval_eval_sexpr(HzEnv* env,HzValue* value){
    for(int i=0;i < value->count;i++){
        value->cell[i] = hzval_eval(env,value->cell[i]);
    }

    for(int i=0;i < value->count;i++){
        if(value->cell[i]->type == HZVAL_ERR) {return hzval_take(value,i);}
    }

    if(value->count==0){return value;}

    if(value->count==1){return hzval_take(value,0);}

    HzValue* first = hzval_pop(value,0);

    if(first->type != HZVAL_FUN){
        hzval_del(value);hzval_del(first);
        return hzval_err("value is not a function");
    }
    // if(first->type != HZVAL_SYM){
    //     hzval_del(first);hzval_del(value);
    //     return hzval_err("S-expression Does not start with symbol!");
    // }

    // HzValue* result = builtin(value,first->sym);
    // hzval_del(first);
    // return result;

    HzValue* result = first->function(env,value);
    hzval_del(first);
    return result;    
}

HzValue* hzval_eval(HzEnv* env,HzValue* value){
    if(value->type == HZVAL_SYM) {
        HzValue* envValue = hzenv_get(env,value);
        hzval_del(value);
        return envValue;
    }
    if(value->type == HZVAL_SEXPR) { return hzval_eval_sexpr(env,value);}

    return value;
}