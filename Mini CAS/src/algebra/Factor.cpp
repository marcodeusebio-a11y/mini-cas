//
//  Factor.cpp
//  Mini Symbolic CAS for Rational Functions
//
//  Created by Marco D’Eusebio on 5/14/26.
//

#include "Factor.h"
#include <algorithm>
#include <map>
#include <numeric>
#include <sstream>
#include <stdexcept>
#include <utility>
#include <vector>

namespace cas {
namespace {

BigInt absBigInt(BigInt value) {
    return value < 0 ? -value : value;
}

BigInt gcdBigInt(BigInt a, BigInt b) {
    a = absBigInt(a);
    b = absBigInt(b);
    
    while (b != 0) {
        BigInt r = a % b;
        a = b;
        b = r;
    }
    
    return a == 0 ? BigInt(1) : a;
}

BigInt lcmBigInt(const BigInt& a, const BigInt& b) {
    if (a == 0 || b == 0) {
        return 0;
    }
    return absBigInt(a / gcdBigInt(a, b) * b);
}

std::vector<BigInt> divisorsOf(const BigInt& n) {
    std::vector<BigInt> result;
    BigInt value = absBigInt(n);
    
    if (value == 0) {
        result.push_back(0);
        return result;
    }
    
    long long asLongLong = 0;
    try {
        asLongLong = value.convert_to<long long>();
    } catch (...) {
        throw std::runtime_error("Integer too large for this phase's divisor search.");
    }
    
    for (long long d = 1; d * d <= asLongLong; ++d) {
        if (asLongLong % d == 0) {
            result.push_back(BigInt(d));
            if (d * d != asLongLong) {
                result.push_back(BigInt(asLongLong / d));
            }
        }
    }
    
    return result;
}

Polynomial divideByLinearFactor(const Polynomial& polynomial, const Rational& root) {
    Polynomial divisor({-root, Rational(1)});
    PolynomialDivisionResult div = divide(polynomial, divisor);
    
    if (!div.remainder.isZero()) {
        throw std::runtime_error("Expected exact division by linear factor.");
    }
    
    return div.quotient;
}

struct IntegerPolynomialData {
    std::vector<BigInt> coefficientsAscending;
    BigInt denominatorLcm = 1;
    BigInt content = 1;
};

IntegerPolynomialData toPrimitiveIntegerPolynomial(const Polynomial& polynomial) {
    IntegerPolynomialData data;
    
    BigInt lcm = 1;
    for (const Rational& c : polynomial.coefficients()) {
        lcm = lcmBigInt(lcm, c.denominator());
    }
    
    std::vector<BigInt> integerCoeffs;
    integerCoeffs.reserve(polynomial.coefficients().size());
    
    for (const Rational& c : polynomial.coefficients()) {
        BigInt scaled = c.numerator() * (lcm / c.denominator());
        integerCoeffs.push_back(scaled);
    }
    
    BigInt content = 0;
    for (const BigInt& c : integerCoeffs) {
        content = gcdBigInt(content, c);
    }
    if (content == 0) {
        content = 1;
    }
    
    for (BigInt& c : integerCoeffs) {
        c /= content;
    }
    
    data.coefficientsAscending = std::move(integerCoeffs);
    data.denominatorLcm = lcm;
    data.content = content;
    return data;
}

}  // namespace

Polynomial makeMonic(const Polynomial& polynomial) {
    if (polynomial.isZero()) {
        return polynomial;
    }
    return Polynomial::scale(polynomial, Rational(1) / polynomial.leadingCoefficient());
}

Polynomial gcd(Polynomial left, Polynomial right) {
    if (left.isZero()) {
        return makeMonic(right);
    }
    if (right.isZero()) {
        return makeMonic(left);
    }
    
    while (!right.isZero()) {
        PolynomialDivisionResult div = divide(left, right);
        left = right;
        right = div.remainder;
    }
    
    return makeMonic(left);
}

std::vector<std::pair<Polynomial, int>> squareFreeFactorization(const Polynomial& polynomial) {
    std::vector<std::pair<Polynomial, int>> result;
    
    if (polynomial.degree() <= 0) {
        return result;
    }
    
    Polynomial f = makeMonic(polynomial);
    Polynomial df = f.derivative();
    
    if (df.isZero()) {
        result.push_back({f, 1});
        return result;
    }
    
    Polynomial a = gcd(f, df);
    Polynomial b = divide(f, a).quotient;
    int i = 1;
    
    while (!b.isZero() && b.degree() > 0) {
        Polynomial c = gcd(b, a);
        Polynomial y = divide(b, c).quotient;
        
        if (!y.isZero() && y.degree() >= 0 && !(y.degree() == 0 && y.leadingCoefficient() == Rational(1))) {
            result.push_back({makeMonic(y), i});
        }
        
        b = c;
        a = divide(a, c).quotient;
        ++i;
        
        if (b.degree() == 0 && b.leadingCoefficient() == Rational(1)) {
            break;
        }
    }
    
    return result;
}

std::optional<Rational> findRationalRoot(const Polynomial& polynomial) {
    if (polynomial.degree() <= 0) {
        return std::nullopt;
    }
    
    IntegerPolynomialData primitive = toPrimitiveIntegerPolynomial(polynomial);
    const std::vector<BigInt>& coeffs = primitive.coefficientsAscending;
    
    BigInt constantTerm = coeffs.front();
    BigInt leadingTerm = coeffs.back();
    
    if (constantTerm == 0) {
        return Rational(0);
    }
    
    std::vector<BigInt> pDivisors = divisorsOf(constantTerm);
    std::vector<BigInt> qDivisors = divisorsOf(leadingTerm);
    
    for (const BigInt& p : pDivisors) {
        for (const BigInt& q : qDivisors) {
            if (q == 0) {
                continue;
            }
            
            Rational candidatePositive(p, q);
            if (polynomial.evaluate(candidatePositive).isZero()) {
                return candidatePositive;
            }
            
            Rational candidateNegative(-p, q);
            if (polynomial.evaluate(candidateNegative).isZero()) {
                return candidateNegative;
            }
        }
    }
    
    return std::nullopt;
}

ExactFactorizationResult factorOverQ(const Polynomial& polynomial) {
    ExactFactorizationResult result;
    result.scale = polynomial.leadingCoefficient();
    result.leftover = makeMonic(polynomial);
    
    if (polynomial.isZero()) {
        result.ok = false;
        result.message = "Cannot factor the zero polynomial.";
        return result;
    }
    
    if (polynomial.degree() <= 0) {
        result.ok = true;
        result.leftover = Polynomial::constant(Rational(1));
        return result;
    }
    
    Polynomial monic = makeMonic(polynomial);
    std::map<std::string, LinearFactor> grouped;
    
    while (monic.degree() > 0) {
        std::optional<Rational> root = findRationalRoot(monic);
        if (!root.has_value()) {
            break;
        }
        
        int multiplicity = 0;
        while (monic.degree() > 0 && monic.evaluate(root.value()).isZero()) {
            monic = divideByLinearFactor(monic, root.value());
            ++multiplicity;
        }
        
        std::string key = root->toString();
        if (grouped.count(key) == 0) {
            grouped[key] = {root.value(), multiplicity};
        } else {
            grouped[key].multiplicity += multiplicity;
        }
    }
    
    for (const auto& [_, lf] : grouped) {
        result.linearFactors.push_back(lf);
    }
    
    std::sort(
              result.linearFactors.begin(),
              result.linearFactors.end(),
              [](const LinearFactor& left, const LinearFactor& right) {
                  return left.root < right.root;
              });
    
    result.leftover = monic;
    result.ok = true;
    
    if (result.leftover.degree() > 0 && !(result.leftover.degree() == 0 && result.leftover.leadingCoefficient() == Rational(1))) {
        result.message = "Leftover irreducible factor over Q: " + result.leftover.toString();
    }
    
    return result;
}

std::string linearFactorToString(const Rational& root) {
    if (root >= Rational(0)) {
        return "(x - " + root.toString() + ")";
    }
    return "(x + " + (-root).toString() + ")";
}

std::string factorizationToString(const ExactFactorizationResult& factorization) {
    std::ostringstream out;
    bool first = true;
    
    if (!(factorization.scale == Rational(1))) {
        out << factorization.scale;
        first = false;
    }
    
    for (const LinearFactor& lf : factorization.linearFactors) {
        if (!first) {
            out << " * ";
        }
        out << linearFactorToString(lf.root);
        if (lf.multiplicity > 1) {
            out << "^" << lf.multiplicity;
        }
        first = false;
    }
    
    if (!(factorization.leftover.degree() == 0 &&
          factorization.leftover.leadingCoefficient() == Rational(1))) {
        if (!first) {
            out << " * ";
        }
        out << "(" << factorization.leftover.toString() << ")";
    }
    
    if (first) {
        return "1";
    }
    
    return out.str();
}

}  // namespace cas
