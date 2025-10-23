#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

int value_stack[256];
char op_stack[256];
int numTop = -1, opTop = -1;

/*
getOperatorPrecedence :
    This function return the precedence of an operator.
    Like * and / will have higher precedence which is (2).
    While + and - will have lower precedence of (1). 
*/
int getOperatorPrecedence(char op) {
    return (op == '*' || op == '/') ? 2 : 1;
}

/*
applyOperator:
    This function applies a given operator to two integer operands.
    Also handles division by zero and invalid operators.
    And it returns the result of applying the operator.
*/
int applyOperator(int val1, int val2, char op, int *errorFlag) {
    if (op == '/' && val2 == 0) {
        *errorFlag = 2;
        return 0;
    }
    switch (op) {
        case '+': return val1 + val2;
        case '-': return val1 - val2;
        case '*': return val1 * val2;
        case '/': return val1 / val2;
        default:
            *errorFlag = 1;
            return 0;
    }
}

/*
calculate:
    This function evaluates a simple arithmatic expression.
    Also it will handle invalid input, operator precedence and division by zero.
*/
void calculate(char* expr) {
    char* p = expr;
    int errorFlag  = 0;
    while (*p && !errorFlag ) {
        if (isspace((char)*p)) {
            p++;
            continue;
        }
        if (isdigit((char)*p)) {
            value_stack[++numTop] = strtol(p, (char**)&p, 10);
            continue;
        }
        while (opTop > -1 && getOperatorPrecedence(op_stack[opTop]) >= getOperatorPrecedence(*p)) {
            if (numTop < 1) { errorFlag  = 1; break; }
            int val2 = value_stack[numTop--];
            int val1 = value_stack[numTop--];
            char op = op_stack[opTop--];
            if (op == '/' && val2 == 0){
                errorFlag  = 2;
                break; 
            }
            if (op == '+'){
                value_stack[++numTop] = val1 + val2;
            }
            if (op == '-'){
                value_stack[++numTop] = val1 - val2;
            }
            if (op == '*'){
                value_stack[++numTop] = val1 * val2;
            }
            if (op == '/'){
                value_stack[++numTop] = val1 / val2;
            }
        }
        if (strchr("+-*/", *p)) {
            if(p == expr || strchr("+-*/", *(p-1))){
                errorFlag = 1;
                break;
            }
            op_stack[++opTop] = *p++;
        } else {
            errorFlag  = 1;
        }
    }
    while (opTop > -1 && !errorFlag) {
        if (numTop < 1) { 
            errorFlag = 1; 
            break; 
        }
        int val2 = value_stack[numTop--];
        int val1 = value_stack[numTop--];
        char op = op_stack[opTop--];
        int result = applyOperator(val1, val2, op, &errorFlag);
        if (!errorFlag) {
            value_stack[++numTop] = result;
        }
    }
    if (errorFlag  == 1 || numTop != 0 || opTop != -1) {
        printf("Error: Invalid expression.\n");
    } else if (errorFlag  == 2) {
        printf("Error: Division by zero.\n");
    } else {
        printf("%d\n", value_stack[0]);
    }
}

/*
    The main function reads an arithmatic expression from the input by user.
    Also it removes newline and calls calculate() to evaluate it.
*/
int main() {
    char userInput[1024];
    if (fgets(userInput, sizeof(userInput), stdin)) {
        int len = strlen(userInput);
        if (len > 0 && userInput[len - 1] == '\n') {
            userInput[len - 1] = '\0';
        }
        calculate(userInput);
    }
    return 0;
}