//
//  Simplify.cpp
//  Mini Symbolic CAS for Rational Functions
//
//  Created by Marco D’Eusebio on 5/14/26.
//

#include "Simplify.h"
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace cas {
namespace {

Rational powRational(Rational base, int exponent) {
    if (exponent < 0) {
        throw std::invalid_argument("Negative exponents are not supported.");
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

struct CoefficientAndBase {
    Rational coefficient;
    ExprPtr base;
};

struct BasePowerGroup {
    std::string key;
    ExprPtr base;
    int exponent = 0;
};

CoefficientAndBase splitCoefficient(const ExprPtr& expr) {
    if (isNumericExpr(expr)) {
        return {numericValueOf(expr), nullptr};
    }
    
    if (expr->kind() == Expr::Kind::Mul) {
        const auto mulNode = std::dynamic_pointer_cast<const MulExpr>(expr);
        const std::vector<ExprPtr>& factors = mulNode->factors();
        
        if (!factors.empty() && isNumericExpr(factors.front())) {
            Rational coeff = numericValueOf(factors.front());
            
            if (factors.size() == 2) {
                return {coeff, factors[1]};
            }
            
            std::vector<ExprPtr> rest;
            for (std::size_t i = 1; i < factors.size(); ++i) {
                rest.push_back(factors[i]);
            }
            
            return {coeff, makeMul(rest)};
        }
    }
    
    return {Rational(1), expr};
}

ExprPtr rationalToExpr(const Rational& value) {
    return makeRational(value);
}

ExprPtr buildScaledTerm(const Rational& coefficient, const ExprPtr& base) {
    if (base == nullptr) {
        return rationalToExpr(coefficient);
    }
    
    if (coefficient == Rational(1)) {
        return base;
    }
    
    return makeMul({rationalToExpr(coefficient), base});
}

ExprPtr simplifyPowExpr(const std::shared_ptr<const PowExpr>& powNode);
ExprPtr simplifyAddExpr(const std::shared_ptr<const AddExpr>& addNode);
ExprPtr simplifyMulExpr(const std::shared_ptr<const MulExpr>& mulNode);

ExprPtr simplifyIntegerOrRational(const ExprPtr& expr) {
    if (expr->kind() == Expr::Kind::Integer) {
        const auto integerNode = std::dynamic_pointer_cast<const IntegerExpr>(expr);
        return makeInteger(integerNode->value());
    }
    
    const auto rationalNode = std::dynamic_pointer_cast<const RationalExpr>(expr);
    return rationalToExpr(rationalNode->value());
}

ExprPtr simplifyPowExpr(const std::shared_ptr<const PowExpr>& powNode) {
    ExprPtr simplifiedBase = simplify(powNode->base());
    int exponent = powNode->exponent();
    
    if (exponent == 0) {
        return makeInteger(1);
    }
    if (exponent == 1) {
        return simplifiedBase;
    }
    
    if (isNumericExpr(simplifiedBase)) {
        Rational value = numericValueOf(simplifiedBase);
        return rationalToExpr(powRational(value, exponent));
    }
    
    if (simplifiedBase->kind() == Expr::Kind::Pow) {
        const auto nestedPow = std::dynamic_pointer_cast<const PowExpr>(simplifiedBase);
        return makePow(nestedPow->base(), nestedPow->exponent() * exponent);
    }
    
    return makePow(simplifiedBase, exponent);
}

ExprPtr simplifyAddExpr(const std::shared_ptr<const AddExpr>& addNode) {
    std::vector<ExprPtr> flatTerms;
    
    for (const ExprPtr& term : addNode->terms()) {
        ExprPtr simplified = simplify(term);
        
        if (simplified->kind() == Expr::Kind::Add) {
            const auto nestedAdd = std::dynamic_pointer_cast<const AddExpr>(simplified);
            for (const ExprPtr& nested : nestedAdd->terms()) {
                flatTerms.push_back(nested);
            }
        } else {
            flatTerms.push_back(simplified);
        }
    }
    
    Rational constantSum(0);
    std::vector<std::string> order;
    std::vector<CoefficientAndBase> grouped;
    
    for (const ExprPtr& term : flatTerms) {
        if (isNumericExpr(term)) {
            constantSum += numericValueOf(term);
            continue;
        }
        
        CoefficientAndBase split = splitCoefficient(term);
        std::string key = split.base->toString();
        
        bool found = false;
        for (CoefficientAndBase& entry : grouped) {
            if (entry.base->toString() == key) {
                entry.coefficient += split.coefficient;
                found = true;
                break;
            }
        }
        
        if (!found) {
            grouped.push_back({split.coefficient, split.base});
            order.push_back(key);
        }
    }
    
    std::vector<ExprPtr> outTerms;
    
    if (!constantSum.isZero()) {
        outTerms.push_back(rationalToExpr(constantSum));
    }
    
    for (const CoefficientAndBase& entry : grouped) {
        if (entry.coefficient.isZero()) {
            continue;
        }
        outTerms.push_back(buildScaledTerm(entry.coefficient, entry.base));
    }
    
    if (outTerms.empty()) {
        return makeInteger(0);
    }
    if (outTerms.size() == 1) {
        return outTerms.front();
    }
    
    return makeAdd(outTerms);
}

ExprPtr simplifyMulExpr(const std::shared_ptr<const MulExpr>& mulNode) {
    std::vector<ExprPtr> flatFactors;
    
    for (const ExprPtr& factor : mulNode->factors()) {
        ExprPtr simplified = simplify(factor);
        
        if (simplified->kind() == Expr::Kind::Mul) {
            const auto nestedMul = std::dynamic_pointer_cast<const MulExpr>(simplified);
            for (const ExprPtr& nested : nestedMul->factors()) {
                flatFactors.push_back(nested);
            }
        } else {
            flatFactors.push_back(simplified);
        }
    }
    
    Rational constantProduct(1);
    std::vector<BasePowerGroup> groups;
    
    for (const ExprPtr& factor : flatFactors) {
        if (isNumericExpr(factor)) {
            constantProduct *= numericValueOf(factor);
            if (constantProduct.isZero()) {
                return makeInteger(0);
            }
            continue;
        }
        
        ExprPtr base = factor;
        int exponent = 1;
        
        if (factor->kind() == Expr::Kind::Pow) {
            const auto powNode = std::dynamic_pointer_cast<const PowExpr>(factor);
            base = powNode->base();
            exponent = powNode->exponent();
        }
        
        std::string key = base->toString();
        bool found = false;
        
        for (BasePowerGroup& group : groups) {
            if (group.key == key) {
                group.exponent += exponent;
                found = true;
                break;
            }
        }
        
        if (!found) {
            groups.push_back({key, base, exponent});
        }
    }
    
    std::vector<ExprPtr> outFactors;
    
    if (!constantProduct.isZero()) {
        if (!(constantProduct == Rational(1)) || groups.empty()) {
            outFactors.push_back(rationalToExpr(constantProduct));
        }
    }
    
    for (const BasePowerGroup& group : groups) {
        if (group.exponent == 1) {
            outFactors.push_back(group.base);
        } else {
            outFactors.push_back(makePow(group.base, group.exponent));
        }
    }
    
    if (outFactors.empty()) {
        return makeInteger(1);
    }
    if (outFactors.size() == 1) {
        return outFactors.front();
    }
    
    return makeMul(outFactors);
}

}  // namespace

ExprPtr simplify(const ExprPtr& expr) {
    switch (expr->kind()) {
        case Expr::Kind::Integer:
        case Expr::Kind::Rational:
            return simplifyIntegerOrRational(expr);
            
        case Expr::Kind::Symbol: {
            const auto symbolNode = std::dynamic_pointer_cast<const SymbolExpr>(expr);
            return makeSymbol(symbolNode->name());
        }
            
        case Expr::Kind::Add:
            return simplifyAddExpr(std::dynamic_pointer_cast<const AddExpr>(expr));
            
        case Expr::Kind::Mul:
            return simplifyMulExpr(std::dynamic_pointer_cast<const MulExpr>(expr));
            
        case Expr::Kind::Pow:
            return simplifyPowExpr(std::dynamic_pointer_cast<const PowExpr>(expr));
    }
    
    throw std::runtime_error("Unknown expression kind.");
}

std::optional<Polynomial> tryConvertToPolynomial(const ExprPtr& expr,
                                                 const std::string& variableName) {
    ExprPtr simplified = simplify(expr);
    
    switch (simplified->kind()) {
        case Expr::Kind::Integer: {
            const auto integerNode = std::dynamic_pointer_cast<const IntegerExpr>(simplified);
            return Polynomial::constant(Rational(integerNode->value()));
        }
            
        case Expr::Kind::Rational: {
            const auto rationalNode = std::dynamic_pointer_cast<const RationalExpr>(simplified);
            return Polynomial::constant(rationalNode->value());
        }
            
        case Expr::Kind::Symbol: {
            const auto symbolNode = std::dynamic_pointer_cast<const SymbolExpr>(simplified);
            if (symbolNode->name() == variableName) {
                return Polynomial::variable();
            }
            return std::nullopt;
        }
            
        case Expr::Kind::Add: {
            const auto addNode = std::dynamic_pointer_cast<const AddExpr>(simplified);
            Polynomial result = Polynomial::constant(Rational(0));
            
            for (const ExprPtr& term : addNode->terms()) {
                std::optional<Polynomial> polyTerm = tryConvertToPolynomial(term, variableName);
                if (!polyTerm.has_value()) {
                    return std::nullopt;
                }
                result = Polynomial::add(result, polyTerm.value());
            }
            
            return result;
        }
            
        case Expr::Kind::Mul: {
            const auto mulNode = std::dynamic_pointer_cast<const MulExpr>(simplified);
            Polynomial result = Polynomial::constant(Rational(1));
            
            for (const ExprPtr& factor : mulNode->factors()) {
                std::optional<Polynomial> polyFactor = tryConvertToPolynomial(factor, variableName);
                if (!polyFactor.has_value()) {
                    return std::nullopt;
                }
                result = Polynomial::multiply(result, polyFactor.value());
            }
            
            return result;
        }
            
        case Expr::Kind::Pow: {
            const auto powNode = std::dynamic_pointer_cast<const PowExpr>(simplified);
            std::optional<Polynomial> basePoly = tryConvertToPolynomial(powNode->base(), variableName);
            if (!basePoly.has_value()) {
                return std::nullopt;
            }
            return Polynomial::power(basePoly.value(), powNode->exponent());
        }
    }
    
    return std::nullopt;
}

bool isPolynomialIn(const ExprPtr& expr, const std::string& variableName) {
    return tryConvertToPolynomial(expr, variableName).has_value();
}

}  // namespace cas
