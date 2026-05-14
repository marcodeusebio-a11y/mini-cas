# Mini CAS

- integers like `5`
- rationals like `2/3`
- variables like `x`
- `+`, `-`, `*`, `/`, `^`
- parentheses
- implicit multiplication such as:
  - `2x`
  - `x(x+1)`

### Exact partial fractions

The current partial-fractions layer supports:
- proper and improper rational functions
- exact polynomial long division
- repeated linear factors over `Q`
- exact quadratic terms when the denominator is entered in visible factored form, such as:
  - `(x^2+1)(x-2)`
  - `(x^2+1)^2(x-1)`

## Example inputs

```text
(x+5)/((x-3)(x+2))
(2x+1)/((x-1)^2(x+2))
(x+1)/((x^2+1)(x-2))
(2x^2+3x+4)/((x^2+1)^2(x-1))
Example output shape
Parsed numerator:
  x + 5
Parsed denominator:
  x^2 - x - 6

Denominator structure:
  (x + 2) * (x - 3)

Polynomial long division:
  quotient  = 0
  remainder = x + 5

Exact partial fraction decomposition:
  -3/5/(x + 2) + 8/5/(x - 3)

Exact reconstruction check:
  passed
```
## Project layout
```
Mini CAS/
├── src/
│   ├── app/
│   │   └── main.cpp
│   ├── core/
│   │   ├── Rational.h
│   │   ├── Rational.cpp
│   │   ├── Polynomial.h
│   │   └── Polynomial.cpp
│   ├── expr/
│   │   ├── Expr.h
│   │   ├── Expr.cpp
│   │   ├── Simplify.h
│   │   └── Simplify.cpp
│   ├── parsing/
│   │   ├── Parser.h
│   │   └── Parser.cpp
│   └── algebra/
│       ├── Factor.h
│       ├── Factor.cpp
│       ├── PartialFractions.h
│       └── PartialFractions.cpp
└── tests/
    └── ParserTests.cpp
```
## Build

**CLI build**

g++ -std=c++17 -O2 -Wall -Wextra -pedantic \
  main.cpp Rational.cpp Polynomial.cpp Expr.cpp Simplify.cpp Parser.cpp Factor.cpp PartialFractions.cpp \
  -o minicAS
./minicAS

**CLI test build**
g++ -std=c++17 -O2 -Wall -Wextra -pedantic \
  tests/ParserTests.cpp Rational.cpp Polynomial.cpp Expr.cpp Simplify.cpp Parser.cpp Factor.cpp PartialFractions.cpp \
  -o minicAS_tests
./minicAS_tests

**Xcode build**

Add these .cpp files to Compile Sources:

- `main.cpp`
- `Rational.cpp`
- `Polynomial.cpp`
- `Expr.cpp`
- `Simplify.cpp`
- `Parser.cpp`
- `Factor.cpp`
- `PartialFractions.cpp`

**Xcode test build**
Add tests/ParserTests.cpp either:
- to a separate test target
            or
- temporarily to the main target while debugging

## Current limitations

This is still a mini-CAS, not a full computer algebra system.

Current limitations include:

- no full symbolic simplifier/canonicalizer yet
- no transcendental functions (sin, cos, log, exp, etc.)
- no symbolic integration
- no general exact factorization for arbitrary higher-degree polynomials over all algebraic extensions
- quadratic partial fractions work best when the denominator is entered in visible factored form
- no assumptions system
- no matrix/tensor algebra
- no Gröbner basis or multivariate algebra engine
- Roadmap

## Planned next steps:

- stronger canonical simplification
- richer parser features
- exact irreducible quadratic handling from expanded forms
- symbolic differentiation on the full expression tree
- broader exact factorization support
- full rational-function normalization
- larger test suite
  
## Why this project exists

This project is meant to teach and demonstrate how symbolic algebra systems are built from the ground up:

- exact arithmetic first
- exact polynomial algebra second
- parsing and simplification third
- factorization and decomposition on top

It is designed as a learning project and a foundation for a more capable symbolic engine later.

## Screenshot

<img width="865" height="825" alt="Screenshot 2026-05-14 at 05 17 07" src="https://github.com/user-attachments/assets/8864d020-9f95-4bfd-84aa-38cdeda9354e" />
