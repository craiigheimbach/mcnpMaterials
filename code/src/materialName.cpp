#include <algorithm>
#include "materials.h"
#include "logger.h"
#include "utilities.h"

extern Logger lg;


//  ***************************************************************************
//  Do not name a material the same as a key word.
//
bool Material3Names::isValidName(const std::string& name)
{
	static const std::vector <std::string> forbiddenNames{"long", "longname",
	"basic", "is", "isBasic", "short", "shortname", "comment", 
	"zzaaa", "mass", "atom", "atomic", "neutron", "photon", "isotope" };

	std::string wkStr = convertNullToEmpty(name);
	if (wkStr.empty()) return true;
	if (hasWhitespace(wkStr))			//  No whitespace allowed
	{
		lg.bump(3) << ERR << "Material name must not have whitespace: " << name << '\n';
		return false;
	}
	//
	//  String must have a leading alphabetic character. zzaaa can not.
	// 
	if (!isalpha(name[0]))
	{
		lg.bump(3) << ERR << "Material name must start with alphabetic character: " << name << '\n';
		return false;
	}
	//
	//  Initial letter may be capital or small
	//
	auto lowName = name;
	lowName[0] = tolower(lowName[0]);
	if (std::find(forbiddenNames.begin(), forbiddenNames.end(), lowName) != forbiddenNames.end())
	{
		return false;
	}
	return true;
}

//  ***************************************************************************
bool Material3Names::setLongName(const std::string& name)
{
	if (!longName.empty())			// Overwriting current name allowed. Give a warning.
	{
		lg.bump(2) << ERR << "Overwriting current longName " << longName << '\n';
	}
	std::string wkStr = convertNullToEmpty(name);
	if (!isValidName(wkStr))
	{
		longName = "";
		return false;
	}
	longName = wkStr; 
	return true;

}

//  ***************************************************************************
bool Material3Names::setShortName(const std::string& name)
{
	if (!shortName.empty())			// Overwriting current name allowed. Give a warning.
	{
		lg.bump(2) << ERR << "Overwriting current shortName " << shortName << '\n';
	}
	std::string wkStr = convertNullToEmpty(name);
	if (!isValidName(wkStr))
	{
		shortName = "";
		return false;
	}
	shortName = wkStr;
	return true;
}

//  ***************************************************************************
bool Material3Names::isValidZZAAA(const std::string& za, bool showMsg)
{
	auto wkStr = trim(za);
	//if (hasWhitespace(wkStr))
	//{
	//	lg.bump(3) << ERR << "ZZAAA cannot have whitespace: " << za << '\n';
	//	return false;
	//}
	okLong okZZAAA = toLongNullable(wkStr);
	if (!okZZAAA.first)
	{
		if (showMsg) lg.bump(3) << ERR << "Cannot set zzaaa to " << za << "\n";
		return false;
	}
	nLong nZZAAA = okZZAAA.second;
	if (!nZZAAA.has_value())
	{
		return true;
	}
	if (nZZAAA.value() < 1000)
	{
		if (showMsg) lg.bump(3) << ERR << "Cannot set zzaaa to " << za << " < 1000\n";
		return false;
	}
	return true;
}


//  ***************************************************************************
bool Material3Names::setZZAAA(const std::string& za)
{
	if (!zzaaa.empty()) lg.bump(1) << ERR << "Overwriting current zzaaa " << zzaaa << '\n';
	std::string wkStr = convertNullToEmpty(za);			// trimmed in convertNullToEmpty.
	if (isValidZZAAA(wkStr))		// Allow only longs.
	{
		zzaaa = wkStr;
		return true;
	}
	else
	{
		zzaaa = "";
		lg.bump(2) << ERR << "Could not set ZZAAA to " << za << '\n';
		return false;
	}
}

//  ***************************************************************************
bool Material3Names::sanityCheck() const
{
	if (zzaaa.empty() && shortName.empty() && longName.empty())
	{
		lg.bump(3) << ERR << "No name\n";
		return false;
	}
	if (!longName.empty() && longName == shortName)
	{
		lg.bump(4) << ERR << "Duplicate longname and shortname: " << longName << "\n";
		return false;
	}
	if (!zzaaa.empty())
	{
		if (zzaaa == longName || zzaaa == shortName)
		{
			lg.bump(3) << ERR << "Duplicate names: " << zzaaa << "\n";
			return false;
		}
	}
	return true;
}

//  ***************************************************************************
bool Material3Names::contains(const std::string& name) const
{
	if (name == shortName || name == longName || name == zzaaa) return true;
	return false;
}

//  ***************************************************************************
std::vector <std::string> Material3Names::getAllNames() const
{
	std::vector <std::string> allNames;
	if (longName != "") allNames.push_back(longName);
	if (shortName != "") allNames.push_back(shortName);
	if (zzaaa != "") allNames.push_back(zzaaa);
	return allNames;
}

//  ***************************************************************************
std::string Material3Names::str() const
{
	return ("{long name : " + getLongName() + "  " + "Short name: "
		+ getShortName() + "  " + "ZZAAA: " + getStringZZAAA()) + "}";

}

//  ***************************************************************************
std::ostream& operator<<(std::ostream& os, const Material3Names& name3)
{
	os << name3.str();
	return os;
}


//  ***************************************************************************
bool operator == (const Material3Names& lhs, const Material3Names& rhs)
{
	if (lhs.longName != rhs.longName || lhs.shortName != rhs.shortName
		|| lhs.zzaaa != rhs.getStringZZAAA()) return false;
	return true;
}

//  ***************************************************************************
//  Used a lambda to have a sorting function with lsz parameters.
//
void Material3Names::sort(std::vector<Material3Names>& names, const std::string& order)
{
	std::sort(names.begin(), names.end(), [order](const Material3Names& lhs, const Material3Names& rhs)
		{
			for (char chr : order)
			{
				switch (chr)
				{
				case 'l':
					if (myLess(lhs.getLongName(), rhs.getLongName())) return true;
					if (lhs.getLongName() != rhs.getLongName()) return false;
					continue;
				case 's':
					if (myLess(lhs.getShortName(), lhs.getShortName())) return true;
					if (lhs.getShortName() != rhs.getShortName()) return false;
					continue;
				case 'z':
					if (myLess(lhs.getStringZZAAA(), rhs.getStringZZAAA())) return true;
					if (lhs.getShortName() != rhs.getShortName()) return false;
					continue;
				default:
					lg.bump(4) << ERR << "Name can be sorted on l, s, z. Not " << chr << '\n';
					continue;
				};
			}
			return false;
		}
	);
}

