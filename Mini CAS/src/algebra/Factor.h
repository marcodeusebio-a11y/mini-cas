//
//  Factor.h
//  Mini Symbolic CAS for Rational Functions
//
//  Created by Marco D’Eusebio on 5/14/26.
//

#pragma once

#include "Polynomial.h"
#include "Rational.h"
#include <optional>
#include <string>
#include <vector>

namespace cas {

struct LinearFactor {
    Rational root;
    int multiplicity = 0;
};

struct ExactFactorizationResult {
    bool ok = false;
    std::string message;
    Rational scale;
    std::vector<LinearFactor> linearFactors;
    Polynomial leftover;
};

Polynomial makeMonic(const Polynomial& polynomial);
Polynomial gcd(Polynomial left, Polynomial right);
std::vector<std::pair<Polynomial, int>> squareFreeFactorization(const Polynomial& polynomial);

std::optional<Rational> findRationalRoot(const Polynomial& polynomial);
ExactFactorizationResult factorOverQ(const Polynomial& polynomial);

std::string linearFactorToString(const Rational& root);
std::string factorizationToString(const ExactFactorizationResult& factorization);

}  // namespace cas
