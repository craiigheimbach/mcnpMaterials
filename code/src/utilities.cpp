#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <iomanip>
#include <climits>

#include "utilities.h"

//  ***************************************************************************
//  split a string into substrings using a delimiter.
//  If maxSplits == 1, can have no more than two pieces.
//
std::vector<std::string> split(std::string const& str, unsigned int maxSplits, const std::string& delim)
{
    std::vector<std::string> pieces;
    size_t start;
    size_t end = 0;
    // Position to next non-delimiter
    while ((start = str.find_first_not_of(delim, end)) != std::string::npos)
    {
        end = str.find_first_of(delim, start);
        pieces.push_back(str.substr(start, end - start));
        if (pieces.size() == maxSplits)                     // if maxSplits==0, do not stop.
        {
            start = str.find_first_not_of(delim, end);
            if (start == std::string::npos) break;
            pieces.push_back(str.substr(start, std::string::npos));
            break;
        }
    }
    return pieces;
}


//  ***************************************************************************
//  remove characters (usually whitespace) from left side of string
//
std::string ltrim(const std::string& str)
{
    std::string cpy = str;
    cpy.erase(0, str.find_first_not_of(whitespace()));
    return cpy;
}

//  ***************************************************************************
//  remove characters (usually whitespace, including return) from right side of string
//
std::string rtrim(const std::string& str)
{
    std::string cpy = str;
    cpy.erase(cpy.find_last_not_of(whitespace()) + 1);
    return cpy;
}

//  ***************************************************************************
std::string trim(const std::string& str)
{
    return ltrim(rtrim(str));
}

//  ***************************************************************************
bool hasWhitespace(const std::string& str)
{
    size_t idx = str.find_first_of(whitespace());
    return (idx == std::string::npos) ? false : true;
}

//  ***************************************************************************
//  Remove anything after '//' that is not in quotation marks.
//
std::string removeComments(const std::string& str)    
{
    bool inQuotes = false;
    bool foundSlash = false;

    std::string newStr = "";
    for (auto ch: str)
    {
        if (ch == '\"')
        {
            inQuotes = !inQuotes;
            foundSlash = false;
        }
        if (!inQuotes)
        {
            if (ch == '/')
            {
                if (foundSlash == true)             // Have already added the first '/'. 
                    return trim(newStr.substr(0, newStr.size() - 1));
                else
                {
                    foundSlash = true;
                }
            }
            else
            {
                foundSlash = false;
            }
        }
        newStr += ch;
    }
    return trim(newStr);        // Removes spaces between end of normal test and '//'
}

//  ***************************************************************************
std::string toLower(const std::string& str)
{
    std::string newString = str;
    for (char& chr: newString)
    {
        chr = std::tolower(chr);
    }
    return newString;
}

//  ***************************************************************************
std::string convertNullToEmpty(const std::string& str)
{
    std::string wkStr = trim(str);
    if (wkStr == "-" || wkStr == "none" || (wkStr == "None") || wkStr == "Null" || (wkStr == "null"))
    {
        return "";
    }
    else return wkStr;
}

//  ***************************************************************************
bool isStringUInt(const std::string& txt)
{
    char* pEnd;

    const char* psp = txt.c_str() + txt.size();
    unsigned long ul = std::strtoul(txt.c_str(), &pEnd, 0);
    if (psp != pEnd || ul > UINT_MAX)
    {
        return false;
    }
    return true;
}

//  ***************************************************************************
//  Used in sorting. Lesser strings come first.
//
int lesserString(const std::string& a, const std::string& b)
{
    if (a.empty() && b.empty()) return 0;
    if (a.empty() && !b.empty()) return 1;
    if (!a.empty() && b.empty()) return -1;
    return a.compare(b);
}

//  ***************************************************************************
//  Used in sorting vectors
//  e.g. (H-1, 1001) vs (H-2, 1002)
//
bool lesser(const std::vector<std::string>& a, const std::vector < std::string>& b)
{
    for (size_t ia = 0; ia < a.size(); ++ia)
    {
        auto lower = lesserString(a[ia], b[ia]);
        if (lower < 0) return true;
        if (lower > 0) return false;
    }
    return false;
}

//  ***************************************************************************
//  Defines my preferences in sorting.
//  Return true iff lhs < rhs 
//
bool myLess(const std::string& lhs, const std::string& rhs)
{
    //  Initial screen
    if (lhs == rhs) return false;           // Equals means neither is less.
    if (lhs.empty()) return false;          // Empty strings to the end
    if (rhs.empty()) return true;           // Empty strings to the end
    //  Spaces to the end of the list.
    bool leftSpaces = std::all_of(lhs.begin(), lhs.end(), isspace); // All lhs is spaces?
    bool rightSpaces = std::all_of(rhs.begin(), rhs.end(), isspace);// All rhs is spaces?
    if (leftSpaces && !rightSpaces) return false;                   // just lhs
    if (rightSpaces && !leftSpaces) return true;                    // just rhs
    if (rightSpaces && leftSpaces) return (lhs.size() < rhs.size());// shorter goes first

    // Numbers are less than text (includes both doubles and non-doubles)
    auto leftDbl = toDoubleNullable(lhs, false);
    bool isLeftANumber = leftDbl.first;
    auto rightDbl = toDoubleNullable(rhs, false);
    bool isRightANumber = rightDbl.first;
    if (isLeftANumber && isRightANumber)  return leftDbl.second < rightDbl.second;
    if (isLeftANumber) return true;
    if (isRightANumber) return false;
    // All other text
    return toLower(lhs) < toLower(rhs);
}

//  ***************************************************************************
//  e.g.  "LongString" -> "longs..." if "longString" does not fit
std::string truncate(int length, const std::string& str)
{
    if (length < 0) return "";
    if (str.size() <= (size_t)length) return str;
    if (length < 3) return std::string(length, '.'); //str.substr(0, length);
    else return str.substr(0, length - 3) + "...";
}

//  ***************************************************************************
bool matchAny(const std::string& lookFor, const std::vector<std::string>& container)
{
    for (const std::string& test : container)
    {
        if (lookFor != test) continue;
        return true;
    }
    return false;
}