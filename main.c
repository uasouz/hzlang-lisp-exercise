#include <stdio.h>
#include <stdlib.h>
// #include "mpc.h"
#include "hzval.h"

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

    mpc_parser_t **parsers = malloc(sizeof(mpc_parser_t *) * 6);

    mpc_parser_t *Number = mpc_new("number");
    mpc_parser_t *Decimal = mpc_new("decimal");
    mpc_parser_t *Symbol = mpc_new("symbol");
    mpc_parser_t *Sexpression = mpc_new("sexpression");
    mpc_parser_t *Expression = mpc_new("expression");
    mpc_parser_t *Hz = mpc_new("hz");

    mpca_lang(MPCA_LANG_DEFAULT,
              "   \
        number: /[-+]?[0-9]+/ ;                \
        decimal: /[-+]?[0-9]+[.]?[0-9]+/ ;                \
        symbol: '+' | '-' | '*' | '/' | '%' | '^' ;                \
        sexpression: '(' <expression>* ')'; \
        expression: (<decimal> | <number> ) | <symbol> | <sexpression>  ;  \
        hz: /^/ <expression>* /$/ ;                \
    ",
              Number, Decimal, Symbol, Sexpression, Expression, Hz);
    parsers[0] = Number;
    parsers[1] = Decimal;
    parsers[2] = Symbol;
    parsers[3] = Sexpression;
    parsers[4] = Expression;
    parsers[5] = Hz;
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

    puts("HzLisp v0.0.4 by Hadara");
    puts("Press Ctrl+C to exit");

    while (1)
    {
        char *input = readline("hz> ");
        add_history(input);

        mpc_result_t r;
        if (mpc_parse("<stdin>", input, parsers[5], &r))
        {
            HzValue* parsedData = hzval_read(r.output);
            HzValue* result = hzval_eval(parsedData);
            hzval_println(result);
            hzval_del(result);
            mpc_ast_delete(r.output);
        }
        else
        {
            mpc_err_print(r.error);
            mpc_err_delete(r.error);
        }

        free(input);
    }

    mpc_list_cleanup(6, parsers);
    return 0;
}