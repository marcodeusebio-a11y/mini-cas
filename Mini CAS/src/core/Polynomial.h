//
//  Polynomial.h
//  Mini Symbolic CAS for Rational Functions
//
//  Created by Marco D’Eusebio on 5/14/26.
//

#pragma once

#include "Rational.h"
#include <iosfwd>
#include <string>
#include <vector>

namespace cas {

class Polynomial {
public:
    Polynomial();
    explicit Polynomial(std::vector<Rational> coefficientsAscending);
    
    static Polynomial constant(const Rational& value);
    static Polynomial variable();
    
    const std::vector<Rational>& coefficients() const;
    int degree() const;
    bool isZero() const;
    const Rational& leadingCoefficient() const;
    Rational coefficient(int power) const;
    
    Rational evaluate(const Rational& x) const;
    std::string toString() const;
    
    Polynomial derivative() const;
    
    static Polynomial add(const Polynomial& left, const Polynomial& right);
    static Polynomial subtract(const Polynomial& left, const Polynomial& right);
    static Polynomial multiply(const Polynomial& left, const Polynomial& right);
    static Polynomial scale(const Polynomial& poly, const Rational& factor);
    static Polynomial power(Polynomial base, int exponent);
    
private:
    std::vector<Rational> coefficients_;
    
    void normalize();
};

struct PolynomialDivisionResult {
    Polynomial quotient;
    Polynomial remainder;
};

PolynomialDivisionResult divide(const Polynomial& numerator, const Polynomial& denominator);

std::ostream& operator<<(std::ostream& out, const Polynomial& polynomial);

}  // namespace cas
