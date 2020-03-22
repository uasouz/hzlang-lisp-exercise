#include "hzval.h"
#include "builtin.h"
#include "builtin/hashmap.h"

//Utils
char *join_string(char *first, char *second) {
    char *buf = malloc(sizeof(char) * 2048);
    snprintf(buf, 2047, "%s%s", first, second);
    return buf;
}

char *create_error_message(char *message) {
    return join_string("Error: ", message);
}

char *hztype_name(int type) {
    switch (type) {
        case HZVAL_FUN:
            return "Function";
        case HZVAL_NUM:
            return "Number";
        case HZVAL_ERR:
            return "Error";
        case HZVAL_SYM:
            return "Symbol";
        case HZVAL_SEXPR:
            return "S-Expression";
        case HZVAL_QEXPR:
            return "Q-Expression";
        default:
            return "Unknown";
    }
}
//End Utils

//Types Constructors
HzValue *hzval_num(long value) {
    HzValue *hzValue = malloc(sizeof(HzValue));
    hzValue->type = HZVAL_NUM;
    hzValue->num = value;
    return hzValue;
}

HzValue *hzval_decimal(double value) {
    HzValue *hzValue = malloc(sizeof(HzValue));
    hzValue->type = HZVAL_DECIMAL;
    hzValue->dec = value;
    return hzValue;
}

HzValue *hzval_err(char *err, ...) {
    HzValue *hzValue = malloc(sizeof(HzValue));
    hzValue->type = HZVAL_ERR;

    va_list va;
    va_start(va, err);

    hzValue->err = malloc(strlen(err) + 1024);

    vsnprintf(hzValue->err, strlen(err) + 1023, err, va);

    hzValue->err = realloc(hzValue->err, strlen(hzValue->err) + 1);

    va_end(va);

    return hzValue;
}

HzValue *hzval_boolean(int bool){
    HzValue *hzValue = malloc(sizeof(HzValue));
    hzValue->type = HZVAL_BOOLEAN;
    hzValue->num = bool;
    return hzValue;
}

HzValue *hzval_command(char *sym) {
    HzValue *hzValue = malloc(sizeof(HzValue));
    hzValue->type = HZVAL_COMMAND;
    hzValue->sym = malloc(strlen(sym) + 1);
    strcpy(hzValue->sym, sym);
    return hzValue;
}

HzValue *hzval_sym(char *sym) {
    HzValue *hzValue = malloc(sizeof(HzValue));
    hzValue->type = HZVAL_SYM;
    hzValue->sym = malloc(strlen(sym) + 1);
    strcpy(hzValue->sym, sym);
    return hzValue;
}


struct HzEnv {
    struct HzEnv *parent;
    struct hashmap store;
};

HzValue *hzval_lambda(HzEnv *parent, HzValue *formals, HzValue *body) {
    HzValue *hzValue = malloc(sizeof(HzValue));
    hzValue->type = HZVAL_FUN;

    /*Making sure it is not a builtin*/
    hzValue->builtin = NULL;
    hzValue->functionEnv = hzenv_new();
    hzValue->functionEnv->parent = parent;

    hzValue->formals = formals;
    hzValue->body = body;

    return hzValue;
}

HzValue *hzval_function(HzFunction function, char *sym) {
    HzValue *hzValue = malloc(sizeof(HzValue));
    hzValue->type = HZVAL_FUN;
    hzValue->sym = malloc(strlen(sym) + 1);
    strcpy(hzValue->sym, sym);
    hzValue->builtin = function;
    return hzValue;
}

HzValue *hzval_sexpression(void) {
    HzValue *hzValue = malloc(sizeof(HzValue));
    hzValue->type = HZVAL_SEXPR;
    hzValue->count = 0;
    hzValue->cell = NULL;
    return hzValue;
}

HzValue *hzval_qexpression(void) {
    HzValue *hzValue = malloc(sizeof(HzValue));
    hzValue->type = HZVAL_QEXPR;
    hzValue->count = 0;
    hzValue->cell = NULL;
    return hzValue;
}
//End Types Constructors

//Environment
typedef struct HzEnvMapEntry {
    struct hashmap_entry ent;
    HzValue *value;
} HzEnvMapEntry;

HzEnvMapEntry *hzenv_map_entry_new(HzValue *key, HzValue *value) {
    HzEnvMapEntry *envEntry = malloc(sizeof(HzEnvMapEntry));
    hashmap_entry_init(envEntry, hash(key->sym, sizeof(key->sym)));
    envEntry->value = hzval_copy(value);
    return envEntry;
}

HzValue *hzenv_get(HzEnv *env, HzValue *key) {
    struct HzEnvMapEntry keyEntry;
    hashmap_entry_init(&keyEntry, hash(key->sym, sizeof(key->sym)));
    struct HzEnvMapEntry *entry = hashmap_get(&env->store, &keyEntry, NULL);
    if (entry != NULL) {
        return hzval_copy(entry->value);
    }

    if (env->parent) {
        return hzenv_get(env->parent, key);
    } else {
        return hzval_err("Unbound Symbol '%s'", key->sym);
    }
}

int hzenv_check_builtin_functions(HzEnv *env, HzValue *key) {
    HzValue *value = hzenv_get(env, key);
//    if (value->type == HZVAL_FUN && strstr(value->sym, key->sym)) {
    if (value->type == HZVAL_FUN && value->builtin != NULL) {
        return 1;
    }
    return 0;
}

int hzenv_put(HzEnv *env, HzValue *key, HzValue *value) {
    if (hzenv_check_builtin_functions(env, key)) {
        return 0;
    }
    HzEnvMapEntry *entry = hzenv_map_entry_new(key, value);
    hashmap_put(&env->store, entry);
    return 1;
}

int hzenv_def(HzEnv *env, HzValue *key, HzValue *value) {

    while (env->parent) { env = env->parent; }

    return hzenv_put(env, key, value);
}

HzEnv *hzenv_new(void) {
    HzEnv *env = malloc(sizeof(HzEnv));
    env->parent = NULL;
    hashmap_init(&env->store, NULL, NULL, 0);
    return env;
}

HzEnv *hzenv_copy(HzEnv *env) {
    HzEnv *copy = hzenv_new();
    struct hashmap_iter iter;
    struct hashmap_entry *e;
    hashmap_iter_init(&env->store, &iter);
    while ((e = hashmap_iter_next(&iter))) {
        hashmap_put(&copy->store, e);
    }
    copy->parent = env->parent;
    return copy;
}

void hzenv_del(HzEnv *env) {
    hashmap_free(&env->store, 0);
    free(env);
}

void hzenv_add_function(HzEnv *env, char *name, HzFunction func) {
    HzValue *key = hzval_sym(name);
    HzValue *value = hzval_function(func, name);
    hzenv_put(env, key, value);
    hzval_del(key);
    hzval_del(value);
}

void hzenv_add_builtins(HzEnv *env) {
    /*Lambda function*/
    hzenv_add_function(env, "\\", builtin_lambda);

    /* List Functions */
    hzenv_add_function(env, "list", builtin_list);
    hzenv_add_function(env, "head", builtin_head);
    hzenv_add_function(env, "tail", builtin_tail);
    hzenv_add_function(env, "eval", builtin_eval);
    hzenv_add_function(env, "join", builtin_join);
    hzenv_add_function(env, "cons", builtin_cons);
    hzenv_add_function(env, "def", builtin_def);
    hzenv_add_function(env, "fun", builtin_fun);
    hzenv_add_function(env, "len", builtin_len);
    hzenv_add_function(env, "=", builtin_put);

    /* Mathematical Functions */
    hzenv_add_function(env, "+", builtin_add);
    hzenv_add_function(env, "-", builtin_sub);
    hzenv_add_function(env, "*", builtin_mul);
    hzenv_add_function(env, "/", builtin_div);
    hzenv_add_function(env, "^", builtin_pow);
}
//End Enviroment

void hzval_del(HzValue *hzValue) {
    switch (hzValue->type) {
        case HZVAL_NUM || HZVAL_DECIMAL:
            break;
        case HZVAL_FUN:
            if (!hzValue->builtin) {
                hzenv_del(hzValue->functionEnv);
                hzval_del(hzValue->formals);
                hzval_del(hzValue->body);
            }
            break;
        case HZVAL_ERR:
            free(hzValue->err);
            break;
        case HZVAL_SYM:
            free(hzValue->sym);
            break;

        case HZVAL_QEXPR:
        case HZVAL_SEXPR:
            for (int i = 0; i < hzValue->count; i++) {
                hzval_del(hzValue->cell[i]);
            }
            free(hzValue->cell);
            break;
    }
    free(hzValue);
}


void hzval_print_type(HzValue *value) {
    switch (value->type) {
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
            printf("HZVAL_SYM");
            break;
        case HZVAL_COMMAND:
            printf("HZVAL_COMMAND");
            break;
        case HZVAL_SEXPR:
            printf("HZVAL_SEXPR");
            break;
        case HZVAL_QEXPR:
            printf("HZVAL_QEXPR");
            break;
        case HZVAL_FUN:
            printf("HZVAL_FUN");
            break;
    }
}

void hzval_print(HzValue *value) {
    switch (value->type) {
        case HZVAL_NUM:
            printf("%li", value->num);
            break;
        case HZVAL_BOOLEAN:
            if(value->num == 0){
                printf("false");
            } else{
                printf("true");
            }
            break;
        case HZVAL_DECIMAL:
            printf("%f", value->dec);
            break;
        case HZVAL_ERR: {
            char *error = create_error_message(value->err);
            printf("%s", error);
            free(error);
            break;
        }
        case HZVAL_COMMAND:
        case HZVAL_SYM:
            printf("%s", value->sym);
            break;
        case HZVAL_SEXPR:
            hzval_expr_print(value, '(', ')');
            break;
        case HZVAL_QEXPR:
            hzval_expr_print(value, '{', '}');
            break;
        case HZVAL_FUN:
            if (value->builtin) {
                printf("<builtin>: %s", value->sym);
            } else {
                printf("(\\");
                hzval_print(value->formals);
                putchar(' ');
                hzval_print(value->body);
                putchar(')');
            }
            break;
    }
}

void hzval_println(HzValue *value) {
    hzval_print(value);
    putchar('\n');
}

void hzval_details_println(HzValue *value) {
    hzval_print_type(value);
    printf("::");
    hzval_print(value);
    printf("::");
    printf("%d", value->count);
    putchar('\n');
}

HzValue *hzval_read_num(mpc_ast_t *tree) {
    errno = 0;
    long number = strtol(tree->contents, NULL, 10);
    return errno != ERANGE ? hzval_num(number) : hzval_err("Invalid Number");
}

HzValue *hzval_read_decimal(mpc_ast_t *tree) {
    errno = 0;
    double number = strtod(tree->contents, NULL);
    return errno != ERANGE ? hzval_decimal(number) : hzval_err("Invalid Number");
}

HzValue *hzval_add(HzValue *parent, HzValue *child) {
    parent->count++;
    parent->cell = realloc(parent->cell, sizeof(HzValue *) * parent->count);
    parent->cell[parent->count - 1] = child;
    return parent;
}

HzValue *hzval_read(mpc_ast_t *tree) {
    if (strstr(tree->tag, "command")) {
        return hzval_command(tree->contents);
    }
    if (strstr(tree->tag, "number")) {
        return hzval_read_num(tree);
    }
    if (strstr(tree->tag, "decimal")) {
        return hzval_read_decimal(tree);
    }
    if (strstr(tree->tag, "symbol")) {
        if(strcmp(tree->contents,"true")==0){
            return hzval_boolean(1);
        } else if(strcmp(tree->contents,"false")==0) {
            return hzval_boolean(0);
        }
        return hzval_sym(tree->contents);
    }

    HzValue *read = NULL;
    if (strcmp(tree->tag, ">") == 0) {
        read = hzval_sexpression();
    }
    if (strstr(tree->tag, "sexpression")) {
        read = hzval_sexpression();
    }
    if (strstr(tree->tag, "qexpression")) {
        read = hzval_qexpression();
    }

    for (int i = 0; i < tree->children_num; i++) {
        if (strcmp(tree->children[i]->contents, "(") == 0) { continue; }
        if (strcmp(tree->children[i]->contents, ")") == 0) { continue; }
        if (strcmp(tree->children[i]->contents, "{") == 0) { continue; }
        if (strcmp(tree->children[i]->contents, "}") == 0) { continue; }
        if (strcmp(tree->children[i]->tag, "regex") == 0) { continue; }
        read = hzval_add(read, hzval_read(tree->children[i]));
    }

    return read;
}

HzValue *hzval_copy(HzValue *value) {
    HzValue *copy = malloc(sizeof(HzValue));
    copy->type = value->type;

    switch (value->type) {
        case HZVAL_NUM:
            copy->num = value->num;
            break;
        case HZVAL_BOOLEAN:
            copy->num = value->num;
            break;
        case HZVAL_DECIMAL:
            copy->dec = value->dec;
            break;
        case HZVAL_FUN:
            if (value->builtin) {
                copy->builtin = value->builtin;
                copy->sym = malloc(strlen(value->sym) + 1);
                strcpy(copy->sym, value->sym);
            } else {
                copy->builtin = NULL;
                copy->functionEnv = hzenv_copy(value->functionEnv);
                copy->formals = hzval_copy(value->formals);
                copy->body = hzval_copy(value->body);
            }
            break;

        case HZVAL_ERR:
            copy->err = malloc(strlen(value->err) + 1);
            strcpy(copy->err, value->err);
            break;
        case HZVAL_SYM:
            copy->sym = malloc(strlen(value->sym) + 1);
            strcpy(copy->sym, value->sym);
            break;

        case HZVAL_SEXPR:
        case HZVAL_QEXPR:
            copy->count = value->count;
            copy->cell = malloc(sizeof(HzValue *) * value->count);
            for (int i = 0; i < copy->count; i++) {
                copy->cell[i] = hzval_copy(value->cell[i]);
            }
            break;
    }

    return copy;
}

void hzval_expr_print(HzValue *hzvalue, char open, char close) {
    putchar(open);
    for (int i = 0; i < hzvalue->count; i++) {
        hzval_print(hzvalue->cell[i]);

        if (i != (hzvalue->count - 1)) {
            putchar(' ');
        }
    }
    putchar(close);
}

HzValue *hzval_pop(HzValue *value, int position) {
    HzValue *item = value->cell[position];

    memmove(&value->cell[position], &value->cell[position + 1],
            sizeof(HzValue *) * (value->count - position - 1));

    value->count--;

    value->cell = realloc(value->cell, sizeof(HzValue *) * value->count);

    return item;
}

HzValue *hzval_take(HzValue *value, int position) {
    HzValue *item = hzval_pop(value, position);
    hzval_del(value);
    return item;
}

HzValue *hzval_join(HzValue *first, HzValue *second) {
    while (second->count) {
        first = hzval_add(first, hzval_pop(second, 0));
    }

    hzval_del(second);
    return first;
}


HzValue *hzval_call(HzEnv *env, HzValue *function, HzValue *body) {

    if (function->builtin) { return function->builtin(env, body); }

    int given = body->count;
    int total = function->formals->count;

    while (body->count) {

        if (function->formals->count == 0) {
            hzval_del(body);
            return hzval_err(
                    "Function passed too many arguments. "
                    "Got %i, Expected %i.", given, total);
        }

        HzValue *symbol = hzval_pop(function->formals, 0);

        if (strcmp(symbol->sym, "&") == 0) {
            if (function->formals->count != 1) {
                hzval_del(body);
                return hzval_err("Function format invalid. "
                                 "Symbol '&' not followed by single symbol.");
            }

            HzValue *nSymbols = hzval_pop(function->formals, 0);
            hzenv_put(function->functionEnv, nSymbols, builtin_list(env, body));
            hzval_del(symbol);
            hzval_del(nSymbols);
            break;
        }

        HzValue *value = hzval_pop(body, 0);

        hzenv_put(function->functionEnv, symbol, value);

        hzval_del(symbol);
        hzval_del(value);
    }

    hzval_del(body);

    if (function->formals->count > 0 &&
        strcmp(function->formals->cell[0]->sym, "&") == 0) {
        if (function->formals->count != 2) {
            return hzval_err("Function format invalid. "
                             "Symbol '&' not followed by single symbol.");
        }

        hzval_del(hzval_pop(function->formals, 0));

        HzValue *symbol = hzval_pop(function->formals, 0);

        HzValue *value = hzval_qexpression();

        hzenv_put(function->functionEnv, symbol, value);

        hzval_del(symbol);
        hzval_del(value);
    }

    if (function->formals->count == 0) {

        function->functionEnv->parent = env;

        return builtin_eval(
                function->functionEnv, hzval_add(hzval_sexpression(), hzval_copy(function->body)));
    } else {
        return hzval_copy(function);
    }

}

HzValue *hzval_eval_sexpr(HzEnv *env, HzValue *value, int *running) {
    for (int i = 0; i < value->count; i++) {
        value->cell[i] = hzval_eval(env, value->cell[i], running);
    }

    for (int i = 0; i < value->count; i++) {
        if (value->cell[i]->type == HZVAL_ERR) { return hzval_take(value, i); }
    }

    if (value->count == 0) { return value; }

    if (value->count == 1) { return hzval_take(value, 0); }

    HzValue *first = hzval_pop(value, 0);

    if (first->type != HZVAL_FUN) {
        HzValue *err = hzval_err(
                "S-Expression starts with incorrect type. "
                "Got %s, Expected %s.", hztype_name(first->type),
                hztype_name(HZVAL_FUN)
        );
        hzval_del(value);
        hzval_del(first);
        return err;
    }

    HzValue *result = hzval_call(env, first, value);
    hzval_del(first);
    return result;
}

HzValue *hzval_eval(HzEnv *env, HzValue *value, int *running) {
    if (value->type == HZVAL_COMMAND) {
        if (strstr(value->sym, ".exit")) {
            *running = 0;
        } else {
            return hzval_err("Unknow Command '%s'", value->sym);
        }
        return value;
    }
    if (value->type == HZVAL_SYM) {
        HzValue *envValue = hzenv_get(env, value);
        hzval_del(value);
        return envValue;
    }
    if (value->type == HZVAL_SEXPR) { return hzval_eval_sexpr(env, value, running); }

    return value;
}