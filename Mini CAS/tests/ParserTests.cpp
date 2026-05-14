
//
//  ParserTests.cpp
//  Partial Fraction Mini CAS
//
//  Created by Marco D’Eusebio on 5/14/26.
//

#include "Parser.h"
#include "PartialFractions.h"
#include "Polynomial.h"
#include "Simplify.h"
#include <exception>
#include <iostream>
#include <optional>
#include <sstream>
#include <string>
#include <vector>

namespace {
struct TestContext {
    int passed = 0;
    int failed = 0;
};

void pass(TestContext& ctx, const std::string& name) {
    ++ctx.passed;
    std::cout << "[PASS] " << name << "\n";
}

void fail(TestContext& ctx, const std::string& name, const std::string& details) {
    ++ctx.failed;
    std::cout << "[FAIL] " << name << "\n";
    std::cout << "       " << details << "\n";
}

void expectTrue(TestContext& ctx, bool condition, const std::string& name, const std::string& details) {
    if (condition) {
        pass(ctx, name);
    } else {
        fail(ctx, name, details);
    }
}

void expectEqual(TestContext& ctx,
                 const std::string& actual,
                 const std::string& expected,
                 const std::string& name) {
    if (actual == expected) {
        pass(ctx, name);
    } else {
        std::ostringstream out;
        out << "expected: " << expected << " | actual: " << actual;
        fail(ctx, name, out.str());
    }
}

const cas::LinearPiece* findLinearPiece(const std::vector<cas::LinearPiece>& pieces,
                                        const cas::Rational& root,
                                        int power) {
    for (const cas::LinearPiece& piece : pieces) {
        if (piece.root == root && piece.power == power) {
            return &piece;
        }
    }
    return nullptr;
}

const cas::QuadraticPiece* findQuadraticPiece(const std::vector<cas::QuadraticPiece>& pieces,
                                              const std::string& factorString,
                                              int power) {
    for (const cas::QuadraticPiece& piece : pieces) {
        if (piece.monicFactor.toString() == factorString && piece.power == power) {
            return &piece;
        }
    }
    return nullptr;
}

void testParserPolynomialConversion(TestContext& ctx) {
    {
        cas::ExprPtr expr = cas::parseExpressionFromString("(2/3)*x + x^2 - 5");
        std::optional<cas::Polynomial> poly = cas::tryConvertToPolynomial(expr, "x");
        
        expectTrue(
                   ctx,
                   poly.has_value(),
                   "parser converts '(2/3)*x + x^2 - 5' to polynomial",
                   "expression should be recognized as a polynomial in x");
        
        if (poly.has_value()) {
            expectEqual(
                        ctx,
                        poly->toString(),
                        "x^2 + 2/3*x - 5",
                        "polynomial string matches exact expected output");
        }
    }
    
    {
        cas::ExprPtr expr = cas::parseExpressionFromString("2x + x(x+1)");
        std::optional<cas::Polynomial> poly = cas::tryConvertToPolynomial(expr, "x");
        
        expectTrue(
                   ctx,
                   poly.has_value(),
                   "parser converts '2x + x(x+1)' to polynomial",
                   "expression should be recognized as a polynomial in x");
        
        if (poly.has_value()) {
            expectEqual(
                        ctx,
                        poly->toString(),
                        "x^2 + 3*x",
                        "implicit multiplication simplifies correctly");
        }
    }
    
    {
        cas::ExprPtr expr = cas::parseExpressionFromString("x/2 + (x+1)^2");
        std::optional<cas::Polynomial> poly = cas::tryConvertToPolynomial(expr, "x");
        
        expectTrue(
                   ctx,
                   poly.has_value(),
                   "parser converts 'x/2 + (x+1)^2' to polynomial",
                   "expression should be recognized as a polynomial in x");
        
        if (poly.has_value()) {
            expectEqual(
                        ctx,
                        poly->toString(),
                        "x^2 + 5/2*x + 1",
                        "division by exact numeric literal is handled exactly");
        }
    }
}

void testInvalidSyntax(TestContext& ctx) {
    bool threw = false;
    
    try {
        static_cast<void>(cas::parseExpressionFromString("(x+1"));
    } catch (const std::exception&) {
        threw = true;
    }
    
    expectTrue(
               ctx,
               threw,
               "parser rejects invalid syntax",
               "missing closing parenthesis should throw");
}

void testExactLinearPartialFractions(TestContext& ctx) {
    cas::RationalFunctionInput input =
    cas::parseRationalFunctionString("(x+5)/((x-3)(x+2))");
    cas::PartialFractionResult result = cas::decomposePartialFractions(input);
    
    expectTrue(
               ctx,
               result.ok,
               "distinct linear factor decomposition succeeds",
               result.message.empty() ? "decomposition failed without a message" : result.message);
    
    if (!result.ok) {
        return;
    }
    
    expectTrue(
               ctx,
               result.exactReconstructionMatches,
               "distinct linear factor reconstruction passes",
               "exact polynomial reconstruction should match remainder");
    
    const cas::LinearPiece* piece1 =
    findLinearPiece(result.linearPieces, cas::Rational(3), 1);
    const cas::LinearPiece* piece2 =
    findLinearPiece(result.linearPieces, cas::Rational(-2), 1);
    
    expectTrue(
               ctx,
               piece1 != nullptr,
               "linear piece for root 3 exists",
               "expected a term for (x - 3)");
    
    expectTrue(
               ctx,
               piece2 != nullptr,
               "linear piece for root -2 exists",
               "expected a term for (x + 2)");
    
    if (piece1 != nullptr) {
        expectEqual(
                    ctx,
                    piece1->coefficient.toString(),
                    "8/5",
                    "coefficient for 1/(x - 3) is exact");
    }
    
    if (piece2 != nullptr) {
        expectEqual(
                    ctx,
                    piece2->coefficient.toString(),
                    "-3/5",
                    "coefficient for 1/(x + 2) is exact");
    }
}

void testRepeatedLinearPartialFractions(TestContext& ctx) {
    cas::RationalFunctionInput input =
    cas::parseRationalFunctionString("(2x+1)/((x-1)^2(x+2))");
    cas::PartialFractionResult result = cas::decomposePartialFractions(input);
    
    expectTrue(
               ctx,
               result.ok,
               "repeated linear factor decomposition succeeds",
               result.message.empty() ? "decomposition failed without a message" : result.message);
    
    if (!result.ok) {
        return;
    }
    
    expectTrue(
               ctx,
               result.exactReconstructionMatches,
               "repeated linear factor reconstruction passes",
               "exact polynomial reconstruction should match remainder");
    
    const cas::LinearPiece* pNeg2 =
    findLinearPiece(result.linearPieces, cas::Rational(-2), 1);
    const cas::LinearPiece* pOnePower1 =
    findLinearPiece(result.linearPieces, cas::Rational(1), 1);
    const cas::LinearPiece* pOnePower2 =
    findLinearPiece(result.linearPieces, cas::Rational(1), 2);
    
    expectTrue(ctx, pNeg2 != nullptr, "piece for root -2 exists", "expected 1/(x+2) term");
    expectTrue(ctx, pOnePower1 != nullptr, "piece for root 1 power 1 exists", "expected 1/(x-1) term");
    expectTrue(ctx, pOnePower2 != nullptr, "piece for root 1 power 2 exists", "expected 1/(x-1)^2 term");
    
    if (pNeg2 != nullptr) {
        expectEqual(ctx, pNeg2->coefficient.toString(), "-1/3", "coefficient for 1/(x+2) is exact");
    }
    if (pOnePower1 != nullptr) {
        expectEqual(ctx, pOnePower1->coefficient.toString(), "1/3", "coefficient for 1/(x-1) is exact");
    }
    if (pOnePower2 != nullptr) {
        expectEqual(ctx, pOnePower2->coefficient.toString(), "1", "coefficient for 1/(x-1)^2 is exact");
    }
}

void testQuadraticPartialFractions(TestContext& ctx) {
    cas::RationalFunctionInput input =
    cas::parseRationalFunctionString("(x+1)/((x^2+1)(x-2))");
    cas::PartialFractionResult result = cas::decomposePartialFractions(input);
    
    expectTrue(
               ctx,
               result.ok,
               "linear + quadratic decomposition succeeds",
               result.message.empty() ? "decomposition failed without a message" : result.message);
    
    if (!result.ok) {
        return;
    }
    
    expectTrue(
               ctx,
               result.exactReconstructionMatches,
               "linear + quadratic reconstruction passes",
               "exact polynomial reconstruction should match remainder");
    
    const cas::LinearPiece* linear =
    findLinearPiece(result.linearPieces, cas::Rational(2), 1);
    
    expectTrue(
               ctx,
               linear != nullptr,
               "linear piece for root 2 exists",
               "expected a term for 1/(x-2)");
    
    if (linear != nullptr) {
        expectEqual(
                    ctx,
                    linear->coefficient.toString(),
                    "3/5",
                    "coefficient for 1/(x-2) is exact");
    }
    
    const cas::QuadraticPiece* quadratic =
    findQuadraticPiece(result.quadraticPieces, "x^2 + 1", 1);
    
    expectTrue(
               ctx,
               quadratic != nullptr,
               "quadratic piece for x^2 + 1 exists",
               "expected a (Bx+C)/(x^2+1) term");
    
    if (quadratic != nullptr) {
        expectEqual(
                    ctx,
                    quadratic->coefficientX.toString(),
                    "-3/5",
                    "quadratic x-coefficient is exact");
        expectEqual(
                    ctx,
                    quadratic->coefficientConstant.toString(),
                    "-1/5",
                    "quadratic constant coefficient is exact");
    }
}

void testRepeatedQuadraticPartialFractions(TestContext& ctx) {
    cas::RationalFunctionInput input =
    cas::parseRationalFunctionString("(2x^2+3x+4)/((x^2+1)^2(x-1))");
    cas::PartialFractionResult result = cas::decomposePartialFractions(input);
    
    expectTrue(
               ctx,
               result.ok,
               "repeated quadratic factor decomposition succeeds",
               result.message.empty() ? "decomposition failed without a message" : result.message);
    
    if (!result.ok) {
        return;
    }
    
    expectTrue(
               ctx,
               result.exactReconstructionMatches,
               "repeated quadratic factor reconstruction passes",
               "exact polynomial reconstruction should match remainder");
    
    const cas::QuadraticPiece* q1 =
    findQuadraticPiece(result.quadraticPieces, "x^2 + 1", 1);
    const cas::QuadraticPiece* q2 =
    findQuadraticPiece(result.quadraticPieces, "x^2 + 1", 2);
    
    expectTrue(ctx, q1 != nullptr, "quadratic power-1 piece exists", "expected (Bx+C)/(x^2+1)");
    expectTrue(ctx, q2 != nullptr, "quadratic power-2 piece exists", "expected (Dx+E)/(x^2+1)^2");
}

}  // namespace

int main() {
    TestContext ctx;
    
    std::cout << "Mini-CAS parser + partial fractions tests\n\n";
    
    testParserPolynomialConversion(ctx);
    testInvalidSyntax(ctx);
    testExactLinearPartialFractions(ctx);
    testRepeatedLinearPartialFractions(ctx);
    testQuadraticPartialFractions(ctx);
    testRepeatedQuadraticPartialFractions(ctx);
    
    std::cout << "\nSummary:\n";
    std::cout << "  passed: " << ctx.passed << "\n";
    std::cout << "  failed: " << ctx.failed << "\n";
    
    return (ctx.failed == 0) ? 0 : 1;
}


