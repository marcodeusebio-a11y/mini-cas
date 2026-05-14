//
//  PartialFractions.h
//  Mini Symbolic CAS for Rational Functions
//
//  Created by Marco D’Eusebio on 5/14/26.
//

#pragma once

#include "Expr.h"
#include "Factor.h"
#include "Polynomial.h"
#include "Rational.h"
#include <string>
#include <vector>

namespace cas {

struct RationalFunctionInput {
    ExprPtr numeratorExpr;
    ExprPtr denominatorExpr;
    Polynomial numerator;
    Polynomial denominator;
};

struct QuadraticFactorInfo {
    Polynomial monicFactor = Polynomial::constant(Rational(1));  // x^2 + b*x + c
    Rational b = Rational(0);
    Rational c = Rational(0);
    int multiplicity = 0;
};

struct DenominatorStructure {
    bool ok = false;
    std::string message;
    Rational scale = Rational(1);
    std::vector<LinearFactor> linearFactors;
    std::vector<QuadraticFactorInfo> quadraticFactors;
    Polynomial leftover = Polynomial::constant(Rational(1));
};

struct LinearPiece {
    Rational root = Rational(0);
    int power = 1;
    Rational coefficient = Rational(0);
};

struct QuadraticPiece {
    Polynomial monicFactor = Polynomial::constant(Rational(1));
    Rational b = Rational(0);
    Rational c = Rational(0);
    int power = 1;
    Rational coefficientX = Rational(0);
    Rational coefficientConstant = Rational(0);
};

struct PartialFractionResult {
    bool ok = false;
    std::string message;
    
    RationalFunctionInput parsed;
    Polynomial quotient;
    Polynomial remainder;
    
    DenominatorStructure denominatorStructure;
    
    std::vector<LinearPiece> linearPieces;
    std::vector<QuadraticPiece> quadraticPieces;
    
    std::string decompositionString;
    bool exactReconstructionMatches = false;
};

RationalFunctionInput parseRationalFunctionString(const std::string& input);
PartialFractionResult decomposePartialFractions(const RationalFunctionInput& input);

std::string denominatorStructureToString(const DenominatorStructure& structure);

}  // namespace cas
