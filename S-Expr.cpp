// S-Expr Copyright Robert L. Read, 2016
// Published under GPL.
// Eventually this may turn into an Arduino library that I publish. Right now it is a draft.

/*

Our basic goal is to write in a tiny Arduino-compatible style a function like:

parse("(m 4 5 6)") that will return
a cons-like tree representing the value: 

(cons m (cons 4 (cons 5 (cons 6 nil)))) 

(although of course that will be represent as a structure in C.

Our main goal her is to read such expressions on the serial port and drive functions
in the C object space.  So this expressions could more or less easily turn into a
call (although dyanamic) like:
m(4,5,6),

where m is a function defined in the C space.

So we want a function "invoke", and if you called 

invoke("(m 4 5 6)") 
it would call m(4,5,6).

So to do a good job with this what would Kent Beck do?

Write the test first!  Although testing on a Arduino is a painful.

Note: At first I will support only strings and integers --- not "symbols".

Our basic alphabet of functions and types:

strings 
int
nil
atom: string | int | nil
sexpr : atom | cons sexpr sexpr

functions:

Cons
car 
cdr
nil
null
list
nth 
delete
lex

But do I really have a need for anyting that can't be handled as a one-level list with the lexer?

I think I probably don't need to continue work on this file until my use-cases sharpen.
 */

#include "limits.h"
#include "S-Expr.h"

// Can this be an enum?
const int NIL_T = 0;
const int CONS_T = 1;
const int INT_T = 2;
const int STRING_T = 3;

    
sexpr NIL = { .car = NULL, .cdr = NULL, .tp = NIL_T  };

// This is a singleton
sexpr* cons_nil() {
  return &NIL;
}

sexpr* cons_int(int n) {
  sexpr* s = new sexpr();
  s->tp = INT_T;
  // This assumes that an int can fit into a pointer... need to check
  s->car = (sexpr*) n;
  return s;
}

sexpr* cons_string(TOKEN tk) {
  sexpr* s = new sexpr;
  s->tp = STRING_T;
  s->car = (sexpr*) tk.buff;
  // Are we guaranteed that sizeof(sexpr*) >= sizeof(int)?
  if (sizeof(sexpr*) < sizeof(int)) {
    Serial.println("Complete catastrophe, Rob really clustered this one.");
  }
  s->cdr = (sexpr*) tk.first;
  s->extra = (int) tk.terminator;
  return s;
}

String value_s(sexpr* s) {
  if (s->tp == STRING_T) {
    return String((char *) (s->car));
  } else {
    return String();
  }
}

int value_i(sexpr* s) {
  if (s->tp == INT_T) {
    return ((int) (s->car));
  } else {
    Serial.println("Value taken of non-int!");
    Serial.println(print_as_String(s));
    return INT_MIN;
  }
}

sexpr* cons(sexpr* car, sexpr* cdr) {
  sexpr* s = new sexpr();
  s->tp = CONS_T;
  s->car = car;
  s->cdr = cdr;
  return s;
}

boolean null(sexpr *s) {
  return s->tp == NIL_T; 
}

void del(sexpr* s) {
  if (s) {
    switch (s->tp) {
    case NIL_T:
      // because this is a constant, we don't delete it.
      break;
    case CONS_T:
      del(s->car);
      del(s->cdr);
      delete s;
      break;
    case INT_T:
      delete s;
      break;
    case STRING_T:
      delete s;
      break;
    default:
      Serial.println("Del Catastrophe!");
      Serial.println(s->tp);
      break;
    }

  }
}

// A little tricky here, we need to produce a list of tokens.
// Everytime we encounter (,), or whitespace, we are generating a new token.
// return the next token that starts at n.
// String getToken(String str,int n) {
//   int len = str.length() - n;
//   int i = 0;
//   while(i < len) {
//     char d = str.charAt(n+i);
//     if (d == '(') {
//       return str.substring(n,(i == 0) ? n+1 : n+i);
//     } else if (d == ')') {
//       return str.substring(n,(i == 0) ? n+1 : n+i);
//     } else if (d == ' ') {
//       return str.substring(n,(i == 0) ? n+1 : n+i);
//     } else {
//       i++;
//     }
//   }
//   return str.substring(n,n+i);
// }

TOKEN getToken(char *str,int n) {
  int len = strlen(str);
  int i = 0;
  while(i < len) {
    char d = str[i+n];
    if ((d == '(') || (d == ')') || (d == ' ')) {
      TOKEN tk = { .buff = str, .first = n, .terminator = (i == 0) ? n+1 : n+i };
      return tk;
    } else {
      i++;
    }
  }
  TOKEN tk = { .buff = str, .first = n, .terminator = n+i };
  return tk;
}

void printTokenForDebugging(TOKEN tk) {
  Serial.println("tk.buff");
  Serial.println(tk.buff);
  Serial.println("first");
  Serial.println(tk.first);
  Serial.println("terminator");
  Serial.println(tk.terminator);
}

boolean StringIsNatural(String str) {
  int len = str.length();
  for(int i = 0; i < len; i++) {
    char c = str.charAt(i);
    if (!isDigit(c))
      return false;
  }
    return true;
}

boolean TokenIsNatural(TOKEN tk) {
  int len = tk.terminator - tk.first;
  for(int i = 0; i < len; i++) {
    char c = tk.buff[i+tk.first];
    if (!isDigit(c))
      return false;
  }
  return true;
}

int TokenToInt(TOKEN tk) {
  String s = "";
  int len = tk.terminator - tk.first;
  char str[len+1];
  for(int i = 0; i < len; i++) {
    char c = tk.buff[i+tk.first];
    str[i] = c;
    if (!isDigit(c)) {
      Serial.println("Catastrophic call of TokenToInt on non-int!");
      return INT_MIN;
    }
  }
  str[len] = '\0';
  return atoi(str);
  
}
int TokenLength(TOKEN tk) { return tk.terminator - tk.first; }

parse_result parse_symbol(char *str,int n) {
    TOKEN token = getToken(str,n);
    parse_result pr;
    sexpr* s;
    
    if (TokenIsNatural(token)) {
      s = cons_int(TokenToInt(token));      
    } else {
      s = cons_string(token);
    }

    pr.s = s;
    pr.used_to = n+TokenLength(token);
    return pr;
}

boolean TokenEquals_string(TOKEN tk, char const *str) {
  char *tkstr = tk.buff+tk.first;
  int i = 0;
  int len = tk.terminator - tk.first;
  while((i < len) && (*str != 0)) {
    if (*tkstr != *str) return false;
    i++;
    tkstr++;
    str++;
  }
  return (i == len) && (*str == '\0');
}
parse_result parse_list(char * str,int n) {
  int k = n;
  TOKEN token = getToken(str,k);

  // This kind of sucks...
  parse_result all[100];
  int cur = 0;

  while (!TokenEquals_string(token,")") && !TokenEquals_string(token,"")) {
    // if whitespace, we do nothing..
    if (TokenEquals_string(token," ")) {
      k = k + 1;
      token = getToken(str,k);
    } else if (TokenEquals_string(token,"(")) {
	parse_result r = parse_list(str,k+1);
	all[cur++] = r;
	k = r.used_to;
	token = getToken(str,k);
    } else {
      parse_result r = parse_symbol(str,k);
      k = r.used_to;
      token = getToken(str,k);
      
      all[cur++] = r;
    }
  }
  if (TokenEquals_string(token,"")) {
    Serial.println("UNABLE TO PARSE, RETURNING NL");
    printTokenForDebugging(token);
    parse_result pr;
    pr.s = (sexpr*) null;
    pr.used_to = 0;
    return pr;
  }
  // now cons up these results in reverse order!
  parse_result pr;
  pr.used_to = k+1;
  sexpr* s = &NIL;
  for(int i = (cur-1); i >= 0; i--) {
    sexpr* c = all[i].s;
    s = cons(c,s);
  }
  pr.s = s;
  return pr;
}


// We'll worry about dotted expressions later!
parse_result parse(char *str, int n) {
    TOKEN token = getToken(str,n);
  // if we start an S-Expression, we call parse-S-Exprsion.
  if (TokenEquals_string(token,"(")) {
    return parse_list(str,n+1);
  } else if (TokenEquals_string(token,")")) { // we can return NIL
    parse_result pr;
    pr.s = &NIL;
    pr.used_to = 1+n;
    return pr;
  } else if (TokenEquals_string(token," ")) { //
    return parse(str,n+1);
  } else {
    // now it must be content, so decide if it an integer
    return parse_symbol(str,n);
  }
}

// TODO: We have no intention of changing this, so should it be
// char const *?  
sexpr* parse(char  *str) {
  return parse(str,0).s;
}


// We need to be very careful here about memory allocation for this.
sexpr* list(int n,sexpr* exps) {
  return (sexpr*) null;
}

sexpr* nth(sexpr* s,int n) {
  int i = 0;
  sexpr* cur = s;
  while ((i < n) && (cur != (sexpr*) null)){
    if (cur->tp != CONS_T) {
      return (sexpr*) &NIL;
    } else {
      cur = cur->cdr;
      i++;
    }
  }
  if (cur == (sexpr*) null) {
      return (sexpr*) &NIL;
  }
  return cur->car;
}

int s_length(sexpr* s) {
  int i = 0;
  sexpr* cur = s;
  while (cur != (sexpr*) null) {
    if (cur->tp != CONS_T) {
      return i;
    } else {
      cur = cur->cdr;
      i++;
    }
  }
  return i;
}

void print_sexpr(sexpr* s) {
  String str = print_as_String(s);
  Serial.println(str);
}

String print_as_String(sexpr* s,boolean inlist) {
  String str = "";
  switch (s->tp) {
  case NIL_T:
    return String("nil");
    break;
  case CONS_T:
    if (s->car->tp == CONS_T) {
      if (!inlist)
	str = str + "(";
      str = str + print_as_String(s->car,false);
    } else {
      if (!inlist)
	str = str + "(";
      str = str + print_as_String(s->car,true);
    }
    if (s->cdr->tp != NIL_T) {
      str = str + " ";
      str = str + print_as_String(s->cdr,true);
    } else {
      str = str + ")";
    }
    return str;
    break;
  case INT_T:
    return String((int) (s->car));
    break;
  case STRING_T:
    {
      char *strx = (char *) s->car;
      int i = 0;
      int lena = (int) s->extra - (int) s->cdr;
      strx += (int) s->cdr;
      while((i < lena)) {
	str = str + *strx;
	i++;
	strx++;
      }
     }
    break;
  default:
    Serial.println("print Catastrophe!");
    return String("print Catastrophe!");
    break;
  }
  return str;
}

String print_as_String(sexpr* s) {
    return print_as_String(s,false);
}

void canon_print_sexpr(sexpr* s) {
    switch (s->tp) {
    case NIL_T:
      Serial.print("nil");
      break;
    case CONS_T:
      Serial.print("(");
      print_sexpr(s->car);
      Serial.print(" .  ");
      print_sexpr(s->cdr);
      Serial.print(")");
      break;
    case INT_T:
      Serial.print((int) (s->car));
      break;
    case STRING_T:
    {
      char *strx = (char *) s->car;
      int i = 0;
      int lena = (int) s->extra - (int) s->cdr;
      strx += (int) s->cdr;
      while((i < lena)) {
	Serial.print(*strx);
	i++;
	strx++;
      }
     }
      break;
    default:
      Serial.println("Catastrophe!");
      break;
    }
}

// This is a essentially a deep equal on value.
boolean equal(sexpr* a, sexpr* b) {
  if (a->tp != b->tp) {
    return false;
  } else {
    switch (a->tp) {
    case NIL_T:
      return true;
    case CONS_T:
      return equal(a->car,b->car) && equal(a->cdr,b->cdr);
    case INT_T:
      return (int) a->car == (int) b->car;
    case STRING_T:
      // TODO: This is wrong, we are storing token stuff now!
      {
      char *stra = (char *) a->car;
      char *strb = (char *) b->car;
      int i = 0;
      int lena = (int) a->extra - (int) a->cdr;
      int lenb = (int) b->extra - (int) b->cdr;
      stra += (int) a->cdr;
      strb += (int) b->cdr;
      while((i < lena) && (i < lenb)) {
	if (*stra != *strb) {
	  return false;
	}
	i++;
	stra++;
	strb++;
      }
      return (i == lena) && (i == lenb);
      }
    default:
      Serial.println("Catastrophein Equal!");
    }
    return false;
  }
}


