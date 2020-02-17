#include <stdio.h>
#include <stdlib.h>
#include "mpc.h"

//HZ Evaluation Value

//Evaluation Types Enum
enum {HZVAL_NUM,HZVAL_ERR};

//Errors Types Enum
enum {HZERR_DIV_ZERO,HZERR_BAD_OP,HZERR_BAD_NUM};
typedef struct {
    int type;
    long num;
    int err;
} HzValue;

HzValue hzval_num(long value){
    HzValue hzValue;
    hzValue.type = HZVAL_NUM;
    hzValue.num = value;
    return hzValue;
}

HzValue hzval_err(int err){
    HzValue hzValue;
    hzValue.type = HZVAL_ERR;
    hzValue.err = err;
    return hzValue;
}

char* join_string(char* first,char* second){
    char* buf = malloc(sizeof(char)* 2048);
    snprintf(buf,2047,"%s%s",first,second);
    return buf;
}

char* create_error_message(char* message){
    return join_string("Error: ", message);
}

void hzval_print(HzValue value){
    switch (value.type)
    {
    case HZVAL_NUM:
        printf("%li",value.num);
        break;
    
    case HZVAL_ERR:
        if(value.err == HZERR_DIV_ZERO){
            printf("%s",create_error_message("Division by zero"));
        }
        if(value.err == HZERR_BAD_OP){
            printf("%s",create_error_message("Invalid Operator"));
        }
        if(value.err == HZERR_BAD_NUM){
            printf("%s",create_error_message("Invalid Number"));
        }
        break;
    }
}

void hzval_println(HzValue value) { hzval_print(value); putchar('\n');}
//End HZ Evaluation Value

//Parser section
void mpc_list_cleanup(int n, mpc_parser_t **list)
{
    int i;
    for (i = 0; i < n; i++)
    {
        mpc_undefine(list[i]);
    }
    for (i = 0; i < n; i++)
    {
        mpc_delete(list[i]);
    }
}

mpc_parser_t **create_basic_parser()
{

    mpc_parser_t **parsers = malloc(sizeof(mpc_parser_t *) * 4);

    mpc_parser_t *Number = mpc_new("number");
    mpc_parser_t *Decimal = mpc_new("decimal");
    mpc_parser_t *Operator = mpc_new("operator");
    mpc_parser_t *Expression = mpc_new("expression");
    mpc_parser_t *Hz = mpc_new("hz");

    mpca_lang(MPCA_LANG_DEFAULT,
              "   \
        number: /[-+]?[0-9]+/ ;                \
        decimal: /[-+]?[0-9]+[.]?[0-9]+/ ;                \
        operator: '+' | '-' | '*' | '/' | '%' | '^' ;                \
        expression: <number> | <decimal> | '(' <operator> <expression>+ ')' ;                \
        hz: /^/ <operator> <expression>+ /$/ ;                \
    ",
              Number, Decimal, Operator, Expression, Hz);
    parsers[0] = Number;
    parsers[1] = Decimal;
    parsers[2] = Operator;
    parsers[3] = Expression;
    parsers[4] = Hz;
    return parsers;
}

//End parser section

//AST Traversal Section
int number_of_nodes(mpc_ast_t* tree){
    if(tree->children_num==0){ return 1;}
    if(tree->children_num >=1 ){
        int total = 1;
        for(int i =0;i < tree->children_num;i++){
            total = total + number_of_nodes(tree->children[i]);
        }
        return total;
    }
    return 0;
}

HzValue eval_op(HzValue x, char* op, HzValue y) {

    if(x.type==HZVAL_ERR){ return x;}
    if(y.type==HZVAL_ERR) { return y;}

  if (strcmp(op, "+") == 0) { return hzval_num(x.num + y.num); }
  if (strcmp(op, "-") == 0) { return hzval_num(x.num - y.num); }
  if (strcmp(op, "*") == 0) { return hzval_num(x.num * y.num); }
  if (strcmp(op, "/") == 0) {
      if(y.num==0){
          return hzval_err(HZERR_DIV_ZERO);
      } else {
        return hzval_num(x.num / y.num);
      }
    }
  if (strcmp(op, "%") == 0) { return hzval_num(x.num % y.num); }
  if (strcmp(op, "^") == 0) { return hzval_num(pow(x.num,y.num)); }
  return hzval_err(HZERR_BAD_OP);
}

HzValue eval(mpc_ast_t* tree){
    //Evaluation base case
    if(strstr(tree->tag,"number")){
        errno = 0;
        long number = strtol(tree->contents,NULL,10);
        return errno != ERANGE ? hzval_num(number) : hzval_err(HZERR_BAD_NUM);
    }

    //Evaluation recursive case
    //The operator is always the second child,the first will be a '('
    char* op = tree->children[1]->contents;

    HzValue evaluated = eval(tree->children[2]);

    int i = 3;
    while (strstr(tree->children[i]->tag,"expression"))
    {
        evaluated = eval_op(evaluated,op,eval(tree->children[i]));
        i++;
    }
    
    return evaluated;
}
//End AST Traversal Section

#ifdef _WIN32
#include <string.h>

static char buffer[2048];

char *readline(char *prompt)
{
    fputs(prompt, stdout);
    fgets(buffer, 2048, stdin);
    char *cpy = malloc(strlen(buffer) + 1);
    strcpy(cpy, buffer);
    cpy[strlen(cpy) - 1] = '\0';
    return cpy;
}

void add_history(char *unused) {}

#else
#include <editline/readline.h>
#ifdef __linux
#include <editline/history.h>
#endif
#endif

int main(int argc, char **argv)
{
    mpc_parser_t **parsers = create_basic_parser();

    puts("HzLisp v0.0.1 by Hadara");
    puts("Press Ctrl+C to exit");

    while (1)
    {
        char *input = readline("hz> ");
        add_history(input);

        mpc_result_t r;
        if (mpc_parse("<stdin>", input, parsers[4], &r))
        {
            HzValue result = eval(r.output);
            hzval_println(result);
            mpc_ast_delete(r.output);
        }
        else
        {
            mpc_err_print(r.error);
            mpc_err_delete(r.error);
        }

        free(input);
    }

    mpc_list_cleanup(4, parsers);
    return 0;
}