#include <set>
#include <algorithm>
#include "materials.h"
#include "utilities.h"

extern Logger lg;
std::unique_ptr<MaterialMap>  materialMap = std::make_unique<MaterialMap>();

//  ***************************************************************************
MaterialMap::MaterialMap()
{
	map.reserve(3000);
}

//  ***************************************************************************
MaterialMap::~MaterialMap()
{
}

//  ***************************************************************************
std::shared_ptr<Material> MaterialMap::find(const std::string& name)
{
	static std::set<std::shared_ptr <Material>> previousMaterials;
 	auto found = map.find(name);
	if (found == map.end())
	{
		lg.bump(2) << ERR << name << " not found.\n";
		return nullptr;
	}
	auto mat = found->second;
	auto different = previousMaterials.insert(mat);		// double ref => circular
	if (different.second == false)
	{
		lg.bump(MAX_VERBOSITY) << ERR << "Circular reference for\n" << *mat << "\n";
		lg << STD << "Already searched for:" << "\n";
		for (auto& previousMat : previousMaterials)
		{
			lg << STD << previousMat->get3Names() << "\n";
		}
		return nullptr;			// No need to remove name. It ws not added.
	}
	if (!mat->fillInMissingData())
	{
		previousMaterials.erase(mat);
		return nullptr;
	}
	previousMaterials.erase(mat);
  	return mat;
}

//  ***************************************************************************
bool MaterialMap::addMaterial(std::shared_ptr<Material> pMat)
{
	if (pMat == nullptr) return false;
	if (!pMat->sanityCheck()) return false;
	Material3Names matName = pMat->get3Names();
	bool removed = removeMaterial(matName);
	if (removed) lg.bump(1) << STD << matName << " added. overwrites.\n";
	for (auto& name : matName.getAllNames()) map.insert({ name, pMat });
	return true;
}

//  ***************************************************************************
bool MaterialMap::removeMaterial(const std::string& name)
{
	auto it = map.find(name);
	if (it == map.end()) return false;		// Not in map, can't remove
	auto matName = it->second->get3Names();
	map.erase(name);						// erase name (e.g. 1001)
	lg.bump(1) << STD << name << " removed.\n";
	removeMaterial(matName);				// erase alternate names (e.g. H-1)
	return true;
}

//  ***************************************************************************
bool MaterialMap::removeMaterial(const Material3Names& mat3Name)
{
	bool removed = false;
	for (const auto& name : mat3Name.getAllNames())
	{
		if (removeMaterial(name)) removed = true;
	}
	return removed;
}

//  ***************************************************************************
std::vector < std::shared_ptr<Material> > MaterialMap::uniqueMaterials() const
{
	// Copy materialMap values into a vector.
	std::vector<std::shared_ptr<Material>> mapValue;
	for (auto it = map.begin(); it != map.end(); ++it)
	{
		mapValue.push_back(it->second);
	}
	std::sort(mapValue.begin(), mapValue.end());		// sorting pointers, not objects.
	std::vector<std::shared_ptr<Material>>::iterator it;
	it = std::unique(mapValue.begin(), mapValue.end());  
	mapValue.resize(std::distance(mapValue.begin(), it));
	return mapValue;
}

//  ***************************************************************************
std::vector <std::shared_ptr<Material>>	MaterialMap::sortByName(
	std::vector <std::shared_ptr<Material>> matVec, const std::string& nameOrder)
{
	std::sort(matVec.begin(), matVec.end(), [nameOrder](const std::shared_ptr<Material>& lhs,
		const std::shared_ptr<Material>& rhs)
	{
		auto& lName = lhs->get3Names();
		auto& rName = rhs->get3Names();
		for (auto chr : nameOrder)
		{
			switch (chr)
			{
			case 'l':
				if (lName.getLongName() == rName.getLongName()) continue;
				if (myLess(lName.getLongName(), rName.getLongName())) return true;
				else return false;
			case 's':
				if (lName.getShortName() == rName.getShortName()) continue;
				if (myLess(lName.getShortName(), rName.getShortName())) return true;
				else return false;
			case 'z':
				if (lName.getStringZZAAA() == rName.getStringZZAAA()) continue;
				if (myLess(lName.getStringZZAAA(), rName.getStringZZAAA())) return true;
				else return false;
			}
		}
		return false;
	}
	);
	return matVec;
}

//  ***************************************************************************
std::vector <Material3Names> MaterialMap::all3Names()
{
	std::vector<Material3Names> names;
	for (auto& kv : map)
	{
		names.push_back(kv.second->get3Names());
	}
	return names;					// Map should not have any duplicates
}
