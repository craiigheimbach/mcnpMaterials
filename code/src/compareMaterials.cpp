#include <iomanip>
#include <set>

#include "utilities.h"
#include "materials.h"
#include "logger.h"


extern Logger lg;
extern std::unique_ptr<MaterialMap>  materialMap;

//  ***************************************************************************
void showMaterialComparison(std::shared_ptr<Material> lhs, std::shared_ptr<Material> rhs)
{
	// Define output layout
	auto wName = 15;
	auto wData = 31;
	auto precision = 6;
	auto wFrac = 16;
	const auto separator = " | ";

	if (lhs == nullptr || rhs == nullptr)
	{
		lg.bump(4) << ERR << "Cannot compare. One material is not defined." << '\n';
		return;
	}

	auto lhsNames = lhs->get3Names();
	auto rhsNames = rhs->get3Names();

	lg << std::left;
	lg << "                      original                          loaded                           Ratio\n";
	lg << std::setw(wName) << "long name" << separator << std::setw(wData) << truncate(wData, lhsNames.getLongName()) << separator;
	lg << std::setw(wData) << truncate(wData, rhsNames.getLongName()) << '\n';
	lg << std::setw(wName) << "short name" << separator << std::setw(wData) << truncate(wData, lhsNames.getShortName()) << separator;
	lg << std::setw(wData) << truncate(wData, rhsNames.getShortName()) << separator << '\n';
	lg << std::setw(wName) << "ZZAAA"      << separator << std::setw(wData) << lhsNames.getStringZZAAA() << separator;
	lg << std::setw(wData) << rhsNames.getStringZZAAA() << separator << '\n';
	lg << std::setw(wName) << "comment" << separator << std::setw(wData) << lhs->getComment() << separator;
	lg << std::setw(wData) << rhs->getComment() << separator << '\n';
	lg << std::setw(wName) << std::boolalpha << "is basic" << separator << std::setw(wData) << lhs->isBasic() << separator;
	lg << std::setw(wData) << rhs->isBasic() << separator << '\n';
	lg << std::defaultfloat << std::setprecision(precision);
	lg << std::setw(wName) << "atomic weight" << separator << std::setw(wData) << lhs->getAtomicWeight() << separator;
	lg << std::setw(wData) << rhs->getAtomicWeight() << separator;
	lg << std::setw(wData) << rhs->getAtomicWeight()/ lhs->getAtomicWeight() << '\n';

	lg << std::setw(wName) << "mass density" << separator << std::setw(wData) << lhs->getMassDensity() << separator;
	lg << std::setw(wData) << rhs->getMassDensity() << separator;
	lg << std::setw(wData) << rhs->getMassDensity() / lhs->getMassDensity() << '\n';

	lg << std::setw(wName) << "atom density" << separator << std::setw(wData) << lhs->getAtomDensity() << separator;
	lg << std::setw(wData) << rhs->getAtomDensity() << separator;
	lg << std::setw(wData) << rhs->getAtomDensity() / lhs->getAtomDensity() << '\n';

	lg << "Neutron composition" << "\n";
	lg << std::setw(wName) << "name" << separator << std::setw(wName)  << "Mass" << std::setw(wFrac)  <<  "Atom";
	lg << separator << std::setw(wName) << "Mass" << std::setw(wFrac) << "Atom";
	lg << separator << std::setw(wName) << "Mass" << std::setw(wFrac) << "Atom" << '\n';

	// Put all component names into a sorted set (no duplicates)
	auto cmp = [](const std::string& left, const std::string& right) { return myLess(left, right); };
	std::set <std::string, decltype(cmp)> jointNames(cmp);
	auto& lhsNeutronComposition = lhs->getNeutronComposition();
	auto& rhsNeutronComposition = rhs->getNeutronComposition();
	for (auto& component : lhsNeutronComposition.getComponents()) jointNames.insert(component.getName());
	for (auto& component : rhsNeutronComposition.getComponents()) jointNames.insert(component.getName());

	lg << std::fixed;
	for (auto& name : jointNames)
	{
		std::string compName = name;
		auto lhsCompo = lhsNeutronComposition.find(name);
		auto rhsCompo = rhsNeutronComposition.find(name);
		auto lComp = lhs->getNeutronComposition().getComponents();
		lg << std::setw(wName) << std::right << name << separator << std::left;
		if (lhsCompo == nullptr) lg << std::setw(wData) << "";
		else lg << std::setw(wName) << lhsCompo->getFraction(MASS) << std::setw(wFrac) << lhsCompo->getFraction(ATOM);	
		lg << separator;
		if (rhsCompo == nullptr) lg << std::setw(wData) << "";
		else lg << std::setw(wName) << rhsCompo->getFraction(MASS) << std::setw(wFrac) << rhsCompo->getFraction(ATOM);
		lg << separator;
		if (lhsCompo == nullptr || rhsCompo == nullptr) lg << std::setw(wName) << "-" << std::setw(wFrac) << "-";
		else
		{
			lg << std::setw(wName) << lhsCompo->getFraction(MASS) / lhsCompo->getFraction(MASS);
			lg << std::setw(wFrac) << lhsCompo->getFraction(ATOM) / lhsCompo->getFraction(ATOM);
		}
		lg << '\n';
	}
	lg << '\n';
}