# DCS backend

## About
DCS (Demidovich Computer Systems) is a programming language for computing of arithmetical expressions like `4 + 6 / 7`.

In addition to standard expressions, DCS supports variables, functions and conditional branches.
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

App contains three binaries:
- `dcs` - main TCP service for compile\store\run DCS programs, issued to competition participants
- `dcs_local` - local version for executing single DCS programs, used in checker
- `dcs_tests` - tests of dcs functionality, dev only

## How to build
`cmake` required. See [this page](https://cmake.org/download/) for more.
Only Linux is supported.
```bash
mkdir -p build && cd build
cmake -DCMAKE_BUILD_TYPE=Release .. && make -j # or Debug
```

## Run tests
All tests:
```
cd build
./dcs_tests
```
Specified tests ([full list](src/tests)):
```bash
cd build
./dcs_tests '--gtest_filter=Functional.*'
./dcs_tests '--gtest_filter=Functional.Factorial'
```
Vulnerability tests
- select test from [functional_tests.cpp](src/tests/functional_tests.cpp) with `Vuln` suite, remove prefix `DISABLED_` in name
- run as specified tests

## Run
```bash
cd build
./dcs_local <expression>
```

```bash
cd build
./dcs -a 0.0.0.0 -p 7654
```