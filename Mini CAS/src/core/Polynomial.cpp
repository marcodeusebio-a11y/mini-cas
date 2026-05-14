//
//  Polynomial.cpp
//  Mini Symbolic CAS for Rational Functions
//
//  Created by Marco D’Eusebio on 5/14/26.
//

#include "Polynomial.h"
#include <stdexcept>
#include <string>
#include <vector>

namespace cas {

Polynomial::Polynomial() : coefficients_{Rational(0)} {}

Polynomial::Polynomial(std::vector<Rational> coefficientsAscending)
: coefficients_(std::move(coefficientsAscending)) {
    normalize();
}

Polynomial Polynomial::constant(const Rational& value) {
    return Polynomial({value});
}

Polynomial Polynomial::variable() {
    return Polynomial({Rational(0), Rational(1)});
}

const std::vector<Rational>& Polynomial::coefficients() const {
    return coefficients_;
}

int Polynomial::degree() const {
    return static_cast<int>(coefficients_.size()) - 1;
}

bool Polynomial::isZero() const {
    for (const Rational& c : coefficients_) {
        if (!c.isZero()) {
            return false;
        }
    }
    return true;
}

const Rational& Polynomial::leadingCoefficient() const {
    return coefficients_.back();
}

Rational Polynomial::coefficient(int power) const {
    if (power < 0 || power >= static_cast<int>(coefficients_.size())) {
        return Rational(0);
    }
    return coefficients_[static_cast<std::size_t>(power)];
}

Rational Polynomial::evaluate(const Rational& x) const {
    Rational result(0);
    
    for (int i = degree(); i >= 0; --i) {
        result = result * x + coefficients_[static_cast<std::size_t>(i)];
    }
    
    return result;
}

std::string Polynomial::toString() const {
    if (isZero()) {
        return "0";
    }
    
    std::string out;
    bool first = true;
    
    for (int power = degree(); power >= 0; --power) {
        Rational coeff = coefficient(power);
        if (coeff.isZero()) {
            continue;
        }
        
        bool negative = coeff < Rational(0);
        Rational magnitude = negative ? -coeff : coeff;
        
        if (!first) {
            out += (negative ? " - " : " + ");
        } else if (negative) {
            out += "-";
        }
        
        bool printCoeff = !(magnitude == Rational(1) && power > 0);
        
        if (printCoeff) {
            out += magnitude.toString();
        }
        
        if (power > 0) {
            if (printCoeff) {
                out += "*";
            }
            out += "x";
            if (power > 1) {
                out += "^" + std::to_string(power);
            }
        }
        
        first = false;
    }
    
    return out;
}

Polynomial Polynomial::derivative() const {
    if (degree() <= 0) {
        return Polynomial::constant(Rational(0));
    }
    
    std::vector<Rational> out(static_cast<std::size_t>(degree()));
    for (int power = 1; power <= degree(); ++power) {
        out[static_cast<std::size_t>(power - 1)] =
        coefficient(power) * Rational(power);
    }
    
    return Polynomial(out);
}

Polynomial Polynomial::add(const Polynomial& left, const Polynomial& right) {
    std::size_t n = std::max(left.coefficients_.size(), right.coefficients_.size());
    std::vector<Rational> out(n, Rational(0));
    
    for (std::size_t i = 0; i < left.coefficients_.size(); ++i) {
        out[i] += left.coefficients_[i];
    }
    for (std::size_t i = 0; i < right.coefficients_.size(); ++i) {
        out[i] += right.coefficients_[i];
    }
    
    return Polynomial(out);
}

Polynomial Polynomial::subtract(const Polynomial& left, const Polynomial& right) {
    std::size_t n = std::max(left.coefficients_.size(), right.coefficients_.size());
    std::vector<Rational> out(n, Rational(0));
    
    for (std::size_t i = 0; i < left.coefficients_.size(); ++i) {
        out[i] += left.coefficients_[i];
    }
    for (std::size_t i = 0; i < right.coefficients_.size(); ++i) {
        out[i] -= right.coefficients_[i];
    }
    
    return Polynomial(out);
}

Polynomial Polynomial::multiply(const Polynomial& left, const Polynomial& right) {
    std::vector<Rational> out(
                              left.coefficients_.size() + right.coefficients_.size() - 1,
                              Rational(0));
    
    for (std::size_t i = 0; i < left.coefficients_.size(); ++i) {
        for (std::size_t j = 0; j < right.coefficients_.size(); ++j) {
            out[i + j] += left.coefficients_[i] * right.coefficients_[j];
        }
    }
    
    return Polynomial(out);
}

Polynomial Polynomial::scale(const Polynomial& poly, const Rational& factor) {
    std::vector<Rational> out = poly.coefficients_;
    for (Rational& c : out) {
        c *= factor;
    }
    return Polynomial(out);
}

Polynomial Polynomial::power(Polynomial base, int exponent) {
    if (exponent < 0) {
        throw std::invalid_argument("Negative polynomial exponents are not supported.");
    }
    
    Polynomial result = Polynomial::constant(Rational(1));
    
    while (exponent > 0) {
        if (exponent & 1) {
            result = Polynomial::multiply(result, base);
        }
        exponent >>= 1;
        if (exponent > 0) {
            base = Polynomial::multiply(base, base);
        }
    }
    
    return result;
}

void Polynomial::normalize() {
    while (coefficients_.size() > 1 && coefficients_.back().isZero()) {
        coefficients_.pop_back();
    }
    
    if (coefficients_.empty()) {
        coefficients_.push_back(Rational(0));
    }
}

PolynomialDivisionResult divide(const Polynomial& numerator, const Polynomial& denominator) {
    if (denominator.isZero()) {
        throw std::domain_error("Division by zero polynomial.");
    }
    
    if (numerator.degree() < denominator.degree()) {
        return {Polynomial::constant(Rational(0)), numerator};
    }
    
    std::vector<Rational> remainder = numerator.coefficients();
    std::vector<Rational> quotient(
                                   static_cast<std::size_t>(numerator.degree() - denominator.degree() + 1),
                                   Rational(0));
    
    int denominatorDegree = denominator.degree();
    Rational denominatorLead = denominator.leadingCoefficient();
    
    while (static_cast<int>(remainder.size()) - 1 >= denominatorDegree) {
        int remainderDegree = static_cast<int>(remainder.size()) - 1;
        int power = remainderDegree - denominatorDegree;
        Rational coeff = remainder.back() / denominatorLead;
        
        quotient[static_cast<std::size_t>(power)] += coeff;
        
        for (int i = 0; i <= denominatorDegree; ++i) {
            remainder[static_cast<std::size_t>(power + i)] -= coeff * denominator.coefficient(i);
        }
        
        while (remainder.size() > 1 && remainder.back().isZero()) {
            remainder.pop_back();
        }
    }
    
    return {Polynomial(quotient), Polynomial(remainder)};
}

std::ostream& operator<<(std::ostream& out, const Polynomial& polynomial) {
    out << polynomial.toString();
    return out;
}

}  // namespace cas
