#pragma once
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <logger.h>
#include <optional>
#include <memory>
#include "nullableMath.h"
#include "utilities.h"


static const double AVOGADRO = 0.602214076;
enum FRACTION_TYPE { MASS, ATOM };

//  ***************************************************************************
//  component of a material. Has name, mass fraction, atom fraction
//  e.g. "C", 0.5, 0.6
//
class Component final
{
public:
	Component();
	~Component() {};
	Component(std::string name, nDbl massFraction, nDbl atomFraction);
	bool sanityCheck() const;
	bool setName(const std::string& name);					// Name may be zzaaa or string (e.g. "H-1")
	void setFraction(FRACTION_TYPE type, nDbl fraction );
	inline nDbl getFraction(FRACTION_TYPE type) const { return (type == MASS)?massFraction:atomFraction; } ;
	inline const std::string& getName() const { return name; };


	friend std::ostream& operator<<(std::ostream& os, const Component& component);

protected:
	bool isFractionValid(nDbl frac);

	std::string name;
	nDbl massFraction = std::nullopt;
	nDbl atomFraction = std::nullopt;
};

bool match(const Component& left, const Component& right);

//  ***************************************************************************
class Composition
{
public:
	Composition();
	~Composition() {};

	void addComponent(Component component);
	const Component* find(const std::string& name) const;
	std::pair<nDbl, nDbl> getSums() const;
	inline const std::vector <Component>& getComponents() const { return components; };
	void sort();

	std::string showMcnpFormat(int cross, int precision, NumFormat fmt) const;
	friend std::ostream& operator<<(std::ostream& os, const Composition& comp);

protected:
	std::vector <Component> components;

	bool setFractions(FRACTION_TYPE type, std::vector <nDbl>& newFractions);
	bool compositionSanityCheck() const;
	bool basicMaterialSanityCheck(const std::string& zzaaa) const;
	bool fillInMissingData();
	bool areFractionsConsistent() const;
	
	std::vector <nDbl> computeFractions(FRACTION_TYPE computedType) const;	// mass from atom and vice-versa

	void removeZeroFractions();
	friend class Material;
};

bool match(const Composition& left, const Composition& right);

//  ***************************************************************************
class NeutronComposition : public Composition
{
public:
	NeutronComposition();
	~NeutronComposition() {};

	Composition convertToPhotonForm();
	nDbl computeAtomicWeight() const;
	bool areFractionsConsistent() const;
	bool expandToBasic();

protected:

};

//  ***************************************************************************
class PhotonComposition : public Composition
{
public:
	PhotonComposition();
	~PhotonComposition() {};

	bool areFractionsConsistent() const;

protected:

};

//  ***************************************************************************
//  Material has 3 possible names: long, short, zzaaa
//  I use "" for no name, not nullopt. This seemed simpler.
//
class Material3Names final
{
public:
	Material3Names() { ; };
	~Material3Names() { ; };

	bool setLongName(const std::string& name);
	bool setShortName(const std::string& name);
	bool setZZAAA(const std::string& za);
	static bool isValidName(const std::string& name);
	static bool isValidZZAAA(const std::string& za, bool showMsg = true);

	inline const std::string& getLongName() const { return longName; } ;
	inline const std::string& getShortName() const { return shortName; };
	inline const std::string& getStringZZAAA() const { return zzaaa; };

	std::vector <std::string> getAllNames() const;
	bool sanityCheck() const;
	bool contains(const std::string& name) const;
	std::string str() const;			// returns all names with labels

	friend std::ostream& operator<<(std::ostream& os, const Material3Names& name3);
	friend bool operator == (const Material3Names& lhs, const Material3Names& rhs); // Used in unique
	friend bool operator != (const Material3Names& lhs, const Material3Names& rhs);
	static void sort(std::vector<Material3Names>& names, const std::string& order);

private:
	std::string zzaaa = "";
	std::string shortName = "";
	std::string longName = "";

};

//  ***************************************************************************
bool operator == (const Material3Names& lhs, const Material3Names& rhs);
inline bool operator != (const Material3Names& lhs, const Material3Names& rhs)
{
	return !(lhs == rhs);
};


//  ***************************************************************************
class Material
{
public:
	Material();
	~Material();

	bool matchName(const std::string& name) const;	// match any longName, shortName or ZZAAA;
	const Material3Names& get3Names() const { return name3; };
	inline bool isBasic() const { return isBasicMat; } ;
	void setBasic(bool opt) { isBasicMat = opt; };
	inline const std::string& getComment() const { return comment; };
	inline nDbl getAtomicWeight() const { return atomicWeight; };
	inline void setAtomicWeight(nDbl atWt) { atomicWeight = noNeg(atWt); };
	inline void setAtomDensity(nDbl atDensity) { atomDensity = noNeg(atDensity); };
	inline void setMassDensity(nDbl msDensity) { massDensity = noNeg(msDensity); };
	inline bool setZZAAA(const std::string& zzaaa) { return name3.setZZAAA(zzaaa); };
	inline bool setLongName(const std::string& name) { return name3.setLongName(name); };
	inline bool setShortName(const std::string& name) { return name3.setShortName(name); };
	inline void setComment(const std::string& newComment) { comment = newComment; };

	inline nDbl getAtomDensity() const { return atomDensity; };
	inline nDbl getMassDensity() const { return massDensity; };
	inline nDbl convertMassAtom(FRACTION_TYPE toType, nDbl fromVal) const
	{
		return (toType == MASS) ? fromVal * atomicWeight : fromVal / atomicWeight;
	}

	inline const Composition& getNeutronComposition() const { return neutronComposition; };
	inline const Composition& getPhotonComposition() const { return photonComposition; };
	inline void addNeutronComponent(Component component) { neutronComposition.addComponent(component); };
	inline void addPhotonComponent(Component component) { photonComposition.addComponent(component); };

	bool sanityCheck() const;			// Perform sanity check before adding to materialMap.
	bool fillInMissingData();
	bool isMaterialSelfConsistent();
	bool areDensitiesConsistent() const;
	bool isAtomicWeightConsistent() const;
	bool doNeutronAndPhotonCompsMatch();

	friend std::ostream& operator<<(std::ostream& os, const Material& mat);
	const std::string str();


protected:
	bool resolveToBasic();
	nDbl computeDensity(FRACTION_TYPE toType) const;
	bool isNameCircular() const;			// Does any component name match any of name3?
	bool isCompositionDirectlyCircular(const Composition& compos) const;
	void cleanPhotonComposition();			// does not expand. Possible issues with atomic weights.


	bool isBasicMat = false;		// has MCNP-useable zzaaa's. Not a compound. Do not expand.
	Material3Names name3;
	std::string  comment;
	std::optional<double> massDensity;
	std::optional<double> atomDensity;
	std::optional<double> atomicWeight;
	NeutronComposition neutronComposition;
	Composition photonComposition = Composition();
};

//  ***************************************************************************
class MaterialMap
{
public:
	MaterialMap();
	~MaterialMap();

	std::shared_ptr<Material> find(const std::string& name);
	bool addMaterial(std::shared_ptr<Material> pMat);
	bool removeMaterial(const Material3Names& mat3Name);
	std::vector < std::shared_ptr<Material> > uniqueMaterials() const;
	static std::vector < std::shared_ptr<Material> >
		sortByName(std::vector <std::shared_ptr<Material>> matVec, const std::string& nameOrder);
	std::vector <Material3Names> all3Names();

protected:
	std::unordered_map<std::string, std::shared_ptr<Material>> map;

	bool removeMaterial(const std::string& name);
};

//  ***************************************************************************
class MaterialFileReader
{
public:
	MaterialFileReader();
	virtual ~MaterialFileReader();
	bool readMaterialFile(const std::string& fileName);
	bool compareMaterialFile(const std::string& fileName);
	bool readNextMaterial();

protected:
	std::ifstream inFileMat;
	enum PROCESSING_STATUS { NEUTRON, PHOTON, NONE };
	std::shared_ptr<Material> mat;					// Material being constructed			
	PROCESSING_STATUS processingStatus = NONE;

	bool processLine(const std::string& line);
	std::pair <std::string, std::string> split2(const std::string& text);
	std::string bypassEquals(const std::string& line);
	std::optional <std::string> checkAndRemoveWord(const std::string& line, const std::string& word);
	okDbl toDoubleReader(const std::string& str);

	bool processIs(const std::string& line);
	bool processIsBasic(const std::string& line);
	bool processComment(const std::string& line);
	bool processLong(const std::string& line);
	bool processLongName(const std::string& line);
	bool processShort(const std::string& line);
	bool processShortName(const std::string& line);
	bool processAtomicWeight(const std::string& line);
	bool processMassDensity(const std::string& line);
	bool processAtomDensity(const std::string& line);
	bool processZZAAA(const std::string& line);

	bool processFractionText(const std::string& line);
	bool processIsotopeText(const std::string& line);
	bool processPhotonCompText(const std::string& line);
	bool processNeutronCompText(const std::string& line);
	bool tryProcessingComponent(std::pair <std::string, std::string>);
};

//  ***************************************************************************
void showMaterialComparison(std::shared_ptr<Material> lhs, std::shared_ptr<Material> rhs);