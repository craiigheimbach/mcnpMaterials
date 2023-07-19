#pragma once
#include <string>
#include <sstream>
#include <vector>
#include "logger.h"

//  ***************************************************************************
//  enums for printing
//
//
enum NumFormat { FIXED, SCIENTIFIC, DEFAULT };

inline const std::string& whitespace() { static const std::string ws("\t\n\v\f\r "); return ws; };
std::vector<std::string> split(std::string const& str, unsigned int maxSplits = 0, const std::string& delim = whitespace());
std::string ltrim(const std::string& str);
std::string rtrim(const std::string& str);
std::string trim(const std::string& str);
bool hasWhitespace(const std::string& str);
std::string truncate(int length, const std::string& str);

//std::ifstream openFileRead(const std::string& fileName);
std::string removeComments(const std::string& str);
std::string toLower(const std::string& str);
bool matchAny(const std::string& lookFor, const std::vector<std::string>& container);

std::string convertNullToEmpty(const std::string& str);
bool myLess(const std::string& lhs, const std::string& rhs);	// for sorting

