//
//  Parser.h
//  Mini Symbolic CAS for Rational Functions
//
//  Created by Marco D’Eusebio on 5/14/26.
//

#pragma once

#include "Expr.h"
#include <string>

namespace cas {

ExprPtr parseExpressionFromString(const std::string& input);

}  // namespace cas
