//
//  PartialFractions.cpp
//  Mini Symbolic CAS for Rational Functions
//
//  Created by Marco D’Eusebio on 5/14/26.
//

#include "PartialFractions.h"
#include "Parser.h"
#include "Simplify.h"
#include <algorithm>
#include <cctype>
#include <optional>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace cas {
namespace {

bool isOnePolynomial(const Polynomial& polynomial) {
    return polynomial.degree() == 0 && polynomial.leadingCoefficient() == Rational(1);
}

bool polynomialsEqual(const Polynomial& left, const Polynomial& right) {
    int maxDegree = std::max(left.degree(), right.degree());
    for (int i = 0; i <= maxDegree; ++i) {
        if (left.coefficient(i) != right.coefficient(i)) {
            return false;
        }
    }
    return true;
}

Rational powRational(Rational base, int exponent) {
    if (exponent < 0) {
        throw std::invalid_argument("Negative rational exponents are not supported.");
    }
    
    Rational result(1);
    while (exponent > 0) {
        if (exponent & 1) {
            result *= base;
        }
        exponent >>= 1;
        if (exponent > 0) {
            base *= base;
        }
    }
    
    return result;
}

std::string stripSpaces(const std::string& input) {
    std::string out;
    for (char ch : input) {
        if (!std::isspace(static_cast<unsigned char>(ch))) {
            out.push_back(ch);
        }
    }
    return out;
}

std::pair<std::string, std::string> splitTopLevelDivision(const std::string& input) {
    int depth = 0;
    std::vector<std::size_t> slashPositions;
    
    for (std::size_t i = 0; i < input.size(); ++i) {
        if (input[i] == '(') {
            ++depth;
        } else if (input[i] == ')') {
            --depth;
        } else if (input[i] == '/' && depth == 0) {
            slashPositions.push_back(i);
        }
    }
    
    if (slashPositions.size() != 1) {
        throw std::runtime_error("Enter one rational function with one top-level '/'.");
    }
    
    std::size_t pos = slashPositions.front();
    return {input.substr(0, pos), input.substr(pos + 1)};
}

Polynomial parsePolynomialSide(const std::string& text, ExprPtr& exprOut) {
    exprOut = parseExpressionFromString(text);
    std::optional<Polynomial> poly = tryConvertToPolynomial(exprOut, "x");
    
    if (!poly.has_value()) {
        throw std::runtime_error("Expression is not a polynomial in x: " + text);
    }
    
    return poly.value();
}

void mergeLinearFactor(std::vector<LinearFactor>& factors,
                       const Rational& root,
                       int multiplicity) {
    for (LinearFactor& lf : factors) {
        if (lf.root == root) {
            lf.multiplicity += multiplicity;
            return;
        }
    }
    
    factors.push_back({root, multiplicity});
}

void mergeQuadraticFactor(std::vector<QuadraticFactorInfo>& factors,
                          const Polynomial& monicFactor,
                          const Rational& b,
                          const Rational& c,
                          int multiplicity) {
    for (QuadraticFactorInfo& qf : factors) {
        if (qf.monicFactor.toString() == monicFactor.toString()) {
            qf.multiplicity += multiplicity;
            return;
        }
    }
    
    QuadraticFactorInfo info;
    info.monicFactor = monicFactor;
    info.b = b;
    info.c = c;
    info.multiplicity = multiplicity;
    factors.push_back(info);
}

bool tryAppendPolynomialFactor(const Polynomial& factorPolynomial,
                               int multiplicity,
                               DenominatorStructure& structure) {
    if (multiplicity <= 0) {
        return true;
    }
    
    if (factorPolynomial.isZero()) {
        structure.ok = false;
        structure.message = "Zero polynomial factor in denominator.";
        return false;
    }
    
    if (factorPolynomial.degree() == 0) {
        structure.scale *= powRational(factorPolynomial.leadingCoefficient(), multiplicity);
        return true;
    }
    
    ExactFactorizationResult exact = factorOverQ(factorPolynomial);
    if (!exact.ok) {
        structure.ok = false;
        structure.message = exact.message;
        return false;
    }
    
    structure.scale *= powRational(exact.scale, multiplicity);
    
    for (const LinearFactor& lf : exact.linearFactors) {
        mergeLinearFactor(structure.linearFactors, lf.root, lf.multiplicity * multiplicity);
    }
    
    if (isOnePolynomial(exact.leftover)) {
        return true;
    }
    
    if (exact.leftover.degree() == 2) {
        mergeQuadraticFactor(
                             structure.quadraticFactors,
                             exact.leftover,
                             exact.leftover.coefficient(1),
                             exact.leftover.coefficient(0),
                             multiplicity);
        return true;
    }
    
    structure.ok = false;
    structure.message =
    "Unsupported exact leftover factor in this phase: " + exact.leftover.toString() +
    ". Enter the denominator in factored form when quadratic factors are involved.";
    return false;
}

bool collectStructuredFactors(const ExprPtr& expr,
                              int multiplicity,
                              DenominatorStructure& structure) {
    if (multiplicity <= 0) {
        return true;
    }
    
    if (expr->kind() == Expr::Kind::Mul) {
        const auto mulNode = std::dynamic_pointer_cast<const MulExpr>(expr);
        for (const ExprPtr& factor : mulNode->factors()) {
            if (!collectStructuredFactors(factor, multiplicity, structure)) {
                return false;
            }
        }
        return true;
    }
    
    if (expr->kind() == Expr::Kind::Pow) {
        const auto powNode = std::dynamic_pointer_cast<const PowExpr>(expr);
        return collectStructuredFactors(powNode->base(), multiplicity * powNode->exponent(), structure);
    }
    
    std::optional<Polynomial> poly = tryConvertToPolynomial(expr, "x");
    if (!poly.has_value()) {
        structure.ok = false;
        structure.message = "Denominator factor is not a polynomial in x: " + expr->toString();
        return false;
    }
    
    return tryAppendPolynomialFactor(poly.value(), multiplicity, structure);
}

Polynomial rebuildDenominatorFromStructure(const DenominatorStructure& structure) {
    Polynomial result = Polynomial::constant(structure.scale);
    
    for (const LinearFactor& lf : structure.linearFactors) {
        Polynomial factor({-lf.root, Rational(1)});
        result = Polynomial::multiply(result, Polynomial::power(factor, lf.multiplicity));
    }
    
    for (const QuadraticFactorInfo& qf : structure.quadraticFactors) {
        result = Polynomial::multiply(result, Polynomial::power(qf.monicFactor, qf.multiplicity));
    }
    
    if (!isOnePolynomial(structure.leftover)) {
        result = Polynomial::multiply(result, structure.leftover);
    }
    
    return result;
}

DenominatorStructure fallbackStructureFromPolynomial(const Polynomial& denominatorPoly) {
    DenominatorStructure structure;
    structure.ok = true;
    structure.scale = Rational(1);
    structure.leftover = Polynomial::constant(Rational(1));
    
    ExactFactorizationResult exact = factorOverQ(denominatorPoly);
    if (!exact.ok) {
        structure.ok = false;
        structure.message = exact.message;
        return structure;
    }
    
    structure.scale = exact.scale;
    structure.linearFactors = exact.linearFactors;
    structure.quadraticFactors.clear();
    structure.leftover = Polynomial::constant(Rational(1));
    
    if (isOnePolynomial(exact.leftover)) {
        return structure;
    }
    
    if (exact.leftover.degree() == 2) {
        structure.quadraticFactors.push_back({
            exact.leftover,
            exact.leftover.coefficient(1),
            exact.leftover.coefficient(0),
            1
        });
        return structure;
    }
    
    structure.ok = false;
    structure.message =
    "Could not extract a full exact linear/quadratic factorization from the denominator. "
    "Enter the denominator in factored form for quadratic-factor support.";
    return structure;
}

DenominatorStructure extractDenominatorStructure(const ExprPtr& denominatorExpr,
                                                 const Polynomial& denominatorPoly) {
    DenominatorStructure structure;
    structure.ok = true;
    structure.scale = Rational(1);
    structure.leftover = Polynomial::constant(Rational(1));
    
    if (!collectStructuredFactors(denominatorExpr, 1, structure)) {
        return structure;
    }
    
    std::sort(
              structure.linearFactors.begin(),
              structure.linearFactors.end(),
              [](const LinearFactor& left, const LinearFactor& right) {
                  return left.root < right.root;
              });
    
    std::sort(
              structure.quadraticFactors.begin(),
              structure.quadraticFactors.end(),
              [](const QuadraticFactorInfo& left, const QuadraticFactorInfo& right) {
                  return left.monicFactor.toString() < right.monicFactor.toString();
              });
    
    Polynomial rebuilt = rebuildDenominatorFromStructure(structure);
    if (!polynomialsEqual(rebuilt, denominatorPoly)) {
        return fallbackStructureFromPolynomial(denominatorPoly);
    }
    
    return structure;
}

struct ExactLinearSolveResult {
    bool ok = false;
    std::string message;
    std::vector<Rational> solution;
};

ExactLinearSolveResult solveLinearSystemExact(std::vector<std::vector<Rational>> matrix,
                                              std::vector<Rational> rhs) {
    int n = static_cast<int>(matrix.size());
    if (n == 0) {
        return {true, "", {}};
    }
    
    for (int col = 0; col < n; ++col) {
        int pivot = -1;
        for (int row = col; row < n; ++row) {
            if (!(matrix[row][col] == Rational(0))) {
                pivot = row;
                break;
            }
        }
        
        if (pivot == -1) {
            return {false, "Linear system is singular.", {}};
        }
        
        std::swap(matrix[col], matrix[pivot]);
        std::swap(rhs[col], rhs[pivot]);
        
        Rational pivotValue = matrix[col][col];
        for (int j = col; j < n; ++j) {
            matrix[col][j] /= pivotValue;
        }
        rhs[col] /= pivotValue;
        
        for (int row = 0; row < n; ++row) {
            if (row == col) {
                continue;
            }
            
            Rational factor = matrix[row][col];
            if (factor == Rational(0)) {
                continue;
            }
            
            for (int j = col; j < n; ++j) {
                matrix[row][j] -= factor * matrix[col][j];
            }
            rhs[row] -= factor * rhs[col];
        }
    }
    
    return {true, "", rhs};
}

std::string factorPowerString(const std::string& base, int power) {
    if (power == 1) {
        return base;
    }
    return base + "^" + std::to_string(power);
}

std::string buildDecompositionString(const Polynomial& quotient,
                                     const std::vector<LinearPiece>& linearPieces,
                                     const std::vector<QuadraticPiece>& quadraticPieces) {
    std::string out;
    bool first = true;
    
    if (!quotient.isZero()) {
        out += quotient.toString();
        first = false;
    }
    
    auto appendTerm = [&](const std::string& text) {
        if (first) {
            out += text;
            first = false;
            return;
        }
        
        if (!text.empty() && text.front() == '-') {
            out += " - " + text.substr(1);
        } else {
            out += " + " + text;
        }
    };
    
    for (const LinearPiece& piece : linearPieces) {
        Rational coeff = piece.coefficient;
        std::string denominator = factorPowerString(linearFactorToString(piece.root), piece.power);
        
        std::string text;
        if (coeff < Rational(0)) {
            text = "-" + (-coeff).toString() + "/" + denominator;
        } else {
            text = coeff.toString() + "/" + denominator;
        }
        
        appendTerm(text);
    }
    
    for (const QuadraticPiece& piece : quadraticPieces) {
        Polynomial numerator({piece.coefficientConstant, piece.coefficientX});
        std::string denominator =
        factorPowerString("(" + piece.monicFactor.toString() + ")", piece.power);
        std::string text = "(" + numerator.toString() + ")/" + denominator;
        appendTerm(text);
    }
    
    if (out.empty()) {
        return "0";
    }
    
    return out;
}

bool reconstructionMatches(const Polynomial& expected,
                           const Polynomial& actual) {
    return polynomialsEqual(expected, actual);
}

}  // namespace

RationalFunctionInput parseRationalFunctionString(const std::string& input) {
    std::string cleaned = stripSpaces(input);
    std::pair<std::string, std::string> split = splitTopLevelDivision(cleaned);
    
    RationalFunctionInput result;
    result.numerator = parsePolynomialSide(split.first, result.numeratorExpr);
    result.denominator = parsePolynomialSide(split.second, result.denominatorExpr);
    
    if (result.denominator.isZero()) {
        throw std::runtime_error("Denominator cannot be zero.");
    }
    
    return result;
}

std::string denominatorStructureToString(const DenominatorStructure& structure) {
    std::string out;
    bool first = true;
    
    if (!(structure.scale == Rational(1))) {
        out += structure.scale.toString();
        first = false;
    }
    
    for (const LinearFactor& lf : structure.linearFactors) {
        if (!first) {
            out += " * ";
        }
        out += factorPowerString(linearFactorToString(lf.root), lf.multiplicity);
        first = false;
    }
    
    for (const QuadraticFactorInfo& qf : structure.quadraticFactors) {
        if (!first) {
            out += " * ";
        }
        out += factorPowerString("(" + qf.monicFactor.toString() + ")", qf.multiplicity);
        first = false;
    }
    
    if (!isOnePolynomial(structure.leftover)) {
        if (!first) {
            out += " * ";
        }
        out += "(" + structure.leftover.toString() + ")";
        first = false;
    }
    
    if (first) {
        return "1";
    }
    
    return out;
}

PartialFractionResult decomposePartialFractions(const RationalFunctionInput& input) {
    PartialFractionResult result;
    result.parsed = input;
    
    try {
        PolynomialDivisionResult division = divide(input.numerator, input.denominator);
        result.quotient = division.quotient;
        result.remainder = division.remainder;
        
        result.denominatorStructure =
        extractDenominatorStructure(input.denominatorExpr, input.denominator);
        
        if (!result.denominatorStructure.ok) {
            result.ok = false;
            result.message = result.denominatorStructure.message;
            return result;
        }
        
        if (!isOnePolynomial(result.denominatorStructure.leftover)) {
            result.ok = false;
            result.message =
            "This phase supports exact partial fractions only for denominators "
            "that decompose into exact linear and/or quadratic factors.";
            return result;
        }
        
        if (result.remainder.isZero()) {
            result.decompositionString = result.quotient.toString();
            result.exactReconstructionMatches = true;
            result.ok = true;
            return result;
        }
        
        std::vector<LinearPiece> linearPieces;
        std::vector<QuadraticPiece> quadraticPieces;
        std::vector<Polynomial> basisPolynomials;
        
        for (const LinearFactor& lf : result.denominatorStructure.linearFactors) {
            Polynomial factor({-lf.root, Rational(1)});
            
            for (int power = 1; power <= lf.multiplicity; ++power) {
                PolynomialDivisionResult div =
                divide(input.denominator, Polynomial::power(factor, power));
                
                LinearPiece piece;
                piece.root = lf.root;
                piece.power = power;
                piece.coefficient = Rational(0);
                
                linearPieces.push_back(piece);
                basisPolynomials.push_back(div.quotient);
            }
        }
        
        for (const QuadraticFactorInfo& qf : result.denominatorStructure.quadraticFactors) {
            for (int power = 1; power <= qf.multiplicity; ++power) {
                PolynomialDivisionResult div =
                divide(input.denominator, Polynomial::power(qf.monicFactor, power));
                
                QuadraticPiece piece;
                piece.monicFactor = qf.monicFactor;
                piece.b = qf.b;
                piece.c = qf.c;
                piece.power = power;
                piece.coefficientX = Rational(0);
                piece.coefficientConstant = Rational(0);
                
                quadraticPieces.push_back(piece);
                
                basisPolynomials.push_back(
                                           Polynomial::multiply(div.quotient, Polynomial::variable()));
                basisPolynomials.push_back(div.quotient);
            }
        }
        
        int unknownCount = static_cast<int>(basisPolynomials.size());
        if (unknownCount != input.denominator.degree()) {
            result.ok = false;
            result.message =
            "Internal setup error: number of unknowns does not match denominator degree.";
            return result;
        }
        
        std::vector<std::vector<Rational>> matrix(
                                                  static_cast<std::size_t>(unknownCount),
                                                  std::vector<Rational>(static_cast<std::size_t>(unknownCount), Rational(0)));
        
        std::vector<Rational> rhs(static_cast<std::size_t>(unknownCount), Rational(0));
        
        for (int power = 0; power < unknownCount; ++power) {
            rhs[static_cast<std::size_t>(power)] = result.remainder.coefficient(power);
            
            for (int col = 0; col < unknownCount; ++col) {
                matrix[static_cast<std::size_t>(power)][static_cast<std::size_t>(col)] =
                basisPolynomials[static_cast<std::size_t>(col)].coefficient(power);
            }
        }
        
        ExactLinearSolveResult solved = solveLinearSystemExact(matrix, rhs);
        if (!solved.ok) {
            result.ok = false;
            result.message = solved.message;
            return result;
        }
        
        std::size_t solutionIndex = 0;
        for (LinearPiece& piece : linearPieces) {
            piece.coefficient = solved.solution[solutionIndex++];
        }
        for (QuadraticPiece& piece : quadraticPieces) {
            piece.coefficientX = solved.solution[solutionIndex++];
            piece.coefficientConstant = solved.solution[solutionIndex++];
        }
        
        result.linearPieces = linearPieces;
        result.quadraticPieces = quadraticPieces;
        result.decompositionString =
        buildDecompositionString(result.quotient, result.linearPieces, result.quadraticPieces);
        
        Polynomial reconstructedRemainder = Polynomial::constant(Rational(0));
        std::size_t basisIndex = 0;
        
        for (const LinearPiece& piece : result.linearPieces) {
            reconstructedRemainder = Polynomial::add(
                                                     reconstructedRemainder,
                                                     Polynomial::scale(basisPolynomials[basisIndex], piece.coefficient));
            ++basisIndex;
        }
        
        for (const QuadraticPiece& piece : result.quadraticPieces) {
            reconstructedRemainder = Polynomial::add(
                                                     reconstructedRemainder,
                                                     Polynomial::scale(basisPolynomials[basisIndex], piece.coefficientX));
            ++basisIndex;
            reconstructedRemainder = Polynomial::add(
                                                     reconstructedRemainder,
                                                     Polynomial::scale(basisPolynomials[basisIndex], piece.coefficientConstant));
            ++basisIndex;
        }
        
        result.exactReconstructionMatches =
        reconstructionMatches(result.remainder, reconstructedRemainder);
        
        result.ok = true;
        return result;
    } catch (const std::exception& ex) {
        result.ok = false;
        result.message = ex.what();
        return result;
    }
}

}  // namespace cas
