/*
//
//  main.cpp
//  Mini Symbolic CAS for Rational Functions
//
//  Created by Marco D’Eusebio on 5/14/26.
//

#include "Expr.h"
#include "Factor.h"
#include "Parser.h"
#include "PartialFractions.h"
#include "Polynomial.h"
#include "Rational.h"
#include "Simplify.h"
#include <iostream>
#include <optional>
#include <string>
#include <vector>

int main() {
    std::cout << "Phase 1 mini-CAS core demo\n\n";
    
    cas::Rational a = cas::Rational::fromString("2/3");
    cas::Rational b = cas::Rational::fromString("5/6");
    
    std::cout << "Exact rational arithmetic:\n";
    std::cout << "a = " << a << "\n";
    std::cout << "b = " << b << "\n";
    std::cout << "a + b = " << (a + b) << "\n";
    std::cout << "a - b = " << (a - b) << "\n";
    std::cout << "a * b = " << (a * b) << "\n";
    std::cout << "a / b = " << (a / b) << "\n\n";
    
    cas::Polynomial p({
        cas::Rational(1),   // constant
        cas::Rational(-2),  // x
        cas::Rational(0),   // x^2
        cas::Rational(1)    // x^3
    });  // x^3 - 2x + 1
    
    cas::Polynomial q({
        cas::Rational(-1),
        cas::Rational(1)
    });  // x - 1
    
    std::cout << "Exact polynomial arithmetic:\n";
    std::cout << "p(x) = " << p << "\n";
    std::cout << "q(x) = " << q << "\n";
    std::cout << "p'(x) = " << p.derivative() << "\n";
    std::cout << "p(3/2) = " << p.evaluate(cas::Rational::fromString("3/2")) << "\n\n";
    
    cas::Polynomial product = cas::Polynomial::multiply(p, q);
    std::cout << "p(x) * q(x) = " << product << "\n";
    
    cas::PolynomialDivisionResult division = cas::divide(product, q);
    std::cout << "\nLong division check:\n";
    std::cout << "(p*q) / q => quotient  = " << division.quotient << "\n";
    std::cout << "(p*q) / q => remainder = " << division.remainder << "\n";
    
    std::cout << "\nNext step:\n";
    std::cout << "Add Expr/Parser/Simplify on top of this exact core.\n";
    
    std::cout << "Phase 2 mini-CAS demo\n\n";
    
    cas::ExprPtr x = cas::makeSymbol("x");
    
    cas::ExprPtr expr = cas::makeAdd({
        cas::makeMul({cas::makeRational(cas::Rational::fromString("2/3")), x}),
        cas::makeMul({cas::makeRational(cas::Rational::fromString("1/3")), x}),
        cas::makeInteger(5),
        cas::makeInteger(-2),
        cas::makePow(x, 2),
        cas::makeMul({cas::makeInteger(-1), cas::makePow(x, 2)}),
        cas::makePow(cas::makeAdd({x, cas::makeInteger(1)}), 2)
    });
    
    std::cout << "Original expression:\n";
    std::cout << "  " << expr->toString() << "\n\n";
    
    cas::ExprPtr simplified = cas::simplify(expr);
    
    std::cout << "Simplified expression:\n";
    std::cout << "  " << simplified->toString() << "\n\n";
    
    std::optional<cas::Polynomial> maybePolynomial = cas::tryConvertToPolynomial(simplified, "x");
    
    if (maybePolynomial.has_value()) {
        std::cout << "Converted to exact polynomial in x:\n";
        std::cout << "  " << maybePolynomial.value() << "\n";
        std::cout << "  derivative: " << maybePolynomial.value().derivative() << "\n";
    } else {
        std::cout << "Expression is not a polynomial in x.\n";
    }
    
    std::cout << "Phase 3 mini-CAS parser demo\n\n";
    std::cout << "Examples:\n";
    std::cout << "  (2/3)*x + x^2 - 5\n";
    std::cout << "  2x + x(x+1)\n";
    std::cout << "  x/2 + (x+1)^2\n\n";
    std::cout << "Enter expression: ";
    
    std::string input;
    std::getline(std::cin, input);
    
    if (input.empty()) {
        std::cerr << "No input provided.\n";
        return 1;
    }
    
    try {
        cas::ExprPtr parsed = cas::parseExpressionFromString(input);
        
        std::cout << "\nSimplified expression:\n";
        std::cout << "  " << parsed->toString() << "\n";
        
        std::optional<cas::Polynomial> maybePolynomial =
        cas::tryConvertToPolynomial(parsed, "x");
        
        if (maybePolynomial.has_value()) {
            std::cout << "\nExact polynomial in x:\n";
            std::cout << "  " << maybePolynomial.value() << "\n";
            std::cout << "  derivative: " << maybePolynomial.value().derivative() << "\n";
        } else {
            std::cout << "\nThis expression is not a polynomial in x.\n";
        }
    } catch (const std::exception& ex) {
        std::cerr << "\nParse error: " << ex.what() << "\n";
        return 1;
    }
    
    std::cout << "Phase 4 mini-CAS exact factoring + exact partial fractions\n\n";
    std::cout << "Examples:\n";
    std::cout << "  (x+5)/((x-3)(x+2))\n";
    std::cout << "  (2x+1)/((x-1)^2(x+2))\n";
    std::cout << "  (x^3+1)/(x(x-1)^2)\n\n";
    std::cout << "Enter rational function: ";
    
    std::getline(std::cin, input);
    
    if (input.empty()) {
        std::cerr << "No input provided.\n";
        return 1;
    }
    
    try {
        cas::RationalFunctionInput parsed = cas::parseRationalFunctionString(input);
        
        std::cout << "\nParsed numerator:\n  " << parsed.numerator << "\n";
        std::cout << "Parsed denominator:\n  " << parsed.denominator << "\n";
        
        cas::ExactFactorizationResult factorization = cas::factorOverQ(parsed.denominator);
        std::cout << "\nExact denominator factorization over Q:\n  "
        << cas::factorizationToString(factorization) << "\n";
        
        if (!factorization.message.empty()) {
            std::cout << "\nFactoring note:\n  " << factorization.message << "\n";
        }
        
        std::vector<std::pair<cas::Polynomial, int>> squareFree =
        cas::squareFreeFactorization(parsed.denominator);
        
        std::cout << "\nSquare-free decomposition:\n";
        if (squareFree.empty()) {
            std::cout << "  (no nontrivial square-free factors)\n";
        } else {
            for (const auto& [poly, multiplicity] : squareFree) {
                std::cout << "  [" << poly << "] multiplicity-layer " << multiplicity << "\n";
            }
        }
        
        cas::PartialFractionResult partial = cas::decomposePartialFractions(parsed);
        
        if (!partial.ok) {
            std::cout << "\nPartial fractions could not be completed:\n  "
            << partial.message << "\n";
            return 0;
        }
        
        std::cout << "\nPolynomial long division:\n";
        std::cout << "  quotient  = " << partial.quotient << "\n";
        std::cout << "  remainder = " << partial.remainder << "\n";
        
        std::cout << "\nExact partial fraction decomposition:\n  "
        << partial.decompositionString << "\n";
        
        std::cout << "\nExact reconstruction check:\n  "
        << (partial.exactReconstructionMatches ? "passed" : "failed")
        << "\n";
    } catch (const std::exception& ex) {
        std::cerr << "\nError: " << ex.what() << "\n";
        return 1;
    }

    std::cout << "Phase 5 mini-CAS exact partial fractions\n\n";
    std::cout << "Supported in this phase:\n";
    std::cout << "  - exact polynomial long division\n";
    std::cout << "  - repeated linear factors over Q\n";
    std::cout << "  - quadratic factors when entered in visible factored form\n";
    std::cout << "  - exact coefficients over Q\n\n";
    
    std::cout << "Examples:\n";
    std::cout << "  (x+5)/((x-3)(x+2))\n";
    std::cout << "  (2x+1)/((x-1)^2(x+2))\n";
    std::cout << "  (x+1)/((x^2+1)(x-2))\n";
    std::cout << "  (2x^2+3x+4)/((x^2+1)^2(x-1))\n\n";
    
    std::cout << "Enter rational function: ";
    
    std::getline(std::cin, input);
    
    if (input.empty()) {
        std::cerr << "No input provided.\n";
        return 1;
    }
    
    try {
        cas::RationalFunctionInput parsed = cas::parseRationalFunctionString(input);
        
        std::cout << "\nParsed numerator:\n  " << parsed.numerator << "\n";
        std::cout << "Parsed denominator:\n  " << parsed.denominator << "\n";
        
        cas::PartialFractionResult result = cas::decomposePartialFractions(parsed);
        
        if (!result.ok) {
            std::cout << "\nPartial fractions could not be completed:\n";
            std::cout << "  " << result.message << "\n";
            return 0;
        }
        
        std::cout << "\nDenominator structure:\n";
        std::cout << "  " << cas::denominatorStructureToString(result.denominatorStructure) << "\n";
        
        if (!result.denominatorStructure.linearFactors.empty()) {
            std::cout << "\nLinear factors:\n";
            for (const cas::LinearFactor& lf : result.denominatorStructure.linearFactors) {
                std::cout << "  root = " << lf.root
                << ", multiplicity = " << lf.multiplicity << "\n";
            }
        }
        
        if (!result.denominatorStructure.quadraticFactors.empty()) {
            std::cout << "\nQuadratic factors:\n";
            for (const cas::QuadraticFactorInfo& qf : result.denominatorStructure.quadraticFactors) {
                std::cout << "  factor = (" << qf.monicFactor << ")"
                << ", multiplicity = " << qf.multiplicity << "\n";
            }
        }
        
        std::cout << "\nPolynomial long division:\n";
        std::cout << "  quotient  = " << result.quotient << "\n";
        std::cout << "  remainder = " << result.remainder << "\n";
        
        std::cout << "\nExact partial fraction decomposition:\n";
        std::cout << "  " << result.decompositionString << "\n";
        
        if (!result.linearPieces.empty()) {
            std::cout << "\nLinear partial-fraction pieces:\n";
            for (const cas::LinearPiece& piece : result.linearPieces) {
                std::cout << "  " << piece.coefficient
                << " / " << cas::linearFactorToString(piece.root);
                if (piece.power > 1) {
                    std::cout << "^" << piece.power;
                }
                std::cout << "\n";
            }
        }
        
        if (!result.quadraticPieces.empty()) {
            std::cout << "\nQuadratic partial-fraction pieces:\n";
            for (const cas::QuadraticPiece& piece : result.quadraticPieces) {
                std::cout << "  (" << piece.coefficientX << "*x + "
                << piece.coefficientConstant << ") / ("
                << piece.monicFactor << ")";
                if (piece.power > 1) {
                    std::cout << "^" << piece.power;
                }
                std::cout << "\n";
            }
        }
        
        std::cout << "\nExact reconstruction check:\n";
        std::cout << "  " << (result.exactReconstructionMatches ? "passed" : "failed") << "\n";
        
        if (!result.exactReconstructionMatches) {
            return 1;
        }
        
        return 0;
    } catch (const std::exception& ex) {
        std::cerr << "\nError: " << ex.what() << "\n";
        return 1;
    }
}
 
*/
