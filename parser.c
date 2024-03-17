/* CSC173 Project2
 * Fall 2023
 *
 * Derived from original file focs_11_27_gf.c
 * 
 * Produces a parse tree of the input expression (if syntactically correct) and prints it,
 * using recursive descent parsing and table-driven parsing.
 * 
 * The recursive descent parser uses lookahead(), match() and parse functions to analyze
 * the input string.
 * 
 * The table-driven parser uses a stack structure and a parsing table (int[][]) to analyze
 *  the input string.
 */
#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#define FAILED NULL

char *STRDUP(const char *s);
//Parsing tree, nodes, and functions of constructing nodes
typedef struct NODE *TREE;
struct NODE {
    char label;
    TREE lmc, rs, par;
};
TREE makeNode0(char x);
TREE makeNode1(char x, TREE t);
TREE makeNode2(char x, TREE t1, TREE t2);
TREE makeNode3(char x, TREE t1, TREE t2, TREE t3);
TREE makeNode4(char x, TREE t1, TREE t2, TREE t3, TREE t4);
void print_tree(int level, TREE t);
//Functions for Recursive desendent parsing
TREE RDparse(char* input);
TREE Expr();
TREE Atom();
TREE Word();
TREE WordT();
TREE Letter();
TREE Number();
TREE NumberT();
TREE Digit();
TREE List();
TREE Elements();
TREE Rest();

//Functions for Stack definition
typedef struct STACKNODE *STACKNODE;
struct STACKNODE{
    char value;
    STACKNODE next_node;
};
typedef struct STACK *STACK;
struct STACK{
    STACKNODE head_node;
};

//Functions for Table definition
typedef struct TABLE *TABLE;
struct TABLE
{
    int table[14][6];
};

//Functions for Table operation
TABLE newTable();
TABLE lisp_table();
int table_get_value(TABLE this,int row, int column);
int translate_row(char c);
int translate_col(char c);
//Functions for Stack operation
STACK newStack();
void push(STACK stack,char c);
char pop(STACK stack);
STACKNODE node_new(char c);
bool TD_lookahead(STACK stack);
void func_operate(STACK stack,int func_choice);

//Functions for Table driven-parsing
void move_pointer();
STACK TDparse(TABLE table,char* input);


TREE parseTree; /* holds the result of the parse */
char *nextTerminal; /* current position in input string */
char*pointer;
TREE parse_tree2;
TREE tree2ptr;

/**
 * Returns true if the current input symbol is the given char, otherwise false.
 */
bool lookahead(char c) {
    return *nextTerminal == c;
}

/**
 * Returns true if the next input symbol is the given char, otherwise false.
 */
bool lookbeyond(char c) {
    char *temp = STRDUP(nextTerminal);
    temp+=1;
    return *temp == c;
}

/**
 * If the current input symbol is the given char, advance to the next
 * character of the input and return true, otherwise leave it and return false.
 */
bool match(char c) {
    if (lookahead(c)) {
	nextTerminal += 1;
	return true;
    } else {
	return false;
    }
}

int main(){
    char input[256]; 
    while(true){
        printf("Please enter the string you want to parse or \"quit\" to exit.\n");
        fgets(input, sizeof(input),stdin);
        size_t len = strlen(input);
        if (len > 0 && input[len - 1] == '\n') {
            input[len - 1] = '\0';
        }else{
            printf("Fault\n");
        }
        if(!strcmp("quit",input)) break;
        nextTerminal = input;
        pointer = input;
        TABLE lisp = lisp_table();

        parseTree = RDparse(input);

        if(parseTree==NULL){
            printf("Recursive-Desendent parsing fail.\n");
        }else{
            printf("Recursive-Desendent parsing success.\n");
            print_tree(0,parseTree);
        }
        //getchar();
        STACK stack = TDparse(lisp,input);

        
        if(stack->head_node==NULL&&*pointer=='\0'){
            printf("Table-Driven parsing success.\n");
            print_tree(0,parse_tree2);
        }else{
            printf("Table-Driven parsing fail.\n");
        }
    }
}

/**
 * Attempt to print the parsing tree.
 */
void print_tree(int level, TREE t)
{
    for (int i = 0; i < level; i++)
    {
        printf(" ");
    }
    printf("%c\n", t->label);
    
    if (t->lmc != NULL)
        print_tree(level + 1, t->lmc);
    if (t->rs != NULL)
        print_tree(level, t->rs);
}

/**
 * Make a single node with no children.
 */
TREE makeNode0(char x){
    TREE root;
    root = (TREE) malloc(sizeof(struct NODE));
    root->label = x;
    root->lmc = NULL;
    root->rs = NULL;
    root->par = NULL;
    return root;
}

/**
 * Make a single node with 1 child.
 */
TREE makeNode1(char x, TREE t){
    TREE root;
    root = makeNode0(x);
    root->lmc = t;
    return root;
}

/**
 * Make a single node with 2 children.
 */
TREE makeNode2(char x, TREE t1, TREE t2){
    TREE root;
    root = makeNode1(x, t1);
    t1->rs = t2;
    return root;
}

/**
 * Make a single node with 3 children.
 */
TREE makeNode3(char x, TREE t1, TREE t2, TREE t3){
    TREE root;
    root = makeNode1(x, t1);
    t1->rs = t2;
    t2->rs = t3;
    return root;
}

/**
 * Make a single node with 4 children.
 */
TREE makeNode4(char x, TREE t1, TREE t2, TREE t3, TREE t4){
    TREE root;
    root = makeNode1(x, t1);
    t1->rs = t2;
    t2->rs = t3;
    t3->rs = t4;
    return root;
}

//--------------Recursive Desendent Parsing-------------------------

// Grammar:
// Expr → Atom | List
// Atom → Word | Number
// Word → Letter WordT
// WordT → Word | ε
// Letter → a | · · · | z
// Number → Digit NumberT
// NumberT → Number | ε
// Digit → 0 | · · · | 9
// List → (Elements)
// Elements → ε | Expr Rest
// Rest → ε | Space Expr Rest

/**
 * Use Recursive-Desendent parsing to analyze input.
 * If the parse succeeds, return the parse tree, else return FAILED (NULL).
 */
TREE RDparse(char* input) {
    nextTerminal = STRDUP(input);
    TREE parseTree = Expr();
    if (parseTree != NULL && lookahead('\0')) {
	return parseTree;
    } else return FAILED;
}

// Expr → List | Atom
TREE Expr(){ 
    if(lookahead('(')){
        TREE list = List();
        if(list==NULL) return FAILED;
        return makeNode1('E',list);
    }else{
        TREE atom = Atom();
        if(atom==NULL) return FAILED;
        return makeNode1('E',atom);
    }
}

// Atom → Word | Number
TREE Atom(){
    TREE word = Word();
    if(word==NULL){
        TREE number = Number();
        if(number==NULL) return FAILED;
        return makeNode1('A',number);
    }else return makeNode1('A',word);
}

// Word → Letter WordT
TREE Word(){
    TREE letter = Letter();
    if(letter==NULL) return FAILED;
    TREE wordT = WordT();
    if(wordT==NULL) return FAILED;
    return makeNode2('W',letter,wordT);
}

// WordT → Word | ε
TREE WordT(){
    TREE word = Word();
    if(word==NULL) return makeNode1('w',makeNode0('&'));
    return makeNode1('w',word);
}

// Letter → a | ··· | z
TREE Letter(){
    for(int i = 97; i<=122;i++){
        if(lookahead((char)i)){
            TREE letter = makeNode0((char)i);
            nextTerminal+=1;
            return makeNode1('l',letter);
        }
    }
    return FAILED;
}

// Number → Digit NumberT
TREE Number(){
    TREE digit = Digit();
    if(digit==NULL) return FAILED;
    TREE numberT = NumberT();
    if(numberT==NULL) return FAILED;
    return makeNode2('N',digit,numberT);
}

// NumberT → Number | ε
TREE NumberT(){
    TREE number = Number();
    if(number==NULL) return makeNode1('n',makeNode0('&'));
    return makeNode1('n',number);
}

// Digit → 0 | ··· | 9
TREE Digit(){
    for(int i = 48; i<=57;i++){
        if(lookahead((char)i)){
            TREE digit = makeNode0((char)i);
            nextTerminal+=1;
            return makeNode1('d',digit);
        }
    }
    return FAILED;
}

// List → (Elements)
TREE List(){
    if(!match('(')) return FAILED;
    TREE elements = Elements();
    if(elements==NULL) return FAILED;
    if(!match(')')) return FAILED;
    return makeNode3('L',makeNode0('('),elements,makeNode0(')'));
}

// Elements → ε | Expr Rest
TREE Elements(){
    TREE expr = Expr();
    if(expr==NULL) return makeNode1('e',makeNode0('&'));
    TREE rest = Rest();
    if(rest==NULL) return FAILED;
    return makeNode2('e',expr,rest);
}

// Rest → ε | Space Expr Rest
TREE Rest(){
    if(lookahead(' ')){
        if(!match(' ')) return FAILED;
        TREE expr = Expr();
        if(expr==NULL) return FAILED;
        TREE rest = Rest();
        if(rest==NULL) return FAILED;
        return makeNode3('R',makeNode0('s'),expr,rest);
    }else{
        return makeNode1('R',makeNode0('&'));
    }
}

//-----------Table-Driven Parsing------------------

//     Condition               Stack push(right to left)
//(1)  Expr('(')             → List
//(2)  Expr(a-z/0-9)         → Atom
//(3)  Atom(a-z)             → Word
//(4)  Atom(0-9)             → Number
//(5)  Word(a-z)             → Letter WordT
//(6)  WordT(a-z)            → Word
//(7)  WordT(')'/' '/'\0')   → ε
//(8)  Letter(a-z)           → l
//(9)  Number(0-9)           → Digit NumberT
//(10) NumberT(0-9)          → Number
//(11) NumberT(')'/' '/'\0') → ε
//(12) Digit(0-9)            → d
//(13) List('(')             → (Elements)
//(14) Elements(a-z/0-9/'(') → Expr Rest
//(15) Elements(')')         → ε
//(16) Rest(' ')             → Space Expr Rest
//(17) Rest(')')             → ε
//(18) '('                   → '('process
//(19) ')'                   → ')'process
//(20) ' '                   → ' 'process

//         0     1     2     3     4     5
//  |     | a-z | 0-9 | (   | )   | ' ' | \0  |
// 0|  E  | 2   | 2   | 1   |     |     |     |
// 1|  A  | 3   | 4   |     |     |     |     |
// 2|  W  | 5   |     |     |     |     |     |
// 3|  w  | 6   |     |     | 7   | 7   | 7   |
// 4|  l  | 8   |     |     |     |     |     |
// 5|  N  |     | 9   |     |     |     |     |
// 6|  n  |     | 10  |     | 11  | 11  | 11  |
// 7|  d  |     | 12  |     |     |     |     |
// 8|  L  |     |     | 13  |     |     |     |
// 9|  e  | 14  | 14  | 14  |     |     |     |
//10|  R  |     |     |     | 17  | 16  |     |
//11|  (  |     |     | 18  |     |     |     |
//12|  )  |     |     |     | 19  |     |     |
//13|  s  |     |     |     |     | 20  |     |

STACK TDparse(TABLE table,char* input){
    parse_tree2=makeNode0('E');
    tree2ptr=parse_tree2;
    STACK stack=newStack();
    while (stack->head_node!=NULL&&*pointer!='\0'){
        char temp_char=pop(stack);
        int row=translate_row(temp_char);
        if (row==-1){
            return stack;
        }
        int col=translate_col(*pointer);
        if (col==-1){
            return stack;
        }
        int func_choice=table_get_value(table,row,col);
        if (func_choice==-1){
            return stack;
        }
        func_operate(stack,func_choice);
        move_pointer();
        if (*pointer=='\0'&&stack->head_node!=NULL){
            if (stack->head_node->value=='n'){
                func_operate(stack,11);
                pop(stack);
            }else if (stack->head_node->value=='w'){
                func_operate(stack,7);
                pop(stack);
            }
        }
    }
    return stack;
}

void move_pointer(){
    if (tree2ptr->lmc!=NULL&&tree2ptr->lmc->label!='&'){
        tree2ptr=tree2ptr->lmc;
    }else{
        while (tree2ptr->rs==NULL){
            if(tree2ptr->par==NULL) return;
            tree2ptr=tree2ptr->par;
        }
        tree2ptr=tree2ptr->rs;
    }
}
TABLE newTable(){
    TABLE table=(TABLE)malloc(sizeof(struct TABLE));
    for (int i = 0; i < 14; i++){
        for (int j = 0; j < 6; j++){
            table->table[i][j]=-1;
        }
    }
    return table;
}
TABLE lisp_table(){
    TABLE table=newTable();
    table -> table[0][0] = 2;
    table -> table[0][1] = 2;
    table -> table[0][2] = 1;
    table -> table[1][0] = 3;
    table -> table[1][1] = 4;
    table -> table[2][0] = 5;
    table -> table[3][0] = 6;
    table -> table[3][3] = 7;
    table -> table[3][4] = 7;
    table -> table[3][5] = 7;
    table -> table[4][0] = 8;
    table -> table[5][1] = 9;
    table -> table[6][1] = 10;
    table -> table[6][3] = 11;
    table -> table[6][4] = 11;
    table -> table[6][5] = 11;
    table -> table[7][1] = 12;
    table -> table[8][2] = 13;
    table -> table[9][0] = 14;
    table -> table[9][1] = 14;
    table -> table[9][2] = 14;
    table -> table[9][3] = 15;
    table -> table[10][3] = 17;
    table -> table[10][4] = 16;
    table -> table[11][2] = 18;
    table -> table[12][3] = 19;
    table -> table[13][4] = 20;
    return table;
}

int translate_row(char c){
    if(c=='E') return 0;
    if(c=='A') return 1;
    if(c=='W') return 2;
    if(c=='w') return 3;
    if(c=='l') return 4;
    if(c=='N') return 5;
    if(c=='n') return 6;
    if(c=='d') return 7;
    if(c=='L') return 8;
    if(c=='e') return 9;
    if(c=='R') return 10;
    if(c=='(') return 11;
    if(c==')') return 12;
    if(c=='s') return 13;
    return -1;
}
int translate_col(char c){
    if(c=='(') return 2;
    if(c==')') return 3;
    if(c==' ') return 4;
    if(c=='\0') return 5;
    //Letter
    for(int i = 97; i<=122; i++){
        if(c==(char)i) return 0;
    }
    //Digit
    for(int i = 48; i<=57; i++){
        if(c==(char)i) return 1;
    }
    return -1;
}

void func_operate(STACK stack,int func_choice){
    switch (func_choice){
    case 1:
        push(stack,'L');
        tree2ptr -> lmc = makeNode0('L');
        tree2ptr -> lmc -> par = tree2ptr;
        break;
    case 2:
        push(stack,'A');
        tree2ptr -> lmc = makeNode0('A');
        tree2ptr -> lmc -> par = tree2ptr;
        break;
    case 3:
        push(stack,'W');
        tree2ptr -> lmc = makeNode0('W');
        tree2ptr -> lmc -> par = tree2ptr;
        break;
    case 4:
        push(stack,'N');
        tree2ptr -> lmc = makeNode0('N');
        tree2ptr -> lmc -> par = tree2ptr;
        break;
    case 5:
        push(stack,'w');
        push(stack,'l');
        tree2ptr -> lmc = makeNode0('l');
        tree2ptr -> lmc -> rs = makeNode0('w');
        tree2ptr -> lmc -> par = tree2ptr;
        tree2ptr -> lmc -> rs -> par = tree2ptr;
        break;
    case 6:
        push(stack,'W');
        tree2ptr -> lmc = makeNode0('W');
        tree2ptr -> lmc -> par = tree2ptr;
        break;
    case 7:
        tree2ptr -> lmc = makeNode0('&');
        tree2ptr -> lmc -> par = tree2ptr;
        break;
    case 8:
        for(int i = 97; i<=122;i++){
            if(*pointer == ((char)i)){
                tree2ptr -> lmc = makeNode0((char)i);
                tree2ptr -> lmc -> par = tree2ptr;
                tree2ptr = tree2ptr -> lmc;
                pointer++;
                break;
            }
        }
        break;
    case 9:
        push(stack,'n');
        push(stack,'d');
        tree2ptr -> lmc = makeNode0('d');
        tree2ptr -> lmc -> rs = makeNode0('n');
        tree2ptr -> lmc -> par = tree2ptr;
        tree2ptr -> lmc -> rs -> par = tree2ptr;
        break;
    case 10:
        push(stack,'N');
        tree2ptr -> lmc = makeNode0('N');
        tree2ptr -> lmc -> par = tree2ptr;
        break;
    case 11:
        tree2ptr -> lmc = makeNode0('&');
        tree2ptr -> lmc -> par = tree2ptr;
        break;
    case 12:
        for(int i = 48; i<=57;i++){
            if(*pointer == ((char)i)){
                tree2ptr -> lmc = makeNode0((char)i);
                tree2ptr -> lmc -> par = tree2ptr;
                tree2ptr = tree2ptr -> lmc;
                pointer++;
                break;
            }
        }
        break;
    case 13:
        push(stack,')');
        push(stack,'e');
        push(stack,'(');
        tree2ptr -> lmc = makeNode0('(');
        tree2ptr -> lmc -> rs = makeNode0('e');
        tree2ptr -> lmc -> rs -> rs = makeNode0(')');
        tree2ptr -> lmc -> par = tree2ptr;
        tree2ptr -> lmc -> rs -> par = tree2ptr;
        tree2ptr -> lmc -> rs -> rs -> par = tree2ptr;
        break;
    case 14:
        push(stack,'R');
        push(stack,'E');
        tree2ptr -> lmc = makeNode0('E');
        tree2ptr -> lmc -> rs = makeNode0('R');
        tree2ptr -> lmc -> par = tree2ptr;
        tree2ptr -> lmc -> rs -> par = tree2ptr;
        break;
    case 15:
        tree2ptr -> lmc = makeNode0('&');
        tree2ptr -> lmc -> par = tree2ptr;
        break;
    case 16:
        push(stack,'R');
        push(stack,'E');
        push(stack,'s');
        tree2ptr -> lmc = makeNode0('s');
        tree2ptr -> lmc -> rs = makeNode0('E');
        tree2ptr -> lmc -> rs -> rs = makeNode0('R');
        tree2ptr -> lmc -> par = tree2ptr;
        tree2ptr -> lmc -> rs -> par = tree2ptr;
        tree2ptr -> lmc -> rs -> rs -> par = tree2ptr;
        break;
    case 17:
        tree2ptr -> lmc = makeNode0('&');
        tree2ptr -> lmc -> par = tree2ptr;
        break;
    case 18:
        pointer++;
        break;
    case 19:
        pointer++;
        break;
    case 20:
        pointer++;
        break;
    }
}

STACK newStack(){
    STACK stack=(STACK)malloc(sizeof(struct STACK));
    stack->head_node=node_new('E');
    return stack;
}
STACKNODE node_new(char c){
    STACKNODE node=(STACKNODE)malloc(sizeof(struct STACKNODE));
    node->value=c;
    node->next_node=NULL;
    return node;
}
void push(STACK stack,char c){
    STACKNODE temp_node=node_new(c);
    temp_node->next_node=stack->head_node;
    stack->head_node=temp_node;
    return;
}
char pop(STACK stack){
    STACKNODE temp_stack=stack->head_node;
    stack->head_node=temp_stack->next_node;
    return temp_stack->value;
}
int table_get_value(TABLE this,int row, int column){
    return this->table[row][column];
}
char *STRDUP(const char *s) {
    char *p = malloc(strlen(s) + 1);
    if (p) {
	strcpy(p, s);
    }
    return p;
}