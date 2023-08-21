#include <iomanip>


#include "controller.h"
#include "utilities.h"
#include "materials.h"
#include <algorithm>
#include "nullableMath.h"
#include "tests.h"

extern Logger lg;
extern std::unique_ptr<MaterialMap>  materialMap;


//  ***************************************************************************
Controller::Controller()
:lineNumber (0), originalLine("")
{
}

//  ***************************************************************************
bool Controller::setControlFileName(const std::string name)
{
	if (ifs.is_open()) ifs.close();
	ifs.open(name, std::ios::in);
	if (!ifs)
	{
		std::cout << "Error opening controller file " << name << std::endl;
		return false;
	};
	return true;
}

//  ***************************************************************************
void Controller::execute()
{
	while (getline(ifs, originalLine))
	{
		lg << SEC << lineNumber << "   " << originalLine << '\n';
		lg.forceWriteToLog();			// Always list command from command file					
		lineNumber++;
		std::string uncommented = removeComments(originalLine);
		std::vector<std::string> spl = split(uncommented, 1);	// split in two. Removes whitespace.
		if (spl.size() == 0) continue;							// skip empty lines
		auto command = toLower(spl[0]);							// lower case for following comparisons

		std::string target = "";
		if (spl.size() > 1) target = spl[1];					// target = spl[1] may contain internal whitespace
		if (command == "loaddata")				materials.readMaterialFile(target);
		else if (command == "consistency")		checkConsistencyMaterial(target);
		else if (command == "compare")		    compare(target);
		else if (command == "errorlimit")		setErrorLimit(target);
		else if (command == "listnames")		listNames(target);
		else if (command == "fileverbosity")	lg.setFileVerbosity(target);
		else if (command == "coutverbosity")	lg.setCoutVerbosity(target);
		else if (command == "logfile")			lg.setLoggerFileName(target);
		else if (command == "show")				showMaterial(target);
		else if (command == "mcnp")				showMcnpFormat(target);
		else if (command == "normalize")		normalize(target);
		else if (command == "tests")
		{
			if (target != "")  std::cout << target << " ignored in command " << originalLine << "\n";
			runTests(); 
		}
		else
		{
			lg.bump(MAX_VERBOSITY) << ERR << "Unrecognized command '" << command << "'\n\n";
		}
		lg.writeToLog();
	}
}

//  ***************************************************************************
void Controller::setErrorLimit(const std::string& limit)
{
	nDbl doubleLimit = toDoubleNullable(limit).second;
	if (!doubleLimit.has_value()) return;
	setPrecisionForMatching(doubleLimit.value());
}

//  ***************************************************************************
void Controller::showMaterial(const std::string& target)
{
	if (target.empty())
	{
		lg.bump(2) << ERR << "No target for show.\n";
 		return;
	}
	auto mat = materialMap->find(target);
	if (!mat)
	{
		lg.bump(MAX_VERBOSITY) << ERR << "material '" << target << "' not found.\n\n";
	}
	else
	{
		lg.writeToLog();
		lg << *mat << '\n';
		lg << "------------------------------------------" << '\n';
		lg.forceWriteToLog();
	}
}

//  ***************************************************************************
//  will show both mass and atom fractions
//
void Controller::showMcnpFormat(const std::string& target)
{
	int numAcross = 2;			// num of isotope {name, fraction} pairs in one line
	int precision = 6;			// precision of fraction
	NumFormat numberFormat = DEFAULT;

	std::vector<std::string> spl = split(target);
	int numItems = (int)spl.size();
	if (numItems == 0)
	{
		lg.bump(4) << ERR << "No material name for mcnp printing\n";
		return;
	}
	std::string matName = spl.back();
	spl.pop_back();
	auto mat = materialMap->find(matName);
	if (!mat)
	{
		lg.bump(4) << ERR << "Cannot interpret ' " << target << " ' for mcnp output.\n";
		return;
	}

	for (const auto& str : spl)
	{
		auto first = tolower(str[0]);
		switch (first)
		{
		case 'a':
		{
			okLong across = toLongNullable(str.substr(1, std::string::npos));
			if (!across.first)
			{
				lg.bump(3) << ERR << "Unrecognized format " << str << '\n';
				break;
			}
			numAcross = across.second.value_or(numAcross);
			break;
		}
		case 'p':
		{
			okLong prec = toLongNullable(str.substr(1, std::string::npos));
			if (!prec.first)
			{
				lg.bump(3) << ERR << "Unrecognized format " << str << '\n';
				break;
			}
			precision = prec.second.value_or(precision);
			break;
		}
		default:
			if (str == "DEFAULT" || str == "default" || str == "Default")
			{
				numberFormat = DEFAULT;
			}
			else if (str == "FIXED" || str == "fixed" || str == "Fixed")
			{
				numberFormat = FIXED;
			}
			else if (str == "SCIENTIFIC" || str == "scientific" || str == "Scientific")
			{
				numberFormat = SCIENTIFIC;
			}
			else
			{
				lg.bump(4) << ERR << "Unrecognized " << str << '\n';
			}
		}
	}

	lg << "c " << mat->get3Names() << '\n';
	lg << "c  mass density = " << -1.0*mat->getMassDensity();
	lg << "     " << "atom density = " << mat->getAtomDensity() << '\n';
	lg << "c  neutron format" << '\n';
	auto& nComp = mat->getNeutronComposition();
	lg << nComp.showMcnpFormat(numAcross, precision, numberFormat);
	lg << "c  photon format" << '\n';
	auto& pComp = mat->getPhotonComposition();
	lg << pComp.showMcnpFormat(numAcross, precision, numberFormat);
	lg << '\n';
	lg.bump(MAX_VERBOSITY);
}

//  ***************************************************************************
//  Should be easier than this.
//
void Controller::listNames(const std::string& target)
{
//	auto mapValues = materialMap->uniqueMaterials();

	// Target can contain only 'l', 's', or 'z' without duplicates
	std::string order = "";
	for (auto chr : target)
	{
		if (chr != 'l' && chr != 's' && chr != 'z')
		{
			lg.bump(MAX_VERBOSITY) << ERR << "Name list can have only 'l', 's', or 'z'. ";
			lg << target << '\n';
			return;
		}
		size_t match = order.find_first_of(chr);
		if (match != std::string::npos)
		{
			lg.bump(MAX_VERBOSITY) << ERR << "Name list can not have duplicates. ";
			lg << target << '\n';
			return;
		}
		order += chr;
	}
	if (order.empty()) order = "lsz";

	auto uniqueMaterials = materialMap->uniqueMaterials();
	if (uniqueMaterials.size() == 0)
	{
		lg << ERR << "No materials to sort.\n";
		return;
	}
	auto uniqueSortedMaterials = materialMap->sortByName(uniqueMaterials, order);

	// Get appropriate column widths
	size_t nColumns = order.size();
	std::vector <size_t> columnWidth;
	for (size_t idx = 0; idx < nColumns; idx++) columnWidth.push_back(0);
	for (auto& mat : uniqueSortedMaterials)
	{
		for (size_t col = 0; col < nColumns; ++col)
		{
			auto chr = order[col];
			switch (chr)
			{
			case 'l': 
				columnWidth[col] = std::max(columnWidth[col], mat->get3Names().getLongName().size());
				break;
			case 's': 
				columnWidth[col] = std::max(columnWidth[col], mat->get3Names().getShortName().size());
				break;
			case 'z':
				columnWidth[col] = std::max(columnWidth[col], mat->get3Names().getStringZZAAA().size());
				break;
			}
		}
	}

	for (auto& mat : uniqueSortedMaterials)
	{
		lg << STD << std::left;
		for (size_t col = 0; col < nColumns; ++col)
		{
			auto chr = order[col];
			switch (chr)
			{
			case 'l':
				lg << std::left << std::setw(columnWidth[col] + 1) << mat->get3Names().getLongName();
				break;
			case 's':
				lg << std::left << std::setw(columnWidth[col] + 1) << mat->get3Names().getShortName();
				break;
			case 'z': 
				lg << std::right << std::setw(columnWidth[col]) << mat->get3Names().getStringZZAAA();
				lg << " ";
				break;
			}
		}
		lg << '\n';
	}
	lg.forceWriteToLog();
}

//  ***************************************************************************
void Controller::showConsistency(std::shared_ptr<Material> mat, bool ok)
{
	static std::ostringstream errMsg;
	if (ok)
	{
		lg.clear();					// Ignore warnings and errors if consistent.
		lg << STD << "Material " << mat->get3Names() << " is consistent within ";
		lg << getPrecisionForMatching() * 100 << "%.\n";
	}
	else
	{
		errMsg.str("");
		errMsg << "Material " << mat->get3Names() << " is not consistent within ";
		errMsg << getPrecisionForMatching() * 100 << "%.\n";
		lg.prepend(mat->str());
		lg.prepend("\n" + errMsg.str());			// Message before
		lg << STD << errMsg.str() << "\n";			// and after
	}

	lg.forceWriteToLog();
}

//  ***************************************************************************
void Controller::checkConsistencyMaterial(const std::string& target)
{
	bool ok;

	if (target.empty())
	{
		lg.bump(MAX_VERBOSITY) << ERR << "No target for evaluation.\n";
		return;
	}
	if (toLower(target) == "all")
	{
		auto mapValues = materialMap->uniqueMaterials();
		for (auto& mat : mapValues)
		{
			ok = mat->isMaterialSelfConsistent();
			showConsistency(mat, ok);
		}
		return;
	}
	if (toLower(target) == "all-")				// show only those that fail
	{
 		auto mapValues = materialMap->uniqueMaterials();
		for (auto& mat : mapValues)
		{
			ok = mat->isMaterialSelfConsistent();
			if (!ok) showConsistency(mat, ok);
			else lg.clear();
		}
	}
	else
	{
		auto mat = materialMap->find(target);
		if (!mat)
		{
			lg.prepend(STD + "Checking consistency of " + target + '\n');
			lg.bump(MAX_VERBOSITY) << ERR << "material '" << target << "' not found.\n";
			lg << STD << "Cannot check consistency.\n";
			lg.writeToLog();
			return;
		}
		ok = mat->isMaterialSelfConsistent();
		showConsistency(mat, ok);
	}
}

//  ***************************************************************************
//  Compare not-loaded material file with already-loaded materials.
//
void Controller::compare(const std::string& target)
{
	lg << STD << "Compare materials in '" << target << "'against materials already loaded.\n";
	if (target.empty())
	{
		lg.bump(4) << ERR << "No target file for comparison.\n";
		return;
	}
	MaterialFileReader reader;
	lg.writeToLog();
 	reader.compareMaterialFile(target);
	lg.forceWriteToLog();

	return;
}

//  ***************************************************************************
//  Normalize a material composition to 1.0
//
void Controller::normalize(const std::string& target)
{
	if (target.empty())
	{
		lg.bump(MAX_VERBOSITY) << ERR << "No target for evaluation.\n";
		return;
	}
	auto mat = materialMap->find(target);
	if (!mat)
	{
		lg.prepend(STD + "normalizing " + target + '\n');
		lg.bump(MAX_VERBOSITY) << ERR << "material '" << target << "' not found.\n";
		lg << STD << "Cannot normalizing.\n";
		return;
	}
	CompositionSums sum = mat->getNeutronComposition().getSums();
	lg << MNR << "Before\n";
	lg << STD << "Neutron composition:   Mass " << sum.mass << "  Atom " << sum.atom << '\n';
	sum = mat->getPhotonComposition().getSums();
	lg << STD << "Photon composition:    Mass " << sum.mass << "  Atom " << sum.atom << '\n';
	mat->normalize();
	sum = mat->getNeutronComposition().getSums();
	lg << '\n' << MNR << "After\n";
	lg << STD << "Neutron composition:   Mass " << sum.mass << "  Atom " << sum.atom << '\n';
	sum = mat->getPhotonComposition().getSums();
	lg << STD << "Photon composition:    Mass " << sum.mass << "  Atom " << sum.atom << '\n';
	lg.forceWriteToLog();
}