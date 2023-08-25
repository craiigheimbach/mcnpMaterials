#include <string>
#include <iostream>
#include <fstream>
#include <iomanip>
#include <vector>
#include <set>
#include <algorithm>

#include "utilities.h"
#include "materials.h"
#include "logger.h"

extern Logger lg;
extern std::unique_ptr<MaterialMap>  materialMap;

//  ***************************************************************************
MaterialFileReader::MaterialFileReader()
{
}

//  ***************************************************************************
MaterialFileReader::~MaterialFileReader()
{
}

//  ***************************************************************************
bool MaterialFileReader::readMaterialFile(const std::string& fileName)
{
	inFileMat.open(fileName, std::ios::in);
	if (!inFileMat.is_open())
	{
		lg.bump(5) << ERR << "Failed to open " << fileName << '\n';
		return false;
	}
	while (inFileMat.peek() != EOF)
	{
		bool ok = readNextMaterial();
		if (mat == nullptr)	continue;				// Check for beginning or end of file
		lg.prepend("   Loading {" + mat->get3Names().str() + "}\n");
		if (!ok || !materialMap->addMaterial(mat))
		{
			lg.bump(MAX_VERBOSITY) << ERR << "Did not load material {" << mat->get3Names() << "}\n";
		}
		else
		{
			lg << STD << "Loaded material {" << mat->get3Names() << "}\n";
		}
		lg << '\n';
		lg.writeToLog();
	}
	inFileMat.close();
	return true;
}

//  ***************************************************************************
bool MaterialFileReader::compareMaterialFile(const std::string& fileName)
{
	// new material may have names corresponding to >1 existing material.
	std::set<std::shared_ptr<Material>> alreadyMatched{};
	inFileMat.open(fileName, std::ios::in);
	if (!inFileMat.is_open())
	{
		lg.bump(5) << ERR << "Failed to open " << fileName << " for comparison\n";
		return false;
	}

	while (inFileMat.peek() != EOF)
	{
		lg.clear();
		bool ok = readNextMaterial();
		if (ok && !mat)	continue;				// materials was commented out or ignored
		if (!ok || !mat->sanityCheck())			// not a valid material
		{
			lg.bump(MAX_VERBOSITY).prepend("Error reading material for comparison.\n");
			lg << *mat;
			lg << STD << "  Did not load.\n";
			lg.writeToLog();
			continue;
		}
		// Show only one comparison even if multiple names match.
		// Material with more than one name may match more than one material
		auto allNames = mat->get3Names().getAllNames();
		for (auto& name : allNames)
		{
			auto original = materialMap->find(name);
			if (!original) continue;
			auto different = alreadyMatched.insert(original);
			if (different.second == false) continue;
			lg << '\n';
			showMaterialComparison(original, mat);
			lg.forceWriteToLog();
		}
		if (alreadyMatched.empty())
		{
			//lg.bump(3) << ERR << "Could not find loaded material to match ";
			//lg << mat->get3Names() << '\n';
			lg.forceWriteToLog();
		}
		alreadyMatched.clear();
	}
	inFileMat.close();
	return true;
}

//  ***************************************************************************
//  Returns ok == true for good material or empty (ignored) material.
//			If empty, sets mat to nullptr
//  If ok == false, mat != nullptr, but is malformed.
//
bool MaterialFileReader::readNextMaterial()
{
	mat = nullptr;
	std::string line;
	processingStatus = NONE;			// not NEUTRON or PHOTON
	bool ok = true;
	bool skipMaterial = false;

	while (std::getline(inFileMat, line))
	{
		line = removeComments(line);	// Also trims
		if (line.empty()) continue;
		if (line.find_first_not_of('-') == std::string::npos) return ok;	// New material separator
		if (line[0] == '*')
		{
			if (line == "*ignore*" || line == "*Ignore*")
			{
				skipMaterial = true;
				mat = nullptr;
				continue;
			}
			else
			{
				lg.bump(4) << ERR << "Error in interpreting line:" << line << '\n';
				continue;
			}
		}
		if (!skipMaterial)
		{
			if (!mat) mat = std::make_shared<Material>();
			ok &= processLine(line);
		}
	}
	return ok;
}

//  ***************************************************************************
//  Log messages from this are added to temp logger so verbosity
//  does not carry over from material to the next.
//
bool MaterialFileReader::processLine(const std::string& line)
{
	auto sp2 = split2(line);
	std::string first = sp2.first;	// Not a reference.

	// Anything other than processing component reverts status to NONE
	PROCESSING_STATUS tempStatus = processingStatus;
	processingStatus = NONE;

	if (first == "ZZAAA") return processZZAAA(sp2.second);
	if (sp2.first == "zzaaa") return processZZAAA(sp2.second);	// lower case allowed
	first[0] = tolower(first[0]);								// Allow initial char upper case
	if (first == "longName")	return processLongName(sp2.second);
	if (first == "long")		return processLong(sp2.second);
	if (first == "shortName")	return processShortName(sp2.second);
	if (first == "short")		return processShort(sp2.second);
	if (first == "comment")		return processComment(sp2.second);
	if (first == "atomic")		return processAtomicWeight(sp2.second);
	if (first == "isBasic")		return processIsBasic(sp2.second);
	if (first == "is")			return processIs(sp2.second);
	if (first == "atom")		return processAtomDensity(sp2.second);
	if (first == "mass")		return processMassDensity(sp2.second);
	if (first == "neutron")		return processNeutronCompText(sp2.second);
	if (first == "photon")		return processPhotonCompText(sp2.second);

	processingStatus = tempStatus;
	if (first == "isotope") return processIsotopeText(sp2.second);
	if (first == "fraction") return processFractionText(sp2.second);
	if (tryProcessingComponent(sp2))
	{
		return true;
	}
	else
	{
		processingStatus = NONE;
		return false;
	}
}

//  ***************************************************************************
std::pair <std::string, std::string> MaterialFileReader::split2(const std::string& text)
{
	std::pair <std::string, std::string> splitPair;
	std::vector<std::string> pieces;
	pieces = split(text, 1);
	if (pieces.size() >= 1) splitPair.first = std::move(pieces[0]);
	if (pieces.size() >= 2) splitPair.second = std::move(pieces[1]);
	return splitPair;
}

// ***************************************************************************
std::string MaterialFileReader::bypassEquals(const std::string& line)
{
	auto cpy = line;
	if (cpy.empty()) return cpy;
	if ((cpy[0] == '=') || (cpy[0] == ':'))
	{
		cpy = cpy.substr(1);
		cpy = ltrim(cpy);
	}
	return cpy;
}

//  ***************************************************************************
//  Check that word matches next word in line.
//
std::optional <std::string> MaterialFileReader::checkAndRemoveWord(const std::string& line, const std::string& word)
{
	if (line.size() == 0) return std::nullopt;
	auto sp2 = split2(line);
	auto& first = sp2.first;
	first[0] = tolower(first[0]);
	if (first != word) return std::nullopt;
	return sp2.second;
}

//  ***************************************************************************
okDbl MaterialFileReader::toDoubleReader(const std::string& str)
{
	std::string wkStr = convertNullToEmpty(str);
	if (wkStr.empty()) return okDbl(true, std::nullopt);
	return toDoubleNullable(wkStr);
}


//  ***************************************************************************
bool MaterialFileReader::processZZAAA(const std::string& line)
{
	std::string zzaaaText = bypassEquals(line);
	return mat->setZZAAA(zzaaaText);
}

//  ***************************************************************************
//  beginning of "is basic"
//
bool MaterialFileReader::processIs(const std::string& line)
{
	auto remainingText = checkAndRemoveWord(line, "basic");
	if (!remainingText.has_value()) return false;
	return processIsBasic(remainingText.value());
}

//  ***************************************************************************
bool MaterialFileReader::processIsBasic(const std::string& line)
{
	static const std::vector <std::string> trueKey{ "true", "t", "True", "T"};
	static const std::vector <std::string> falseKey{ "false", "False", "f", "F"};
	mat->setBasic(false);
	auto text = bypassEquals(line);
	text = convertNullToEmpty(text);
	if (text.empty())
	{
		mat->setBasic(false);
	}
	else if (matchAny(text, trueKey))
	{
		mat->setBasic(true);
	}
	else if (matchAny(text, falseKey))
	{
		mat->setBasic(false);
	}
	else
	{
		mat->setBasic(false);				// default
		std::cout << text << " should be 'true' of 'false'." << '\n';
	}
	return true;
}

//  ***************************************************************************
bool MaterialFileReader::processComment(const std::string& line)
{
	std::string comment = bypassEquals(line);
	mat->setComment(comment);
	return true;
}

//  ***************************************************************************
bool MaterialFileReader::processLong(const std::string& line)
{
	auto remainingText = checkAndRemoveWord(line, "name");
	if (!remainingText.has_value()) return false;
	return processLongName(remainingText.value());
}

//  ***************************************************************************
bool MaterialFileReader::processLongName(const std::string& line)
{
	std::string name = bypassEquals(line);
	return mat->setLongName(name);
}

//  ***************************************************************************
bool MaterialFileReader::processShort(const std::string& line)
{
	auto remainingText = checkAndRemoveWord(line, "name");
	if (!remainingText.has_value()) return false;
	return processShortName(remainingText.value());
}

//  ***************************************************************************
//  Will find "shortname" or "short name" because of lshift.
//
bool MaterialFileReader::processShortName(const std::string& line)
{
	std::string name = bypassEquals(line);
	return mat->setShortName(name);
}

//  ***************************************************************************
bool MaterialFileReader::processAtomicWeight(const std::string& line)
{
	static const std::vector <std::string> weightKey{ "weight", "wt", "Wt", "Weight"};

	auto sp2 = split2(line);
	if (!matchAny(sp2.first, weightKey)) return false;
	if (mat->getAtomicWeight().has_value())
	{
		lg.bump(2) << ERR << "Overwriting atomic weight " << mat->getAtomicWeight() << '\n';
	}
	auto text = bypassEquals(sp2.second);
	nDbl atomicWeight = toDoubleReader(text).second;
	mat->setAtomicWeight(atomicWeight);
	return true;
}

//  ***************************************************************************
bool MaterialFileReader::processMassDensity(const std::string& line)
{
	static const std::vector <std::string> densityKey{ "density", "den", "Density", "Den"};

	auto sp2 = split2(line);
	if (!matchAny(sp2.first, densityKey)) return false;
	if (mat->getMassDensity().has_value())
	{
		lg.bump(2) << ERR << "Overwriting mass density " << mat->getMassDensity() << '\n';
	}
	auto text = bypassEquals(sp2.second);
	nDbl massDensity = toDoubleReader(text).second;
	mat->setMassDensity(massDensity);
	return true;
}

//  ***************************************************************************
bool MaterialFileReader::processAtomDensity(const std::string& line)
{
	static const std::vector <std::string> densityKey{ "density", "den", "Density", "Den" };

	auto sp2 = split2(line);
	if (!matchAny(sp2.first, densityKey)) return false;
	if (mat->getAtomDensity().has_value())
	{
		lg.bump(2) << ERR << "Overwriting atom density " << mat->getAtomDensity() << '\n';
	}
	auto text = bypassEquals(sp2.second);
	nDbl atomDensity = toDoubleReader(text).second;
	mat->setAtomDensity(atomDensity);
	return true;
}

//  ***************************************************************************
bool MaterialFileReader::processNeutronCompText(const std::string& line)
{
	static const std::vector <std::string> compositionKey = { "composition", "comp",
		"Composition", "Comp", "composition:", "comp:", "Composition:"};

	auto sp2 = split2(line);
	if (!matchAny(sp2.first, compositionKey)) return false;
	sp2.second = bypassEquals(sp2.second);
	if (sp2.second.empty()) processingStatus = NEUTRON;
	return true;
}

//  ***************************************************************************
bool MaterialFileReader::processPhotonCompText(const std::string& line)
{
	static const std::vector <std::string> compositionKey = { "composition", "comp",
		"Composition", "Comp", "composition:", "comp:", "Composition:", "Comp:"};
	auto sp2 = split2(line);
	if (!matchAny(sp2.first, compositionKey)) return false;
	sp2.second = bypassEquals(sp2.second);
	if (sp2.second.empty()) processingStatus = PHOTON;
	return true;
}


//  ***************************************************************************
bool MaterialFileReader::processIsotopeText(const std::string& line)
{
	static const std::vector <std::string> massKey = { "mass", "Mass"};
	static const std::vector <std::string> atomKey = { "atom", "Atom"};

	auto sp2 = split2(line);
	if (!matchAny(sp2.first, massKey)) return false;
	if (!matchAny(sp2.second, atomKey)) return false;
	return true;
}

//  ***************************************************************************
bool MaterialFileReader::processFractionText(const std::string& line)
{
	static const std::vector <std::string> fractionKey{ "fraction", "frac", "Fraction", "Frac"};

	auto sp2 = split2(line);
	if (!matchAny(sp2.first, fractionKey)) return false;
	if (sp2.second != "") return false;
	return true;
}

//  ***************************************************************************
bool MaterialFileReader::tryProcessingComponent(std::pair <std::string, std::string> sp2)
{
	auto componentName = sp2.first;
	auto fractions = split2(sp2.second);
	okDbl okMass = toDoubleReader(fractions.first);
	okDbl okAtom = toDoubleReader(fractions.second);
	if ((okMass.first == false) && (okAtom.first == false))
	{
		return false;
	}
	nDbl massFraction = okMass.second;
	nDbl atomFraction = okAtom.second;
	if (processingStatus == NEUTRON)
	{
		mat->addNeutronComponent(Component(componentName, massFraction, atomFraction));
	}
	else if (processingStatus == PHOTON)
	{
		mat->addPhotonComponent(Component(componentName, massFraction, atomFraction));
	}
	else
	{
		lg.bump(4) << ERR << "Cannot process component without knowing whether it is neutron ot photon\n";
		lg << STD << "Component name is " << componentName << '\n';
		return false;
	}
	return true;
}
