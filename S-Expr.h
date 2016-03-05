#ifndef S_Expr_h
#define S_Expr_h

#include "Arduino.h"

// S-Expr Copyright Robert L. Read, 2016
// Published under GPL.
// Eventually this may turn into an Arduino library that I publish. Right now it is a draft.
extern const int NIL_T;
extern const int CONS_T;
extern const int INT_T;
extern const int STRING_T;


typedef struct sexpr {
  int tp;
  sexpr* car;
  sexpr* cdr;
} Sexpr;

typedef struct parse_result {
  int used_to;
  sexpr* s;
} PR;

// sexpr NIL;

sexpr* cons_nil();

sexpr* cons_int(int n);

sexpr* cons_string(String str);
sexpr* cons(sexpr* car, sexpr* cdr);

boolean null(sexpr *s);
void del(sexpr* car);


sexpr* parse(String str);

// We need to be very careful here about memory allocation for this.
sexpr* list(int n,sexpr* exps);

sexpr* nth(sexpr* expr,int n);

int s_length(sexpr* s);

void print_sexpr(sexpr* s,boolean inlist);

void print_sexpr(sexpr* s);

String print_as_String(sexpr* s);

void canon_print_sexpr(sexpr* s);

// This is a essentially a deep equal on value.
boolean equal(sexpr* a, sexpr* b);
 
String value_s(sexpr* s);

int value_i(sexpr* s);

// These functions are public exposed, mainly so that they will be available to the tests!

String getToken(String str,int n);

#endif
