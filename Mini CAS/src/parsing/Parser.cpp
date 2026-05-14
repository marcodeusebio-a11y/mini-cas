//
//  Parser.cpp
//  Mini Symbolic CAS for Rational Functions
//
//  Created by Marco D’Eusebio on 5/14/26.
//

#include "Parser.h"
#include "Rational.h"
#include "Simplify.h"
#include <cctype>
#include <stdexcept>
#include <string>
#include <utility>

namespace cas {
namespace {

class ParserImpl {
public:
    explicit ParserImpl(std::string input) : input_(removeSpaces(std::move(input))) {}
    
    ExprPtr parse() {
        ExprPtr result = parseExpression();
        if (!atEnd()) {
            throw std::runtime_error("Unexpected input near: " + input_.substr(position_));
        }
        return result;
    }
    
private:
    std::string input_;
    std::size_t position_ = 0;
    
    static std::string removeSpaces(std::string text) {
        std::string out;
        out.reserve(text.size());
        
        for (char ch : text) {
            if (!std::isspace(static_cast<unsigned char>(ch))) {
                out.push_back(ch);
            }
        }
        
        return out;
    }
    
    bool atEnd() const {
        return position_ >= input_.size();
    }
    
    char peek() const {
        return atEnd() ? '\0' : input_[position_];
    }
    
    char peekNext() const {
        return position_ + 1 < input_.size() ? input_[position_ + 1] : '\0';
    }
    
    bool match(char expected) {
        if (peek() == expected) {
            ++position_;
            return true;
        }
        return false;
    }
    
    static bool beginsPrimary(char ch) {
        return std::isdigit(static_cast<unsigned char>(ch)) ||
        std::isalpha(static_cast<unsigned char>(ch)) ||
        ch == '(';
    }
    
    ExprPtr parseExpression() {
        ExprPtr left = parseTerm();
        
        while (true) {
            if (match('+')) {
                left = makeAdd({left, parseTerm()});
            } else if (match('-')) {
                left = makeAdd({left, makeNegate(parseTerm())});
            } else {
                break;
            }
        }
        
        return left;
    }
    
    ExprPtr parseTerm() {
        ExprPtr left = parseFactor();
        
        while (true) {
            if (match('*')) {
                left = makeMul({left, parseFactor()});
                continue;
            }
            
            if (match('/')) {
                ExprPtr divisor = parseFactor();
                ExprPtr simplifiedDivisor = simplify(divisor);
                
                if (!isNumericExpr(simplifiedDivisor)) {
                    throw std::runtime_error(
                                             "Only division by exact numeric expressions is supported in this phase.");
                }
                
                Rational divisorValue = numericValueOf(simplifiedDivisor);
                if (divisorValue.isZero()) {
                    throw std::runtime_error("Division by zero.");
                }
                
                left = makeMul({left, makeRational(Rational(1) / divisorValue)});
                continue;
            }
            
            if (beginsPrimary(peek())) {
                left = makeMul({left, parseFactor()});
                continue;
            }
            
            break;
        }
        
        return left;
    }
    
    ExprPtr parseFactor() {
        ExprPtr base = parsePrimary();
        
        if (match('^')) {
            int exponent = parseNonNegativeInteger();
            return makePow(base, exponent);
        }
        
        return base;
    }
    
    ExprPtr parsePrimary() {
        if (match('+')) {
            return parsePrimary();
        }
        
        if (match('-')) {
            return makeNegate(parsePrimary());
        }
        
        if (match('(')) {
            ExprPtr inside = parseExpression();
            if (!match(')')) {
                throw std::runtime_error("Missing closing ')'.");
            }
            return inside;
        }
        
        if (std::isdigit(static_cast<unsigned char>(peek()))) {
            return parseNumericLiteral();
        }
        
        if (std::isalpha(static_cast<unsigned char>(peek()))) {
            return parseSymbol();
        }
        
        throw std::runtime_error("Unexpected token near: " + input_.substr(position_));
    }
    
    ExprPtr parseNumericLiteral() {
        std::string leftDigits = parseUnsignedIntegerString();
        
        if (peek() == '/' && std::isdigit(static_cast<unsigned char>(peekNext()))) {
            ++position_;
            std::string rightDigits = parseUnsignedIntegerString();
            return makeRational(Rational::fromString(leftDigits + "/" + rightDigits));
        }
        
        return makeInteger(BigInt(leftDigits));
    }
    
    ExprPtr parseSymbol() {
        std::size_t start = position_;
        
        while (!atEnd() &&
               (std::isalnum(static_cast<unsigned char>(peek())) || peek() == '_')) {
            ++position_;
        }
        
        return makeSymbol(input_.substr(start, position_ - start));
    }
    
    std::string parseUnsignedIntegerString() {
        if (!std::isdigit(static_cast<unsigned char>(peek()))) {
            throw std::runtime_error("Expected an integer.");
        }
        
        std::size_t start = position_;
        while (!atEnd() && std::isdigit(static_cast<unsigned char>(peek()))) {
            ++position_;
        }
        
        return input_.substr(start, position_ - start);
    }
    
    int parseNonNegativeInteger() {
        std::string text = parseUnsignedIntegerString();
        return std::stoi(text);
    }
};

}  // namespace

ExprPtr parseExpressionFromString(const std::string& input) {
    ParserImpl parser(input);
    return simplify(parser.parse());
}

}  // namespace cas
