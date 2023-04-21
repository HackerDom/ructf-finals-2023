# DCS

## About

DCS (Demidovich Computer Systems) is a compiled programming language for arithmetical expressions computations.

In addition to standard expressions, DCS supports variables, functions and conditional statements. Program for DCS looks pretty simple:

```
fun fib(a, b, n) {
    c = a + b;
    a = b;
    b = c;
    if (n <= 0) {
        return b;
    } else {
        return fib(a, b, n - 1);
    }
}

fun main() {
    return fib(0, 1, 5);
}
```

## Features
There are several useful features in the language:
- functions
- local variables
- global constants
- arigthmetical expressions
- conditional statements

*DCS* uses double type for all values in programs (constants, immediates, variables, function arguments, function return values).

### Functions
Function can be defined by keyword **fun**. Functions can have up to 3 arguments, defined in function definition.
Every function must return some value.
Examples of function definitions:
```
fun f() {
    return 3.14;
}

fun g(x) {
    return x + 1;
}

fun h(x, y) {
    return x + y;
}

fun j(x, y, z) {
    return x + y + z;
}

// will get compilation error
fun l(x, y, z, u) {
}
```

There is one special function that must be defined in program - **main**. It cant have any arguments, and its return value are result of program evaluation.

```
fun main() {
    return 3.14;
}
```

### Local variables
Every function can have local variables, useful for computation. Local variables are accessible only in function that defines it. Note, that function arguments are local variables too.
You dont need to declare local variables, only assignment needed. *Please note: variable usage before its assignment is **undefined behaviour**.*
Ex:
```
fun f(x) {
    x = x + 1;
    return x;
}

fun main() {
    x = f(4);
    return x;
}
```

The return value of main is undefined:
```
fun main() {
    y = x + 1;
    x = 2;
    return y;
}
```

### Global constants
You can define global values, which are accessible from every function, and cant be changed during program execution. Constant definition must be ended with semicolon;
```
pi = 3.14;
e = 2.7;

fun min() {
    if (pi > e) {
        return e;
    }

    return pi;
}

fun main() {
    return min();
}
```

### Arithmetical expressions
Expr is:
- -Expr
- (Expr)
- Expr + Expr
- Expr - Expr
- Expr * Expr
- Exrp / Expr

Priorities of operations are the same as in any other popular languages. Ex:

```
fun main() {
    x = 1;
    y = 2;
    z = x + y;
    t = (-x + y - 5.5) * z / (y * x);

    return t;
}
```

### Conditional statements
You can branch your execution flow with `if () {} else {}` statements. Ex:
```
fun abs(x) {
    if (x < 0) {
        return -x;
    } else {
        return x;
    }
}
``` 

This can be simplified by removing `else`:
```
fun abs(x) {
    if (x < 0) {
        return -x;
    }

    return x;
}
```


## Undefined behaviours

### Usage before assignment
As written before, usage of local variable before its assignment is undefined bevaviour and will lead to garbage value in expression.

### Infinite recursion
Result of this code is indefined (possible stackoverflow when executed):
```
fun f(x) {
    return x + f(x);
}

fun main() {
    return f();
}
```