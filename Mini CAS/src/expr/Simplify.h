//
//  Simplify.h
//  Mini Symbolic CAS for Rational Functions
//
//  Created by Marco D’Eusebio on 5/14/26.
//

#pragma once

#include "Expr.h"
#include "Polynomial.h"
#include <optional>
#include <string>

namespace cas {

ExprPtr simplify(const ExprPtr& expr);

bool isPolynomialIn(const ExprPtr& expr, const std::string& variableName = "x");
std::optional<Polynomial> tryConvertToPolynomial(const ExprPtr& expr,
                                                 const std::string& variableName = "x");

}  // namespace cas
