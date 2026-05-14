//
//  Expr.h
//  Mini Symbolic CAS for Rational Functions
//
//  Created by Marco D’Eusebio on 5/14/26.
//

#pragma once

#include "Rational.h"
#include <memory>
#include <string>
#include <vector>

namespace cas {

class Expr;
class IntegerExpr;
class RationalExpr;
class SymbolExpr;
class AddExpr;
class MulExpr;
class PowExpr;

using ExprPtr = std::shared_ptr<const Expr>;

class Expr {
public:
    enum class Kind {
        Integer,
        Rational,
        Symbol,
        Add,
        Mul,
        Pow
    };
    
    virtual ~Expr() = default;
    
    virtual Kind kind() const = 0;
    virtual std::string toString() const = 0;
};

class IntegerExpr final : public Expr {
public:
    explicit IntegerExpr(BigInt value);
    
    Kind kind() const override;
    const BigInt& value() const;
    std::string toString() const override;
    
private:
    BigInt value_;
};

class RationalExpr final : public Expr {
public:
    explicit RationalExpr(Rational value);
    
    Kind kind() const override;
    const Rational& value() const;
    std::string toString() const override;
    
private:
    Rational value_;
};

class SymbolExpr final : public Expr {
public:
    explicit SymbolExpr(std::string name);
    
    Kind kind() const override;
    const std::string& name() const;
    std::string toString() const override;
    
private:
    std::string name_;
};

class AddExpr final : public Expr {
public:
    explicit AddExpr(std::vector<ExprPtr> terms);
    
    Kind kind() const override;
    const std::vector<ExprPtr>& terms() const;
    std::string toString() const override;
    
private:
    std::vector<ExprPtr> terms_;
};

class MulExpr final : public Expr {
public:
    explicit MulExpr(std::vector<ExprPtr> factors);
    
    Kind kind() const override;
    const std::vector<ExprPtr>& factors() const;
    std::string toString() const override;
    
private:
    std::vector<ExprPtr> factors_;
};

class PowExpr final : public Expr {
public:
    PowExpr(ExprPtr base, int exponent);
    
    Kind kind() const override;
    const ExprPtr& base() const;
    int exponent() const;
    std::string toString() const override;
    
private:
    ExprPtr base_;
    int exponent_;
};

ExprPtr makeInteger(long long value);
ExprPtr makeInteger(const BigInt& value);
ExprPtr makeRational(const Rational& value);
ExprPtr makeSymbol(const std::string& name);
ExprPtr makeAdd(const std::vector<ExprPtr>& terms);
ExprPtr makeMul(const std::vector<ExprPtr>& factors);
ExprPtr makePow(const ExprPtr& base, int exponent);
ExprPtr makeNegate(const ExprPtr& expr);

bool isNumericExpr(const ExprPtr& expr);
Rational numericValueOf(const ExprPtr& expr);

}  // namespace cas
