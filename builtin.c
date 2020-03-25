#include "builtin.h"

#define LASSERT(args, cond, err, ...) \
  if (!(cond)) {  HzValue* error = hzval_err(err,##__VA_ARGS__);hzval_del(args); return error;}

#define LASSERT_ARGUMENT_COUNT(function_name, value, number_of_arguments) \
  LASSERT(value,value->count == number_of_arguments,"Function '%s' passed incorrect number of arguments.Got %i, Expected %i.",function_name,value->count,number_of_arguments);

#define LASSERT_TYPE(function_name, value, index, expected) \
 LASSERT(value, value->cell[index]->type == expected, \
         "Function %s passed incorrect type.Got %s, Expected %s.", function_name, \
        hztype_name(value->cell[index]->type), hztype_name(expected));

#define LASSERT_NOT_EMPTY(func, value, index) \
  LASSERT(value, value->cell[index]->count != 0, \
    "Function '%s' passed {} for argument %i.", func, index);

HzValue *eval_op(HzValue *x, char *op, HzValue *y) {
    if (x->type == HZVAL_ERR) { return hzval_err(x->err); }
    if (y->type == HZVAL_ERR) { return hzval_err(y->err); }

    if (x->type == HZVAL_DECIMAL || y->type == HZVAL_DECIMAL) {
        if (x->type == HZVAL_NUM) {
            x->dec = x->num;
        }
        if (y->type == HZVAL_NUM) {
            y->dec = y->num;
        }

        if (strcmp(op, "+") == 0) { return hzval_decimal(x->dec + y->dec); }
        if (strcmp(op, "-") == 0) { return hzval_decimal(x->dec - y->dec); }
        if (strcmp(op, "*") == 0) { return hzval_decimal(x->dec * y->dec); }
        if (strcmp(op, "/") == 0) {
            if (y->dec == 0) {
                return hzval_err("Division by zero");
            } else {
                return hzval_decimal(x->dec / y->dec);
            }
        }
        if (strcmp(op, "%") == 0) { return hzval_decimal(fmod(x->dec, y->dec)); }
        if (strcmp(op, "^") == 0) { return hzval_decimal(pow(x->dec, y->dec)); }
    }
    if (strcmp(op, "+") == 0) { return hzval_num(x->num + y->num); }
    if (strcmp(op, "-") == 0) { return hzval_num(x->num - y->num); }
    if (strcmp(op, "*") == 0) { return hzval_num(x->num * y->num); }
    if (strcmp(op, "/") == 0) {
        if (y->num == 0) {
            return hzval_err("Division by zero");
        } else {
            return hzval_num(x->num / y->num);
        }
    }
    if (strcmp(op, "%") == 0) { return hzval_num(x->num % y->num); }
    if (strcmp(op, "^") == 0) { return hzval_num(pow(x->num, y->num)); }
    return hzval_err("Bad Operation");
}

HzValue *builtin_op(HzEnv *env, HzValue *value, char *op) {
    for (int i = 0; i < value->count; i++) {
        if (!(value->cell[i]->type == HZVAL_NUM || value->cell[i]->type == HZVAL_DECIMAL)) {
            hzval_del(value);
            return hzval_err("Cannot operate on non-number");
        }
    }
    HzValue *start = hzval_pop(value, 0);

    if ((strcmp(op, "-") == 0) && value->count == 0) {
        start->num = -start->num;
    }

    while (value->count > 0) {
        HzValue *next = hzval_pop(value, 0);

        start = eval_op(start, op, next);

        hzval_del(next);
    }
    hzval_del(value);
    return start;
}

HzValue *builtin_head(HzEnv *env, HzValue *value) {
    LASSERT_ARGUMENT_COUNT("head", value, 1);
    LASSERT_TYPE("head", value, 0, HZVAL_QEXPR);
    LASSERT_NOT_EMPTY("head", value, 0);

    HzValue *head = hzval_take(value, 0);

    while (head->count > 1) { hzval_del(hzval_pop(head, 1)); }
    // hzval_details_println(head);
    return head;
}

HzValue *builtin_tail(HzEnv *env, HzValue *value) {
//    LASSERT(value, value->count == 1, "Function 'tail' passed too many arguments.Got %i, Expected %i.", value->count,
//            1);
//    LASSERT(value, value->cell[0]->type == HZVAL_QEXPR, "Function 'tail' passed incorrect type.Got %s, Expected %s.",
//            hztype_name(value->cell[0]->type), hztype_name(HZVAL_QEXPR));
//    LASSERT(value, value->cell[0]->count != 0, "Function 'tail' passed '{}'.");
    LASSERT_ARGUMENT_COUNT("tail", value, 1);
    LASSERT_TYPE("tail", value, 0, HZVAL_QEXPR);
    LASSERT_NOT_EMPTY("tail", value, 0);

    HzValue *tail = hzval_take(value, 0);

    hzval_del(hzval_pop(tail, 0));

    return tail;
}

HzValue *builtin_list(HzEnv *env, HzValue *value) {
    value->type = HZVAL_QEXPR;
    return value;
}

HzValue *builtin_eval(HzEnv *env, HzValue *value) {
//    LASSERT(value, value->count == 1, "Function 'eval' passed too many arguments.Got %i, Expected %i.", value->count,
//            1);
//    LASSERT(value, value->cell[0]->type == HZVAL_QEXPR, "Function 'eval' passed incorrect type.Got %s, Expected %s.",
//            hztype_name(value->cell[0]->type), hztype_name(HZVAL_QEXPR));

    LASSERT_ARGUMENT_COUNT("eval", value, 1);
    LASSERT_TYPE("eval", value, 0, HZVAL_QEXPR);

    HzValue *evaluable = hzval_take(value, 0);
    evaluable->type = HZVAL_SEXPR;
    int r = 1;
    return hzval_eval(env, evaluable, &r);
}

HzValue *builtin_join(HzEnv *env, HzValue *value) {

    // LASSERT(value,value->count == 2,"Function 'join' passed too many arguments.Got %i, Expected %i.",value->count,2);

    for (int i = 0; i < value->count; i++) {
        LASSERT(value, value->cell[i]->type == HZVAL_QEXPR,
                "Function 'join' passed incorrect type.Got %s, Expected %s.", hztype_name(value->cell[i]->type),
                hztype_name(HZVAL_QEXPR));
    }
    // hzval_details_println(value);
    HzValue *accumulator = hzval_pop(value, 0);

    while (value->count) {
        accumulator = hzval_join(accumulator, hzval_pop(value, 0));
    }

    hzval_del(value);
    return accumulator;
}

HzValue *builtin_cons(HzEnv *env, HzValue *value) {
//    LASSERT(value, value->count == 2, "Function 'cons' passed wrong number of arguments.Got %i, Expected %i.",
//            value->count, 2);
//    LASSERT(value, value->cell[1]->type == HZVAL_QEXPR,
//            "Function 'cons' passed incorrect type for second value,must be a Q-Expression.Got %s, Expected %s.",
//            hztype_name(value->cell[1]->type), hztype_name(HZVAL_QEXPR));

    LASSERT_ARGUMENT_COUNT("cons", value, 2);
    LASSERT_TYPE("cons", value, 1, HZVAL_QEXPR);

    if (!(value->cell[0]->type == HZVAL_QEXPR || value->cell[0]->type == HZVAL_SEXPR)) {
        HzValue *parent = hzval_qexpression();
        value->cell[0] = hzval_add(parent, value->cell[0]);
    }

    return builtin_join(env, value);
}

HzValue *builtin_len(HzEnv *env, HzValue *value) {
//    LASSERT(value, value->count == 1, "Function 'eval' passed too many arguments.Got %i, Expected %i.", value->count,
//            1);
//    LASSERT(value, value->cell[0]->type == HZVAL_QEXPR, "Function 'eval' passed incorrect type.Got %s, Expected %s.",
//            hztype_name(value->cell[0]->type), hztype_name(HZVAL_QEXPR));

    LASSERT_ARGUMENT_COUNT("len", value, 1);
    LASSERT_TYPE("len", value, 0, HZVAL_QEXPR);

    return hzval_num(value->cell[0]->count);
}


HzValue *builtin_add(HzEnv *env, HzValue *value) {
    return builtin_op(env, value, "+");
}

HzValue *builtin_sub(HzEnv *env, HzValue *value) {
    return builtin_op(env, value, "-");
}

HzValue *builtin_mul(HzEnv *env, HzValue *value) {
    return builtin_op(env, value, "*");
}

HzValue *builtin_div(HzEnv *env, HzValue *value) {
    return builtin_op(env, value, "/");
}

HzValue *builtin_pow(HzEnv *env, HzValue *value) {
    return builtin_op(env, value, "^");
}

/*deprecated*/
//HzValue *builtin_def(HzEnv *env, HzValue *value) {
//    LASSERT(value, value->cell[0]->type == HZVAL_QEXPR, "Function 'def' passed incorrect type.");
//
//    HzValue *symbols = value->cell[0];
//
//    for (int i = 0; i < symbols->count; i++) {
//        LASSERT(value, symbols->cell[i]->type == HZVAL_SYM, "Function 'def' cannot define non-symbol");
//    }
//
//    LASSERT(value, symbols->count == value->count - 1, "Function 'def cannot define incorrect "
//                                                       "number of values to symbols");
//
//    for (int i = 0; i < symbols->count; i++) {
//        if (!hzenv_put(env, symbols->cell[i], value->cell[i + 1])) {
//            hzval_del(value);
//            return hzval_err("builtin values cannot be reassigned");
//        }
//    }
//
//    hzval_del(value);
//    return hzval_sexpression();
//}

HzValue *builtin_var(HzEnv *env, HzValue *value, char *function_name) {
    LASSERT_TYPE(function_name, value, 0, HZVAL_QEXPR);

    HzValue *symbols = value->cell[0];

    for (int i = 0; i < symbols->count; i++) {
        LASSERT(value, symbols->cell[i]->type == HZVAL_SYM,
                "Function '%s' cannot define non-symbol. "
                "Got %s, Expected %s.", function_name,
                hztype_name(symbols->cell[i]->type),
                hztype_name(HZVAL_SYM));
    }

    LASSERT(value, symbols->count == value->count - 1,
            "Function '%s' passed too many arguments for symbols. "
            "Got %i, Expected %i.", symbols->count, value->count - 1);

    for (int i = 0; i < symbols->count; i++) {

        if (strcmp(function_name, "def") == 0) {
            if (!hzenv_def(env, symbols->cell[i], value->cell[i + 1])) {
                hzval_del(value);
                return hzval_err("builtin values cannot be reassigned");
            }
        }

        if (strcmp(function_name, "=") == 0) {
            if (!hzenv_put(env, symbols->cell[i], value->cell[i + 1])) {
                hzval_del(value);
                return hzval_err("builtin values cannot be reassigned");
            }
        }
    }

    hzval_del(value);
    return hzval_sexpression();
}

HzValue *builtin_def(HzEnv *env, HzValue *value) {
    return builtin_var(env, value, "def");
}

HzValue *builtin_put(HzEnv *env, HzValue *value) {
    return builtin_var(env, value, "=");
}

HzValue *builtin_lambda(HzEnv *env, HzValue *value) {
    LASSERT_ARGUMENT_COUNT("\\", value, 2);
    LASSERT_TYPE("\\", value, 0, HZVAL_QEXPR);
    LASSERT_TYPE("\\", value, 1, HZVAL_QEXPR);

    HzValue *symbols = value->cell[0];

    for (int i = 0; i < symbols->count; i++) {
        LASSERT(value, symbols->cell[i]->type == HZVAL_SYM,
                "Cannot define non-symbol. Got %s, Expected %s.",
                hztype_name(symbols->cell[i]->type), hztype_name(HZVAL_SYM));
    }

    HzValue *formals = hzval_pop(value, 0);
    HzValue *body = hzval_pop(value, 0);
    hzval_del(value);

    return hzval_lambda(env, formals, body);
}

HzValue *builtin_fun(HzEnv *env, HzValue *value) {
    HzValue *lambda = builtin_lambda(env, value);
    HzValue *body = lambda->body;
    HzValue *args = lambda->formals;
    HzValue *name = hzval_pop(hzval_copy(args), 0);
    HzValue *fArgs = builtin_tail(env, hzval_add(hzval_qexpression(), hzval_copy(args)));

    hzval_details_println(body);
    hzval_details_println(args);
    hzval_details_println(fArgs);
    HzValue *nameValue = hzval_add(hzval_qexpression(), name);
    hzval_details_println(name);
    HzValue *defValue = hzval_qexpression();
    defValue = hzval_add(defValue, nameValue);
    defValue = hzval_add(defValue, hzval_lambda(env, fArgs, body));

    hzval_del(args);
    hzval_details_println(defValue);
    return builtin_def(env, defValue);
}

HzValue *builtin_and(HzEnv *env, HzValue *value) {
    for (int i = 0; i < value->count; i++) {
        if (!(value->cell[i]->type == HZVAL_BOOLEAN || value->cell[i]->type == HZVAL_NUM)) {
            hzval_del(value);
            return hzval_err("Cannot operate on non-number or non-boolean");
        }
    }

    HzValue *first = hzval_pop(value, 0);

    while (value->count > 0) {
        HzValue *next = hzval_pop(value, 0);
        if (first->num && next->num) {
            first = hzval_boolean(1);
        } else {
            return hzval_boolean(0);
        }
        hzval_del(next);
    }
    hzval_del(value);
    return hzval_boolean(1);
}

HzValue *builtin_or(HzEnv *env, HzValue *value) {

    HzValue *first = hzval_pop(value, 0);

    if (first->type != HZVAL_BOOLEAN) {
        return first;
    }

    while (value->count > 0) {
        HzValue *next = hzval_pop(value, 0);
        if (!first->num && !next->num) {
            first = hzval_boolean(0);
        } else {
            return next;
        }
        hzval_del(next);
    }
    hzval_del(value);
    return hzval_boolean(0);
}


HzValue *builtin_not(HzEnv *env, HzValue *value) {
    LASSERT_ARGUMENT_COUNT("not", value, 1);
    LASSERT_TYPE("not", value, 0, HZVAL_BOOLEAN);

    if (value->cell[0]->num == 0) {
        return hzval_boolean(1);
    }
    return hzval_boolean(0);
}

HzValue *builtin_ord(HzEnv *env, HzValue *value, char *op) {
    LASSERT_ARGUMENT_COUNT(op, value, 2)
    for (int i = 0; i < value->count; i++) {
        if (!(value->cell[i]->type == HZVAL_BOOLEAN || value->cell[i]->type == HZVAL_NUM)) {
            hzval_del(value);
            return hzval_err("Cannot compare non-number or non-boolean");
        }
    }


    int result = 0;
    if (strcmp(op, ">") == 0) {
        result = (value->cell[0]->num > value->cell[1]->num);
    }
    if (strcmp(op, ">=") == 0) {
        result = (value->cell[0]->num >= value->cell[1]->num);
    }
    if (strcmp(op, "<") == 0) {
        result = (value->cell[0]->num < value->cell[1]->num);
    }
    if (strcmp(op, "<=") == 0) {
        result = (value->cell[0]->num <= value->cell[1]->num);
    }
    hzval_del(value);
    return hzval_boolean(result);
}

HzValue *builtin_gt(HzEnv *env, HzValue *value) {
    return builtin_ord(env, value, ">");
}

HzValue *builtin_gte(HzEnv *env, HzValue *value) {
    return builtin_ord(env, value, ">=");
}

HzValue *builtin_lt(HzEnv *env, HzValue *value) {
    return builtin_ord(env, value, "<");
}

HzValue *builtin_lte(HzEnv *env, HzValue *value) {
    return builtin_ord(env, value, "<=");
}

HzValue *builtin_cmp(HzEnv *env, HzValue *value, char *op) {
    LASSERT_ARGUMENT_COUNT(op, value, 2)
    int result = 0;
    if (strcmp(op, "==") == 0) {
        result = hzval_eq(value->cell[0], value->cell[1]);
    }
    if (strcmp(op, "!=") == 0) {
        result = !hzval_eq(value->cell[0], value->cell[1]);
    }
    hzval_del(value);
    return hzval_boolean(result);
}

HzValue *builtin_eq(HzEnv *env, HzValue *value) {
    return builtin_cmp(env, value, "==");
}

HzValue *builtin_not_eq(HzEnv *env, HzValue *value) {
    return builtin_cmp(env, value, "!=");
}

HzValue *builtin_if(HzEnv *env, HzValue *value) {
    LASSERT_ARGUMENT_COUNT("if", value, 3)
    LASSERT(value, value->cell[0]->type == HZVAL_NUM || value->cell[0]->type == HZVAL_BOOLEAN,
            "Function %s passed incorrect type.Got %s, Expected %s.", "if",
            hztype_name(value->cell[0]->type), hztype_name(HZVAL_BOOLEAN));
    LASSERT_TYPE("if", value, 1, HZVAL_QEXPR)
    LASSERT_TYPE("if", value, 2, HZVAL_QEXPR)

    HzValue* result;
    value->cell[1]->type = HZVAL_SEXPR;
    value->cell[2]->type = HZVAL_SEXPR;
    int running = 1;
    if(value->cell[0]->num){
        result = hzval_eval(env,value->cell[1],&running);
    } else {
        result = hzval_eval(env,value->cell[2],&running);
    }
    hzval_del(value);
    return result;
}

/*deprecated(?)*/
HzValue *builtin(HzEnv *env, HzValue *value, char *func) {
    if (strcmp("list", func) == 0) { return builtin_list(env, value); }
    if (strcmp("head", func) == 0) { return builtin_head(env, value); }
    if (strcmp("tail", func) == 0) { return builtin_tail(env, value); }
    if (strcmp("join", func) == 0) { return builtin_join(env, value); }
    if (strcmp("eval", func) == 0) { return builtin_eval(env, value); }
    if (strcmp("cons", func) == 0) { return builtin_cons(env, value); }
    if (strcmp("len", func) == 0) { return builtin_len(env, value); }
    if (strstr("+-/*^", func)) { return builtin_op(env, value, func); }
    hzval_del(value);
    return hzval_err("Unknown Function!");
}