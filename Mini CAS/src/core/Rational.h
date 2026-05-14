//
//  Rational.h
//  Mini Symbolic CAS for Rational Functions
//
//  Created by Marco D’Eusebio on 5/14/26.
//

#pragma once

#include <iosfwd>
#include <limits>
#include <stdexcept>
#include <string>

namespace cas {

class BigInt {
public:
    BigInt() = default;
    BigInt(long long value) : value_(value) {}
    explicit BigInt(const std::string& text) {
        std::size_t parsedLength = 0;
        value_ = std::stoll(text, &parsedLength);
        if (parsedLength != text.size()) {
            throw std::invalid_argument("Invalid integer string: " + text);
        }
    }
    
    template <typename T>
    T convert_to() const {
        if (value_ < static_cast<long long>(std::numeric_limits<T>::min()) ||
            value_ > static_cast<long long>(std::numeric_limits<T>::max())) {
            throw std::overflow_error("Integer conversion out of range.");
        }
        return static_cast<T>(value_);
    }
    
    BigInt operator+() const { return *this; }
    BigInt operator-() const { return BigInt(-value_); }
    
    BigInt& operator+=(const BigInt& other) { value_ += other.value_; return *this; }
    BigInt& operator-=(const BigInt& other) { value_ -= other.value_; return *this; }
    BigInt& operator*=(const BigInt& other) { value_ *= other.value_; return *this; }
    BigInt& operator/=(const BigInt& other) { value_ /= other.value_; return *this; }
    BigInt& operator%=(const BigInt& other) { value_ %= other.value_; return *this; }
    
    friend BigInt operator+(BigInt left, const BigInt& right) { left += right; return left; }
    friend BigInt operator-(BigInt left, const BigInt& right) { left -= right; return left; }
    friend BigInt operator*(BigInt left, const BigInt& right) { left *= right; return left; }
    friend BigInt operator/(BigInt left, const BigInt& right) { left /= right; return left; }
    friend BigInt operator%(BigInt left, const BigInt& right) { left %= right; return left; }
    
    friend bool operator==(const BigInt& left, const BigInt& right) { return left.value_ == right.value_; }
    friend bool operator!=(const BigInt& left, const BigInt& right) { return !(left == right); }
    friend bool operator<(const BigInt& left, const BigInt& right) { return left.value_ < right.value_; }
    friend bool operator<=(const BigInt& left, const BigInt& right) { return !(right < left); }
    friend bool operator>(const BigInt& left, const BigInt& right) { return right < left; }
    friend bool operator>=(const BigInt& left, const BigInt& right) { return !(left < right); }
    
    friend std::ostream& operator<<(std::ostream& out, const BigInt& value);
    friend std::istream& operator>>(std::istream& in, BigInt& value);
    
private:
    long long value_ = 0;
};

class Rational {
public:
    Rational();
    Rational(long long value);
    Rational(const BigInt& numerator);
    Rational(const BigInt& numerator, const BigInt& denominator);
    
    static Rational fromString(const std::string& text);
    
    const BigInt& numerator() const;
    const BigInt& denominator() const;
    
    bool isZero() const;
    bool isOne() const;
    
    std::string toString() const;
    
    Rational operator+() const;
    Rational operator-() const;
    
    Rational& operator+=(const Rational& other);
    Rational& operator-=(const Rational& other);
    Rational& operator*=(const Rational& other);
    Rational& operator/=(const Rational& other);
    
private:
    BigInt numerator_;
    BigInt denominator_;
    
    void normalize();
};

Rational operator+(Rational left, const Rational& right);
Rational operator-(Rational left, const Rational& right);
Rational operator*(Rational left, const Rational& right);
Rational operator/(Rational left, const Rational& right);

bool operator==(const Rational& left, const Rational& right);
bool operator!=(const Rational& left, const Rational& right);
bool operator<(const Rational& left, const Rational& right);
bool operator<=(const Rational& left, const Rational& right);
bool operator>(const Rational& left, const Rational& right);
bool operator>=(const Rational& left, const Rational& right);

std::ostream& operator<<(std::ostream& out, const Rational& value);

}  // namespace cas
