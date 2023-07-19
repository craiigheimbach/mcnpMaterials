#pragma once

#include <optional>

using nDbl = std::optional <double>;          // nullable double
using okDbl = std::pair <bool, nDbl>;         // If input is nonsense, bool=false and nDbl is nullopt.          
using nLong = std::optional <long>;           // nullable long
using okLong = std::pair <bool, nLong>;       // If input is nonsense, bool=false and nLong is nullopt.          


okDbl toDoubleNullable(const std::string& str, bool showMsg = true);
okLong toLongNullable(const std::string& str, bool showMsg = true);

std::string to_string(const nDbl& dbl);
std::string to_string(nLong lng);

inline double dabs(double a) { return (a >= 0.) ? a : -a; }; // linux vs windows
const double smallFloat = 1.0E-40;

//  ***************************************************************************
inline nDbl noNeg(nDbl num)
{
    if (num.has_value() && num < 0.) return std::nullopt;
    return num;
}

//  ***************************************************************************
inline nDbl operator+ (const nDbl& lhs, const nDbl& rhs)
{
    if (!lhs.has_value() || !rhs.has_value()) return std::nullopt;
    return lhs.value() + rhs.value(); 
}
 
//  ***************************************************************************
inline nDbl operator+ (const nDbl& lhs, double rhs)
{
    if (!lhs.has_value()) return std::nullopt;
    return lhs.value() + rhs;
}

//  ***************************************************************************
inline nDbl operator+ (double lhs, const nDbl& rhs)
{
    if (!rhs.has_value()) return std::nullopt;
    return rhs.value() + lhs;
}

//  ***************************************************************************
inline nDbl operator- (const nDbl& lhs, const nDbl& rhs)
{
    if (!lhs.has_value() || !rhs.has_value()) return std::nullopt;
    return lhs.value() - rhs.value();
}

//  ***************************************************************************
inline nDbl operator- (const nDbl& lhs, double rhs)
{
    if (!lhs.has_value()) return std::nullopt;
    return lhs.value() - rhs;
}

//  ***************************************************************************
inline nDbl operator- (double lhs, const nDbl& rhs)
{
    if (!rhs.has_value()) return std::nullopt;
    return lhs - rhs.value();
}

//  ***************************************************************************
inline nDbl operator* (const nDbl& lhs, const nDbl& rhs)
{
    if (!lhs.has_value() || !rhs.has_value()) return std::nullopt;
    return lhs.value() * rhs.value();
}

//  ***************************************************************************
inline nDbl operator* (const nDbl& lhs, double rhs)
{
    if (!lhs.has_value()) return std::nullopt;
    return lhs.value() * rhs;
}

//  ***************************************************************************
inline nDbl operator* (double lhs, const nDbl& rhs)
{
    if (!rhs.has_value()) return std::nullopt;
    return lhs * rhs.value();
}

//  ***************************************************************************
inline nDbl operator/ (const nDbl& lhs, const nDbl& rhs)
{
    if (!lhs.has_value() || !rhs.has_value()) return std::nullopt;
     return lhs.value() / rhs.value();           // May return inf or NaN
}

//  ***************************************************************************
inline nDbl operator/ (const nDbl& lhs, double rhs)
{
    if (!lhs.has_value()) return std::nullopt;
    return lhs.value() / rhs;
}

//  ***************************************************************************
inline nDbl operator/ (double lhs, const nDbl& rhs)
{
    if (!rhs.has_value()) return std::nullopt;
    return lhs / rhs.value();
}

//  ***************************************************************************
inline nDbl abs(nDbl zz)
{
    if (!zz.has_value()) return std::nullopt;
    return abs(zz.value());
}

//  ***************************************************************************
bool match(nDbl a, nDbl b);
void setPrecisionForMatching(double err);
double getPrecisionForMatching();

//  ***************************************************************************
inline std::ostream& operator<<(std::ostream& os, const nDbl& zz)
{
    if (zz.has_value()) os << zz.value();
    else os << "  -  ";
    return os;
}

