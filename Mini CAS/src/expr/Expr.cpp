//
//  Expr.cpp
//  Mini Symbolic CAS for Rational Functions
//
//  Created by Marco D’Eusebio on 5/14/26.
//

#include "Expr.h"
#include <sstream>
#include <stdexcept>
#include <utility>

namespace cas {
namespace {

std::string bigIntToString(const BigInt& value) {
    std::ostringstream out;
    out << value;
    return out.str();
}

bool needsParenthesesInProduct(const ExprPtr& expr) {
    return expr->kind() == Expr::Kind::Add;
}

bool needsParenthesesInPower(const ExprPtr& expr) {
    return expr->kind() == Expr::Kind::Add || expr->kind() == Expr::Kind::Mul;
}

}  // namespace

IntegerExpr::IntegerExpr(BigInt value) : value_(std::move(value)) {}

Expr::Kind IntegerExpr::kind() const {
    return Kind::Integer;
}

const BigInt& IntegerExpr::value() const {
    return value_;
}

std::string IntegerExpr::toString() const {
    return bigIntToString(value_);
}

RationalExpr::RationalExpr(Rational value) : value_(std::move(value)) {}

Expr::Kind RationalExpr::kind() const {
    return Kind::Rational;
}

const Rational& RationalExpr::value() const {
    return value_;
}

std::string RationalExpr::toString() const {
    return value_.toString();
}

SymbolExpr::SymbolExpr(std::string name) : name_(std::move(name)) {}

Expr::Kind SymbolExpr::kind() const {
    return Kind::Symbol;
}

const std::string& SymbolExpr::name() const {
    return name_;
}

std::string SymbolExpr::toString() const {
    return name_;
}

AddExpr::AddExpr(std::vector<ExprPtr> terms) : terms_(std::move(terms)) {}

Expr::Kind AddExpr::kind() const {
    return Kind::Add;
}

const std::vector<ExprPtr>& AddExpr::terms() const {
    return terms_;
}

std::string AddExpr::toString() const {
    if (terms_.empty()) {
        return "0";
    }
    
    std::string out;
    bool first = true;
    
    for (const ExprPtr& term : terms_) {
        std::string text = term->toString();
        
        if (first) {
            out += text;
            first = false;
            continue;
        }
        
        if (!text.empty() && text.front() == '-') {
            out += " - ";
            out += text.substr(1);
        } else {
            out += " + ";
            out += text;
        }
    }
    
    return out;
}

MulExpr::MulExpr(std::vector<ExprPtr> factors) : factors_(std::move(factors)) {}

Expr::Kind MulExpr::kind() const {
    return Kind::Mul;
}

const std::vector<ExprPtr>& MulExpr::factors() const {
    return factors_;
}

std::string MulExpr::toString() const {
    if (factors_.empty()) {
        return "1";
    }
    
    std::string out;
    bool first = true;
    
    for (const ExprPtr& factor : factors_) {
        if (!first) {
            out += "*";
        }
        
        if (needsParenthesesInProduct(factor)) {
            out += "(" + factor->toString() + ")";
        } else {
            out += factor->toString();
        }
        
        first = false;
    }
    
    return out;
}

PowExpr::PowExpr(ExprPtr base, int exponent)
: base_(std::move(base)), exponent_(exponent) {
    if (exponent_ < 0) {
        throw std::invalid_argument("Negative exponents are not supported in PowExpr.");
    }
}

Expr::Kind PowExpr::kind() const {
    return Kind::Pow;
}

const ExprPtr& PowExpr::base() const {
    return base_;
}

int PowExpr::exponent() const {
    return exponent_;
}

std::string PowExpr::toString() const {
    std::string baseText = needsParenthesesInPower(base_) ? "(" + base_->toString() + ")" : base_->toString();
    return baseText + "^" + std::to_string(exponent_);
}

ExprPtr makeInteger(long long value) {
    return std::make_shared<IntegerExpr>(BigInt(value));
}

ExprPtr makeInteger(const BigInt& value) {
    return std::make_shared<IntegerExpr>(value);
}

ExprPtr makeRational(const Rational& value) {
    if (value.denominator() == 1) {
        return makeInteger(value.numerator());
    }
    return std::make_shared<RationalExpr>(value);
}

ExprPtr makeSymbol(const std::string& name) {
    return std::make_shared<SymbolExpr>(name);
}

ExprPtr makeAdd(const std::vector<ExprPtr>& terms) {
    if (terms.size() == 1) {
        return terms.front();
    }
    return std::make_shared<AddExpr>(terms);
}

ExprPtr makeMul(const std::vector<ExprPtr>& factors) {
    if (factors.size() == 1) {
        return factors.front();
    }
    return std::make_shared<MulExpr>(factors);
}

ExprPtr makePow(const ExprPtr& base, int exponent) {
    if (exponent == 0) {
        return makeInteger(1);
    }
    if (exponent == 1) {
        return base;
    }
    return std::make_shared<PowExpr>(base, exponent);
}

ExprPtr makeNegate(const ExprPtr& expr) {
    return makeMul({makeInteger(-1), expr});
}

bool isNumericExpr(const ExprPtr& expr) {
    return expr->kind() == Expr::Kind::Integer || expr->kind() == Expr::Kind::Rational;
}

Rational numericValueOf(const ExprPtr& expr) {
    if (expr->kind() == Expr::Kind::Integer) {
        const auto integerNode = std::dynamic_pointer_cast<const IntegerExpr>(expr);
        return Rational(integerNode->value());
    }
    if (expr->kind() == Expr::Kind::Rational) {
        const auto rationalNode = std::dynamic_pointer_cast<const RationalExpr>(expr);
        return rationalNode->value();
    }
    
    throw std::invalid_argument("Expression is not numeric.");
}

}  // namespace cas
