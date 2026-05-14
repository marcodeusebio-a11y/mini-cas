//
//  Rational.cpp
//  Mini Symbolic CAS for Rational Functions
//
//  Created by Marco D’Eusebio on 5/14/26.
//

#include "Rational.h"
#include <sstream>
#include <stdexcept>
#include <string>

namespace cas {

std::ostream& operator<<(std::ostream& out, const BigInt& value) {
    out << value.value_;
    return out;
}

std::istream& operator>>(std::istream& in, BigInt& value) {
    in >> value.value_;
    return in;
}

namespace {

BigInt absBigInt(BigInt value) {
    return value < 0 ? -value : value;
}

BigInt gcdBigInt(BigInt a, BigInt b) {
    a = absBigInt(a);
    b = absBigInt(b);
    
    while (b != 0) {
        BigInt r = a % b;
        a = b;
        b = r;
    }
    
    return a == 0 ? BigInt(1) : a;
}

BigInt parseBigInt(const std::string& text) {
    if (text.empty()) {
        throw std::invalid_argument("Empty integer string.");
    }
    
    std::istringstream in(text);
    BigInt value = 0;
    in >> value;
    
    if (!in || !in.eof()) {
        throw std::invalid_argument("Invalid integer string: " + text);
    }
    
    return value;
}

}  // namespace

Rational::Rational() : numerator_(0), denominator_(1) {}

Rational::Rational(long long value) : numerator_(value), denominator_(1) {}

Rational::Rational(const BigInt& numerator) : numerator_(numerator), denominator_(1) {}

Rational::Rational(const BigInt& numerator, const BigInt& denominator)
: numerator_(numerator), denominator_(denominator) {
    normalize();
}

Rational Rational::fromString(const std::string& text) {
    std::size_t slash = text.find('/');
    if (slash == std::string::npos) {
        return Rational(parseBigInt(text));
    }
    
    BigInt num = parseBigInt(text.substr(0, slash));
    BigInt den = parseBigInt(text.substr(slash + 1));
    return Rational(num, den);
}

const BigInt& Rational::numerator() const {
    return numerator_;
}

const BigInt& Rational::denominator() const {
    return denominator_;
}

bool Rational::isZero() const {
    return numerator_ == 0;
}

bool Rational::isOne() const {
    return numerator_ == denominator_;
}

std::string Rational::toString() const {
    std::ostringstream out;
    out << numerator_;
    if (denominator_ != 1) {
        out << "/" << denominator_;
    }
    return out.str();
}

Rational Rational::operator+() const {
    return *this;
}

Rational Rational::operator-() const {
    return Rational(-numerator_, denominator_);
}

Rational& Rational::operator+=(const Rational& other) {
    numerator_ = numerator_ * other.denominator_ + other.numerator_ * denominator_;
    denominator_ *= other.denominator_;
    normalize();
    return *this;
}

Rational& Rational::operator-=(const Rational& other) {
    numerator_ = numerator_ * other.denominator_ - other.numerator_ * denominator_;
    denominator_ *= other.denominator_;
    normalize();
    return *this;
}

Rational& Rational::operator*=(const Rational& other) {
    numerator_ *= other.numerator_;
    denominator_ *= other.denominator_;
    normalize();
    return *this;
}

Rational& Rational::operator/=(const Rational& other) {
    if (other.numerator_ == 0) {
        throw std::domain_error("Division by zero rational.");
    }
    
    numerator_ *= other.denominator_;
    denominator_ *= other.numerator_;
    normalize();
    return *this;
}

void Rational::normalize() {
    if (denominator_ == 0) {
        throw std::domain_error("Rational denominator cannot be zero.");
    }
    
    if (numerator_ == 0) {
        denominator_ = 1;
        return;
    }
    
    if (denominator_ < 0) {
        numerator_ = -numerator_;
        denominator_ = -denominator_;
    }
    
    BigInt g = gcdBigInt(numerator_, denominator_);
    numerator_ /= g;
    denominator_ /= g;
}

Rational operator+(Rational left, const Rational& right) {
    left += right;
    return left;
}

Rational operator-(Rational left, const Rational& right) {
    left -= right;
    return left;
}

Rational operator*(Rational left, const Rational& right) {
    left *= right;
    return left;
}

Rational operator/(Rational left, const Rational& right) {
    left /= right;
    return left;
}

bool operator==(const Rational& left, const Rational& right) {
    return left.numerator() == right.numerator() &&
    left.denominator() == right.denominator();
}

bool operator!=(const Rational& left, const Rational& right) {
    return !(left == right);
}

bool operator<(const Rational& left, const Rational& right) {
    return left.numerator() * right.denominator() <
    right.numerator() * left.denominator();
}

bool operator<=(const Rational& left, const Rational& right) {
    return !(right < left);
}

bool operator>(const Rational& left, const Rational& right) {
    return right < left;
}

bool operator>=(const Rational& left, const Rational& right) {
    return !(left < right);
}

std::ostream& operator<<(std::ostream& out, const Rational& value) {
    out << value.toString();
    return out;
}

}  // namespace cas
