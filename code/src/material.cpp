#include <iostream>
#include <iomanip>
#include <optional>
#include <string>

#include "utilities.h"
#include "materials.h"
#include "nullableMath.h"
#include <algorithm>


extern Logger lg;
extern std::unique_ptr<MaterialMap>  materialMap;

//  ***************************************************************************
Component::Component()
	:name(""), massFraction(std::nullopt), atomFraction(std::nullopt)
{}

//  ***************************************************************************
Component::Component(std::string name, nDbl massFraction, nDbl atomFraction)
	:Component()
{
	setName(name);
	setFraction(MASS, massFraction);
	setFraction(ATOM, atomFraction);
}

//  ***************************************************************************
bool Component::setName(const std::string& newName)
{
	auto wkStr = convertNullToEmpty(newName);
	name = "";								// set name to empty string if error found.
	if (wkStr.empty())
	{
		lg.bump(4) << ERR << "Component must have a name, not empty string or null " << newName << '\n';
		return false;
	}
	//if (hasWhitespace(wkStr))
	//{
	//	lg.bump(4) << ERR << "Component name cannot have whitespace " << newName << '\n';
	//	return false;
	//}
	// Can be either ZZAAA or valid name. 
	okLong okZZAAA = toLongNullable(wkStr, false);			// Check ZZAAA
	if (okZZAAA.first)
	{
		nLong nZZAAA = okZZAAA.second;
		if (!nZZAAA.has_value())
		{
			lg.bump(4) << ERR << "Component name cannot be null " << newName << '\n';
			return false;
		}
		if (nZZAAA.value() < 1000)
		{
			lg.bump(3) << ERR << "Component name cannot be zzaaa < 1000 " << nZZAAA << '\n';
			return false;
		}
		name = wkStr;
		return true;
	}
	if (Material3Names::isValidName(newName))		// ok to keep error messages.
	{
		name = newName;
		return true;
	}
	lg.bump(3) << ERR << "Component name is not valid '" << newName << "'\n";
	return false;
}

//  ***************************************************************************
//  Allow fractions == 0 or > 1.0 with a warning
//
bool Component::isFractionValid(nDbl frac)
{
	if (!frac.has_value()) return true;
	else if (frac < 0.0) return false;
	else if (frac == 0.0)
	{
		lg.bump(1) << ERR << "Fraction should not == 0." << '\n';
	}
	return true;
}

//  ***************************************************************************
void Component::setFraction(FRACTION_TYPE type, nDbl fraction)
{
	if (!isFractionValid(fraction))
	{
		fraction = std::nullopt;
	}
	if (type == MASS) massFraction = fraction;
	else  atomFraction = fraction;
}

//  ***************************************************************************
bool Component::sanityCheck() const
{
	if (name.empty())
	{
		lg.bump(3) << ERR << "Component must have a name. " << *this << "\n";
		return false;
	}
	if (!massFraction && !atomFraction)
	{
		lg.bump(3) << ERR << "Component must have atom or mass fractions. " << *this << "\n";
		return false;
	}
	if (massFraction.has_value() && (massFraction.value()) < 0.)
	{
		lg.bump(3) << ERR << "Component fraction < 0. \n   ";
		lg << STD << *this << "\n";
		return false;
	}
	if (atomFraction.has_value() && (atomFraction.value() < 0.))
	{
		lg.bump(3) << ERR << "Component fraction < 0. \n   ";
		lg << STD << *this << "\n";
		return false;
	}
	return true;
}

//  ***************************************************************************
bool match(const Component& left, const Component& right)
{
	if (left.getName() != right.getName()) return false;
	if (!match(left.getFraction(ATOM), right.getFraction(ATOM))) return false;
	if (!match(left.getFraction(MASS), right.getFraction(MASS))) return false;
	return true;
}

//  ***************************************************************************
std::ostream& operator<<(std::ostream& os, const Component& component)
{
	os << std::right << std::setw(12) << component.getName();
	os << "  "; 
	os << std::left << std::setw(12) << component.massFraction;
	os << "  ";
	os << std::left << std::setw(12) << component.atomFraction;
	return os;
}


//  ***************************************************************************
Composition::Composition()
{
}

//  ***************************************************************************
bool Composition::setFractions(FRACTION_TYPE type, std::vector <nDbl>& newFractions)
{
	if (components.size() != newFractions.size()) return false;
	for (size_t ia = 0; ia < components.size(); ++ia)
	{
		if (type == MASS) components[ia].setFraction(MASS, newFractions[ia]);
		else components[ia].setFraction(ATOM, newFractions[ia]);
	}
	return true;
}

//  ***************************************************************************
std::pair<nDbl, nDbl> Composition::getSums() const
{
	nDbl sumMass = 0;
	nDbl sumAtom = 0;
	for (const auto& comp : components)
	{
		sumMass = sumMass + comp.getFraction(MASS);
		sumAtom = sumAtom + comp.getFraction(ATOM);
	}
	return std::make_pair(sumMass, sumAtom);
}

//  ***************************************************************************
void Composition::sort()
{
	std::sort(components.begin(), components.end(), [](const Component& left, const Component& right)
			{
				return myLess(left.getName(), right.getName());
			});
}

//  ***************************************************************************
//  Does not check anything that needs atomic weight
//
bool Composition::areFractionsConsistent() const
{
	bool ok = true;
	if (components.empty())	return ok;

	nDbl sumAtom = 0.0;			// Fractions should sum to 1.0
	nDbl sumMass = 0.0;
	for (auto& component : components)
	{
		sumAtom = sumAtom + component.getFraction(ATOM);
		sumMass = sumMass + component.getFraction(MASS);
	}
	if (!sumAtom.has_value() && !sumMass.has_value())
	{
		ok = false;
		lg.bump(4) << ERR << "Need a complete mass or atom set of fractions.\n";
	}
	else
	{
		if (sumAtom.has_value() && !match(sumAtom.value(), 1.0))
		{
			ok = false;
			lg.bump(4) << ERR << "Atom fractions sum to  " << sumAtom.value() << " , not 1.0\n";
		}
		if (sumMass.has_value() && !match(sumMass.value(), 1.0))
		{
			ok = false;
			lg.bump(4) << ERR << "Mass fractions sum to  " << sumMass.value() << " , not 1.0\n";
		}
		if (sumAtom.has_value() && sumMass.has_value())
		{
			if (!match(sumMass.value(), sumAtom.value()))
			{
				ok = false;
				lg.bump(4) << ERR << "Sums of atom and mass fractions do not match";
				lg << STD << " sum mass = " << sumMass.value();
				lg << STD << " sum atom = " << sumAtom.value();
			}
		}
	}
	return ok;
}

//  ***************************************************************************
void Composition::normalize()
{
	auto [sumMass, sumAtom] = getSums();
	for (auto& component : components)
	{
		component.setFraction(MASS, component.getFraction(MASS) / sumMass);
		component.setFraction(ATOM, component.getFraction(ATOM) / sumAtom);
	}
}

//  ***************************************************************************
//  For basic materials only
//
bool Composition::basicMaterialSanityCheck(const std::string& zzaaa) const
{
	if (zzaaa.empty())
	{
		lg.bump(3) << ERR << "Basic material must have a zzaaa\n";
		return false;
	}
	if (components.size() == 0) return true;	// Will fill in when needed.
	if (components.size() > 1)
	{
		lg.bump(3) << ERR << "Composition of basic material must have one component.\n";
		return false;
	}
	auto& onlyComponent = components[0];
	if (zzaaa != onlyComponent.getName())
	{
		lg.bump(3) << ERR << "For basic material, zzaaa must match neutron component name.\n";
		lg << STD << zzaaa << " vs " << onlyComponent.getName() << "\n";
		return false;
	}
	if (!match(onlyComponent.getFraction(MASS), 1.0) || !match(onlyComponent.getFraction(ATOM), 1.0))
	{
		lg.bump(3) << ERR << "For basic material, component fractions must be 1.0.\n";
		return false;
	}
	return true;
}

//  ***************************************************************************
//  Compute mass fractions from atom fractions and vice-versa.
//  Need atomic weights of components.
//  One computed nullopt fraction makes all computed fraction nullopt.
//  Maintains normalization.
//
std::vector <nDbl> Composition::computeFractions(FRACTION_TYPE computedType) const
{
	nDbl sumOriginalFractions = 0.0;
	nDbl sumDerivedlFractions = 0.0;
	size_t numComponents = components.size();
	std::vector <nDbl> derivedFraction(numComponents, std::nullopt);

	for (size_t ia = 0; ia < numComponents; ++ia)
	{
		nDbl originalFraction = std::nullopt;

		auto& originalComponent = components[ia];
		auto mat = materialMap->find(originalComponent.getName());
		if (!mat) return std::vector <nDbl>(numComponents, std::nullopt);
		// To get mass fraction, start with atom fraction and vice-versa
		if (computedType == MASS)
		{
			originalFraction = originalComponent.getFraction(ATOM);
			derivedFraction[ia] = mat->convertMassAtom(MASS, originalFraction);
		}
		else
		{
			originalFraction = originalComponent.getFraction(MASS);
			derivedFraction[ia] = mat->convertMassAtom(ATOM, originalFraction);
		}
		sumOriginalFractions = sumOriginalFractions + originalFraction;
		sumDerivedlFractions = sumDerivedlFractions + derivedFraction[ia];
	}
	nDbl normalization = sumOriginalFractions / sumDerivedlFractions;
	if (!normalization.has_value()) return std::vector <nDbl>(numComponents, std::nullopt);
	for (auto&  frac : derivedFraction) frac = frac * normalization;
	return derivedFraction;
}

//  ***************************************************************************
void Composition::addComponent(Component newComponent)
{
	if (newComponent.getName().empty())
	{
		lg.bump(4) << ERR << "Cannot have a component with no name. " << newComponent << "\n";
		return;
	}
	for (auto& comp : components)		// If component already in composition, just add.
	{
		if (newComponent.getName() == comp.getName())
		{
			comp.setFraction(MASS, comp.getFraction(MASS) + newComponent.getFraction(MASS));
			comp.setFraction(ATOM, comp.getFraction(ATOM) + newComponent.getFraction(ATOM));
			return;
		}
	}
	components.push_back(newComponent);	// Not a duplicate
};

//  ***************************************************************************
const Component* Composition::find(const std::string& name) const
{
	for (const auto& component : components)
	{
		if (name == component.getName()) return &component;
	}
	return nullptr;
}

//  ***************************************************************************
bool Composition::compositionSanityCheck() const
{
	if (components.empty()) return true;
	for (const auto& component : components)
	{
		if (!component.sanityCheck()) return false;
	}
	return true;
}

//  ***************************************************************************
//  Fill in mass fractions from atom fractions or vice-versa, etc, if needed.
bool Composition::fillInMissingData()
{
	if (components.size() == 0) return true;
	auto sums = getSums();
	bool hasMassData = sums.first.has_value();
	bool hasAtomData = sums.second.has_value();
	if (hasAtomData == true && hasMassData == true) return true;
	if (hasAtomData == false && hasMassData == false)
	{
		lg.bump(MAX_VERBOSITY) << ERR << "Insufficient data to compute material properties.\n";
		return false;
	}

	if (hasMassData == true && hasAtomData == false)
	{
		std::vector <nDbl> aFrac =  computeFractions(ATOM);
		setFractions(ATOM, aFrac);
	}
	if (hasAtomData == true && hasMassData == false)
	{
		std::vector <nDbl> mFrac = computeFractions(MASS);
		setFractions(MASS, mFrac);
	}
	return true;
}

//  ***************************************************************************
void Composition::removeZeroFractions()
{
	std::vector <Component> nonZero;				// Put desired components here
	for (auto& component : components)
	{
		if (!component.getFraction(MASS).has_value() || !component.getFraction(ATOM).has_value())
		{
			nonZero.push_back(component);
			continue;
		}
		double atomF = component.getFraction(ATOM).value();
		double massF = component.getFraction(MASS).value();
		if ((atomF == 0.0 && massF != 0.0) || (atomF != 0.0 && massF == 0.0))
		{
			lg.bump(MAX_VERBOSITY) << ERR << "atom fraction = " << atomF << ", but ";
			lg << STD << "massF = " << massF << '\n';
			continue;
		}
		if (atomF != 0.0)				// From previous "if", need to check only one
		{
			nonZero.push_back(component);
		}
	}
	components = nonZero;
}

//  ***************************************************************************
bool match(const Composition& left, const Composition& right)
{
	Composition leftSorted = left;
	leftSorted.sort();
	Composition rightSorted = right;
	rightSorted.sort();
	auto& leftComponents = leftSorted.getComponents();
	auto& rightComponents = rightSorted.getComponents();
	if (leftComponents.size() != rightComponents.size()) return false;
	for (size_t index = 0; index < leftComponents.size(); ++index)
	{
		if (!match(leftComponents[index], rightComponents[index]))
			return false;
	}
	return true;
}

//  ***************************************************************************
std::ostream& operator<<(std::ostream& os, const Composition& comp)
{
	os << "  Isotope      Mass      Atom\n";
	os << "             Fraction  Fraction\n";
	nDbl sumMass = 0.0;
	nDbl sumAtom = 0.0;
    for (auto& component : comp.components)
    {
		sumMass = sumMass + component.getFraction(MASS);
		sumAtom = sumAtom + component.getFraction(ATOM);
        os << component << '\n';
    }
	Component sum = Component("sum", sumMass, sumAtom);
	if (!match(sumMass, 1.0, 0.001) || !match(sumAtom, 1.0, 0.001))
	{
		os << sum << '\n';
		os << '\n';
	}
    return os;
}

//  ***************************************************************************
nDbl Material::computeDensity(FRACTION_TYPE toType) const
{
	nDbl sourceDensity;
	if (toType == MASS)
	{
		sourceDensity = atomDensity;
		return convertMassAtom(MASS, sourceDensity) / AVOGADRO;
	}
	else
	{
		sourceDensity = massDensity;
		return convertMassAtom(ATOM, sourceDensity) * AVOGADRO;
	}
};


//  ***************************************************************************
//  This function reduces the components of a material to basic, or MCNP-ready
//  form, which will be ZZAAA. 
//  Expands based on neutron composition.
//
bool Material::resolveToBasic()
{ 
	if (isBasic()) return true;
	//
	// expand each existing component and add to new composition
	//  e.g. 1000 -> 1001 and 1002
	//
	//  Compute all components.
	//  Compute atom fraction from atom fractions, mass fraction from mass fractions.
	//
	auto newComposition = NeutronComposition();
	for (const auto& cmpnt : neutronComposition.components)
	{
		nDbl atomFrac = cmpnt.getFraction(ATOM);
		nDbl massFrac = cmpnt.getFraction(MASS);
		auto cmpntMat = materialMap->find(cmpnt.getName());
		if (!cmpntMat)
		{
			lg << STD << "Decomposing " << name3 << '\n';
			lg.bump(5) << ERR << "Cannot find " << cmpnt.getName() << "\n";
			return false;
		}
		for (const auto& subCmpnt : cmpntMat->neutronComposition.getComponents())
		{
			auto component = Component(subCmpnt.getName(), massFrac * subCmpnt.getFraction(MASS),
				atomFrac * subCmpnt.getFraction(ATOM));
			newComposition.addComponent(component);
		}
	}
	neutronComposition = newComposition;
  	return true;
}

//  ***************************************************************************
std::string Composition::showMcnpFormat(int across, int precision, NumFormat fmt) const
{
	if (across <= 0) across = 1;
	if (precision <= 0) precision = 3;
	int width = precision + 7;

	std::ostringstream oss;
	oss << std::setprecision(precision);
	switch (fmt)
	{
	case FIXED:
		oss << std::fixed;
		width = precision + 2;
		break;
	case SCIENTIFIC:
		oss << std::scientific;
		width = precision + 7;
		break;
	case DEFAULT:
		oss << std::defaultfloat;
	}

	// Extra test for sums
	auto [sumMass, sumAtom] = getSums();
	Component sum = Component("sum", sumMass, sumAtom);

	oss << "c Mass Fractions  ";
	if (!match(sumMass, 1.0, 0.001))
	{
		oss << "sum = " << sumMass;
	}
	oss << '\n';
	int nAcross = 0;
	for (auto& component : components)
	{
		if (nAcross >= across)
		{
			oss << " &\n";
			nAcross = 0;
		}
		nAcross++;
		oss << "  " << std::left << std::setw(7) << component.getName();
		oss << " " << std::setw(width) << - 1.0 * component.getFraction(MASS);
	}
	oss << "\nc Atom Fractions  ";
	if (!match(sumAtom, 1.0, 0.001))
	{
		oss << "sum = " << std::setprecision(6) << sumAtom;
	}
	oss << '\n';
	nAcross = 0;
	for (auto& component : components)
	{
		if (nAcross >= across)
		{
			oss << " &\n";
			nAcross = 0;
		}
		nAcross++;
		oss << "  " << std::left << std::setw(7) << component.getName();
		oss << " " << std::setw(width) << component.getFraction(ATOM);
	}
	oss << '\n';
	return oss.str();
}


//  ***************************************************************************
NeutronComposition::NeutronComposition()
{
};

//  ***************************************************************************
nDbl NeutronComposition::computeAtomicWeight() const
{
	auto sums = getSums();
	if (!sums.second.has_value())
	{
		return std::nullopt;
	}

	nDbl average = 0.0;
	for (Component comp : components)
	{
		auto componentMat = materialMap->find(comp.getName());
		if (!componentMat) return std::nullopt;
		if (!componentMat->getAtomicWeight().has_value())
		{
			lg.bump(4) << ERR << "Cannot compute atomic weight. Component atomic weight is missing.  ";
			lg << STD << comp.getName() << '\n';
			return std::nullopt;
		}
		average = average + comp.getFraction(ATOM).value() * componentMat->getAtomicWeight().value();
	}
	return average;
}

//  ***************************************************************************
bool NeutronComposition::areFractionsConsistent() const
{
	bool ok = Composition::areFractionsConsistent();		// Used for both neutrons snd photons

	// see if atom fractions convert to mass fractions.
	auto derivedMassFractions = computeFractions(MASS);
	auto derivedAtomFractions = computeFractions(ATOM);
	bool componentsMatch = true;
	for (size_t ia = 0; ia < components.size(); ++ia)
	{
		if (!match(derivedMassFractions[ia], components[ia].getFraction(MASS)) ||
			!match(derivedAtomFractions[ia], components[ia].getFraction(ATOM)))
		{
			componentsMatch = false;
			break;
		}
	}
	if (!componentsMatch)
	{
		lg.bump(3) << ERR << "Problem with Neutron Consistency\n";
		lg << STD << "Mass and atom fractions are not consistent.\n";

		static const int componentNameWidth = 10;
		static const int floatPrecision = 4;
		static const int minFloatWidth = floatPrecision + 5;
		static const int floatWidth = std::max(minFloatWidth, 13); // Need room for column titles
		static const std::string columnSeparator = " | ";
		static std::vector <std::string> columnHeader {"massF given", "massF derived",
			"Ratio", "atomF given", "atomF derived", "Ratio"};

		lg << std::right << std::setw(componentNameWidth) << "component" << std::left;
		for (auto& col : columnHeader)
		{
			lg << columnSeparator << std::setw(floatWidth) << col;
		}
		lg << '\n';

		auto numComponents = derivedMassFractions.size();
		lg << std::showpoint << std::setprecision(floatPrecision);
		for (size_t ia = 0; ia < numComponents; ++ia)
		{
			auto& original = components[ia];
			auto& derMass = derivedMassFractions[ia];
			auto& derAtom = derivedAtomFractions[ia];
			lg << std::right << std::setw(componentNameWidth) << original.getName() << std::left;
			lg << columnSeparator << std::setw(floatWidth) << original.getFraction(MASS);
			lg << columnSeparator << std::setw(floatWidth) << derMass;
			lg << columnSeparator << std::setw(floatWidth) << derMass / original.getFraction(MASS);
			lg << columnSeparator << std::setw(floatWidth) << original.getFraction(ATOM);
			lg << columnSeparator << std::setw(floatWidth) << derAtom;
			lg << columnSeparator << std::setw(floatWidth) << derAtom / original.getFraction(ATOM);
			lg << '\n';
		}
		ok = false;
	}
	return ok;
}

//  ***************************************************************************
bool NeutronComposition::expandToBasic()
{
	//  New composition is weighted sum of components
	auto newComposition = Composition();
	//
	// expand each existing component and add to new composition
	//  e.g. 1000 -> 1001 and 1002
	//
	for (auto& currentComponent : components)
	{
		auto currentComponentMaterial = materialMap->find(currentComponent.getName());
		if (!currentComponentMaterial) return false;				// component material is not in library.
		// Use composition of current component.
		for (const Component& cpnt : currentComponentMaterial->getNeutronComposition().getComponents())
		{
			// each old component may have several pieces.  e.g. 1000 -> 1001 and 1002
			// New pieces should sum to 1.0
			// Use old fractions to weight the new piece as it is added.
			//
			auto newComponent = Component(cpnt.getName(), 0., 0.);
			newComponent.setFraction(MASS, cpnt.getFraction(MASS) * currentComponent.getFraction(MASS));
			newComponent.setFraction(ATOM,cpnt.getFraction(ATOM) * currentComponent.getFraction(ATOM));
			newComposition.addComponent(newComponent);
		}
	}
	components = newComposition.getComponents();
	return true;
}

//  ***************************************************************************
//  change zzaaaa to zz000.
//  If componentNames not in zzaaa form, just return neutron form.
//
PhotonComposition NeutronComposition::convertToPhotonForm() const
{
	PhotonComposition photonForm;

	for (const auto& comp : components)
	{
		std::string newName = "";
		okLong zz = toLongNullable(comp.getName());
		if (!zz.first or !zz.second.has_value())
		{
			newName = comp.getName();
		}
		else
		{
			newName = std::to_string((zz.second.value() / 1000) * 1000);
		}

		photonForm.addComponent(Component(newName, comp.getFraction(MASS), comp.getFraction(ATOM)));
	}
	return photonForm;
}

//  ***************************************************************************
PhotonComposition::PhotonComposition()
{

}

//  ***************************************************************************
bool PhotonComposition::areFractionsConsistent() const
{
	bool ok = Composition::areFractionsConsistent();		// Used for both neutrons snd photons
	if (!ok)
	{
		lg.bump(3) << ERR << "Problem with Photon Consistency\n";
	}
	return ok;
};

//  ***************************************************************************
Material::Material()
	:isBasicMat(false), comment(""), massDensity(std::nullopt), atomDensity(std::nullopt),
		atomicWeight(std::nullopt)
{
}


//  ***************************************************************************
Material::~Material()
{
}

//  ***************************************************************************
void Material::normalize()
{
//	 const NeutronComposition nComp = getNeutronComposition();

}

//  ***************************************************************************
const std::string Material::str()
{
	static std::ostringstream oss;
	oss.str("");
	oss << *this;
	return oss.str();
}

//  ***************************************************************************
bool Material::matchName(const std::string& testName) const
{
	if (name3.contains(testName)) return true;
	return false;
}

//  ***************************************************************************
bool Material::areDensitiesConsistent() const
{
	if (massDensity.has_value()  && atomDensity.has_value())
	{
		nDbl massFromAtom = computeDensity(MASS);
		nDbl atomFromMass = computeDensity(ATOM);
		if (!match(massFromAtom, massDensity) || !match(atomFromMass, atomDensity))
		{
			lg.bump(4) << ERR << "Densities do not match.\n";
			lg << STD << "As given:     mass density = " << std::left << std::setw(12);
			lg << massDensity << " atom density = " << atomDensity << "\n";
			lg << STD << "As computed:  mass density = " << std::setw(12);
			lg << massFromAtom  << " atom density = " << atomFromMass << "\n";
			lg << STD << "Ratios                       " << std::setw(12);
			lg << massDensity / massFromAtom << "                ";
			lg << std::setw(12) << atomDensity / atomFromMass << "\n";
			return false;
		}
	}
	return true;
}


//  ***************************************************************************
//  Listed atomic weight should be consistent with atomic weight computed from components
//
bool Material::isAtomicWeightConsistent() const
{
	if (isBasic()) return true;		// No components
	bool ok = true;
	if (atomicWeight.has_value())
	{
		auto atWt = neutronComposition.computeAtomicWeight();
		if (atWt.has_value() && !match(atWt, atomicWeight))
		{
			lg.bump(5) << ERR << "Given and computed atomic weights do not match\n";
			lg << STD << "Given: " << atomicWeight << "   ";
			lg << STD << "Calculated: " << atWt << "\n";
			ok = false;
		}
	}
	return ok;
}

//  ***************************************************************************
bool Material::doNeutronAndPhotonCompsMatch() const
{
	if (match(neutronComposition, photonComposition)) return true;
	auto derived = neutronComposition.convertToPhotonForm();
	if (match(derived, photonComposition)) return true;
	lg.bump(2) << ERR << "Neutron and photon compositions do not match.\n";
	lg << "photon composition derived from neutron composition\n";
	lg << derived << '\n';
	return false;
}

//  ***************************************************************************
bool Material::isCompositionDirectlyCircular(const Composition& compos) const
{
	for (auto& comp : compos.components)
	{
		if (name3.contains(comp.getName()))
		{
			lg.bump(3) << ERR << "Material may not be one of its own components.  ";
			lg << comp.getName() << "\n";
			return true;
		}
	}
	return false;
}

//  ***************************************************************************
//  Simple check for self-reference before adding to map.
//  Does not test for component referring to another material
//     which then refers back to this one.
//  This check allows neutron/photon composition inconsistency.
//
bool Material::isNameCircular() const
{
	if (isCompositionDirectlyCircular(neutronComposition)) return true;
	if (isCompositionDirectlyCircular(photonComposition)) return true;
	return false;
};

//  ***************************************************************************
void Material::cleanPhotonComposition()
{
//	auto& components = photonComposition.getComponents();
	if (photonComposition.getComponents().empty())
	{
		photonComposition = neutronComposition.convertToPhotonForm();
	}
	//  Verify component names are of form zzaaa.
	//  Do not expand photons because of possible problems with atomic weight.
	PhotonComposition newComposition;
	for (const Component& cmpnt : photonComposition.getComponents())
	{
		auto& currentName = cmpnt.getName();
		Component newComponent = cmpnt;			// non-const
		if (!Material3Names::isValidZZAAA(currentName, false))
		{ 
			auto mat = materialMap->find(currentName);
			if (mat && mat->isBasic())
			{
				newComponent.setName(mat->get3Names().getStringZZAAA());
			}
			else
			{
				lg.bump(2) << ERR << "Will not expand Photon name ";
				lg << currentName << '\n';
			}
		}
		std::string revisedName = newComponent.getName();		// Check if ends in "000"
		if (Material3Names::isValidZZAAA(revisedName, false))
		{
			revisedName = revisedName.substr(0, revisedName.size() - 3) + "000";
			newComponent.setName(revisedName);
		}
		newComposition.addComponent(newComponent);
	}
	photonComposition = newComposition;
}

//  ***************************************************************************
bool Material::sanityCheck() const
{
	if (!name3.sanityCheck()) return false;

	if (isBasicMat)
	{
		if (!neutronComposition.basicMaterialSanityCheck(name3.getStringZZAAA())) return false;
		if (!photonComposition.basicMaterialSanityCheck(name3.getStringZZAAA())) return false;
	}
	else
	{	//  not basic
		//  A material cannot be one of its components.
		//  This removes only direct references.
		//
		if ((neutronComposition.components.size() == 0) && (photonComposition.components.size() == 0))
		{
			lg.bump(3) << ERR << "Material must have a composition\n";
			return false;
		}
		if (isNameCircular()) return false;
		if (!neutronComposition.compositionSanityCheck()) return false;
		if (!photonComposition.compositionSanityCheck()) return false;
	}
	return true;
}


//  ***************************************************************************
//  Reset verbosity after each check. Get error messages only for checks that do not work.
//
bool Material::isMaterialSelfConsistent()
{
	bool ok = true;
	fillInMissingData();
	ok = areDensitiesConsistent();
	if (isBasicMat) return ok;
	// Only derived materials need be checked for other parameters
	//
	ok &= neutronComposition.areFractionsConsistent();
	ok &= photonComposition.areFractionsConsistent();
	ok &= doNeutronAndPhotonCompsMatch();
	ok &=  isAtomicWeightConsistent();
	return ok;
}


//  ***************************************************************************
//  Compute missing parts where possible. e.g. mass density from atom density.
//  Not all data is required for every use. E.g., no density for components.
//  Requirements for any possible use should be in sanity check.
//
bool Material::fillInMissingData()
{
	if (isBasicMat)				// Basic material might not have components
	{
		if (neutronComposition.getComponents().size() == 0)
		{
			auto nComponent = Component(name3.getStringZZAAA(), 1.0, 1.0);
			neutronComposition.addComponent(nComponent);
		}
		if (photonComposition.getComponents().size() == 0)
		{
			photonComposition = neutronComposition.convertToPhotonForm();
		}
	}		// Do not return here. Check density below.

	if (!isBasicMat)
	{
		if (!resolveToBasic())
		{
			lg.bump(3) << ERR << "Could not complete neutron composition in {" << name3 << "}\n";
			return false;
		}
		neutronComposition.sort();
		if (!neutronComposition.fillInMissingData())
		{
			lg.bump(3) << ERR << "Could not complete neutron composition in {" << name3 << "}\n";
			return false;
		}
		if (!atomicWeight)
		{
			atomicWeight = neutronComposition.computeAtomicWeight();
		}
		cleanPhotonComposition();		// Computes from neutron composition if empty
		photonComposition.sort();
	}
	// Check densities after composition and atomic weights are done.
	//
	if (!massDensity && atomDensity)
	{
		massDensity = computeDensity(MASS);
	}
	else if (massDensity && !atomDensity)
	{
		atomDensity = computeDensity(ATOM);
	}
	return true;
}


//  ***************************************************************************
std::ostream& operator<<(std::ostream& os, const Material& mat)
{
	os << "long name = " << mat.name3.getLongName() << '\n';
	os << "short name = " << mat.name3.getShortName() << '\n';
	os << "ZZAAA = " << mat.name3.getStringZZAAA() << '\n';
	os << "comment  = " << mat.comment << '\n';
	os << "isBasic = " << std::boolalpha << mat.isBasic() << '\n';
	os << "atomic weight = " << std::setprecision(6) << mat.atomicWeight << '\n';
	os << "massDensity = " << mat.massDensity << '\n';
	os << "atomDensity = " << mat.atomDensity << '\n';
	os << "Neutron Composition " << '\n' << mat.neutronComposition;
	os << "Photon Composition "  << '\n' << mat.photonComposition;
	return os;
}


