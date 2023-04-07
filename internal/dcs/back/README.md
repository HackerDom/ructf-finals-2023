# DCS backend

## About
DCS (Demidovich Computer Systems) is a compiled programming language for arithmetical expressions computations.

In addition to standard expressions, DCS supports variables, functions and conditional statements.
Program for DCS looks like:
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

## How to build
`cmake` required. See [this page](https://cmake.org/download/) for more.
Only Linux is supported.
```bash
git submodule update --init --recursive
mkdir -p build && cd build
cmake -DCMAKE_BUILD_TYPE=Release .. && make -j # or Debug
```

## Run tests
### language tests
```bash
cd build/lang
./dcslang_tests
./dcslang_tests '--gtest_filter=Compiler.*'
./dcslang_tests '--gtest_filter=Functional.*'
./dcslang_tests '--gtest_filter=Functional.Factorial'
```
Vulnerability tests
- select test from [functional_tests.cpp](lang/tests/functional_tests.cpp) with `Vuln` suite, remove prefix `DISABLED_` in name
- run with `./dcslang_tests '--gtest_filter=Vuln.<NAME>'`

### server tests
```bash
cd build/server
./dcsserver_tests
./dcsserver_tests '--gtest_filter=Storage.*'
```

## Run
Server:
```bash
cd build/server
./dcsserver -a 0.0.0.0 -p 7654
```
