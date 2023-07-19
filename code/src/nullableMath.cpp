#include <string>
#include "utilities.h"
#include "nullableMath.h"

extern Logger lg;
double allowedError = 0.0001;			// relative error percentage used for matching

// ***************************************************************************
std::string to_string(const nDbl& dbl)
{
    if (dbl.has_value()) return std::to_string(dbl.value());
    else return "-";
}


// ***************************************************************************
std::string to_string(nLong lng)
{
    if (lng.has_value()) return std::to_string(lng.value());
    else  return "-";
}

// ***************************************************************************
okDbl toDoubleNullable(const std::string& str, bool showMsg)
{
    std::size_t nextChar;
    okDbl result{ false, std::nullopt};
    std::string wkStr = convertNullToEmpty(str);
    if (wkStr.empty()) return { true, std::nullopt };
    // Sanity check to avoid some string to double (stod) exceptions
    char first = wkStr[0];
    if (!isdigit(first) && first != '-' && first != '+' && first != '.')
    {
        if (showMsg) lg.bump(2) << ERR << "Cannot convert " << str << " to double.\n";
        return okDbl(false, std::nullopt);
    }
    try
    {
        double dbl = stod(wkStr, &nextChar);
        result =  okDbl(true, dbl);
    }
    catch (...)
    {
        if (showMsg) lg.bump(3) << ERR << "toDouble Invalid argument: " + wkStr + '\n';
        return okDbl(false, std::nullopt);
    }
    if (nextChar != wkStr.size())
    {
        if (showMsg) lg.bump(3) << ERR << "toDouble Invalid argument: " + wkStr + '\n';
        return okDbl(false, std::nullopt);
    }
    return result;
}


//  ***************************************************************************
std::pair<bool, nLong> toLongNullable(const std::string& str, bool showMsg)
{
    showMsg = false;

    std::size_t nextChar;
    long lng;
    bool negative = false;      // keep track of sign

    std::string wkStr = convertNullToEmpty(str);
    if (wkStr.empty()) return { true, std::nullopt };       // nullopt is ok
    char first = wkStr[0];
    switch (first)
    {
    case '-':
        negative = true;
        [[fallthrough]];
    case '+':
        wkStr = wkStr.substr(1);
        break;
    }
    for (const char& chr : wkStr)          // try to avoid exceptions
    {
        if (!isdigit(chr))
        {
            if (showMsg)
            {
                lg.bump(3) << ERR << "Long must be all digits except for leading +- ";
                lg << str << '\n';
            }
            return { false, std::nullopt };
        }
    }
    try
    {
        lng = stol(wkStr, &nextChar);
    }
    catch (...)
    {
        if (showMsg) lg.bump(3) << ERR << "Error in converting string to long " << str << '\n';
        return { false, std::nullopt };
    }
    if (nextChar != wkStr.size())
    {
        if (showMsg) lg.bump(3) << ERR << "Error in converting string to long " << str << '\n';
        return { false, std::nullopt };
    }
    if (negative)
    {
        lng = -lng;
    }
    return std::make_pair(true, lng);
}

//  ***************************************************************************
//  err is a relative error
//
bool match(nDbl a, nDbl b)
{
    if (!a.has_value() && !b.has_value()) return true;  // Matching nullopt
    if (a.has_value() != b.has_value()) return false; 
    double aVal = a.value();
    double bVal = b.value();

    if (dabs(aVal) < smallFloat && dabs(bVal) < smallFloat) return true;
    double delta = dabs(aVal - bVal);
    double average = dabs((aVal + bVal) / 2.);
    if (delta < average * allowedError) return true;
    return false;
}

//  ***************************************************************************
void setPrecisionForMatching(double err)
{
    if (err < 1.e-20) allowedError = 1.e-20;
    else if (err > 0.9) allowedError = 0.9;
    else allowedError = err;
}

//  ***************************************************************************
double getPrecisionForMatching()
{
    return allowedError;
}