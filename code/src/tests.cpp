#include<string>
//#include <cmath>
#include "tests.h"
#include "utilities.h"
#include "nullableMath.h"
#include "materials.h"
#include <iostream>
#include "logger.h"

Logger lgT;					// Logger for tests. Goes to a separate file.
extern Logger lg;
extern std::unique_ptr<MaterialMap>  materialMap;
bool errorsInTesting = false;

//  ***************************************************************************
template <typename T>
std::string vectorToString(std::vector<T> vec)
{
	std::ostringstream oss;
	oss << "{ ";
	for (auto& item : vec)
	{
		oss << item << ' ';
	}
	oss << "}\n";
	return oss.str();
}

//  ***************************************************************************
std::string details(const Component& component)
{
	std::ostringstream oss;
	oss << "{ name: " << component.getName() << "  mass: " <<
		component.getFraction(MASS) << "  atom: " << component.getFraction(ATOM) << " }";
	return oss.str();
}

//  ***************************************************************************
void handleError()
{
	if (lgT.getVerbosity() > 0)
	{
		errorsInTesting = true;
		lgT.writeToLog();
	}
	else
	{
		lgT.clear();
	}
	lg.clear();		// remove errors auto-generated in othe modules. 
}


//  ***************************************************************************
void runTests()
{
	std::unique_ptr<MaterialMap>  testMap = std::make_unique<MaterialMap>();
	testMap.swap(materialMap);

	lgT.setLoggerFileName("test_out.txt");
	lgT.setCoutVerbosity(4);		// put to screen. Skip boilerplate.
	lgT.setFileVerbosity(4);		// put to file. Skip boilerplate.
	handleError();

	runUtilitiesTests();
	runNullableMathTests();
	runMaterialTests();
	runFunctionalTests();
	lg.clear();				// clean up messages created during testing.
	if (errorsInTesting)
	{
		lg.bump(MAX_VERBOSITY) << ERR << "Error in testing. See test_out.txt file.\n";
	}

	testMap.swap(materialMap);
}

//  ***************************************************************************
void runUtilitiesTests()
{
	testTrim();
	testSplit();
	testTruncate();
	testRemoveComments();
	testToLower();
	testMatchAny();
	testMyLess();
}

//  ***************************************************************************
void testTrim()
{
	lgT << MNR << "trim test\n";
	std::string tStr{ " \n\t   a aa \n      " };
	std::string tStr1{ "\" \\n\\t   a aa \\n       \"" };

	if (ltrim(tStr) != "a aa \n      ")
	{
		lgT.bump(1) << ERR << "ltrim failed to trim leading spaces.";
	}
	if (rtrim(tStr) != " \n\t   a aa")
	{
		lgT.bump(1) << ERR << "rtrim failed to trim trailing spaces.";
	}
	if (trim(tStr) != "a aa")
	{
		lgT.bump(1) << ERR << "trim failed to trim leading or trailing spaces.";
	}
	tStr = "1234";
	if (trim(tStr) != "1234")
	{
		lgT.bump(1) << ERR << "trim failed to trim '1234'.";
	}
	handleError();
}

//  ***************************************************************************
void testSplit()
{
	lgT << MNR << "split test\n";
	std::string tStr = "  aaa bbb c\tcc \n dd";
	auto spl = split(tStr);
	if (spl.size() != 5 || spl[0] != "aaa" || spl[1] != "bbb" ||
		spl[2] != "c" || spl[3] != "cc" || spl[4] != "dd")
	{
		lg.bump(1) << ERR << "split   'aaa bbb c\tcc \n dd' incorrectly" << '\n';
		lgT << STD << "split into " << spl.size() << "pieces.\n";
		for (size_t ia = 0; ia < spl.size(); ++ia)
		{
			lgT << STD << spl[ia] << '\n';
		}
		lgT << '\n';
	}
	handleError();
}

//  ***************************************************************************
void testTruncate() 
{
	lgT << MNR << "truncate test\n";
	std::string  tStr = "0123456789";
	if (truncate(10, tStr) != "0123456789")
	{
		lgT.bump(1) << ERR << "truncate(10, '0123456789') gave "
			<< truncate(10, tStr) << " not '0123456789'\n";
	}
	if (truncate(9, tStr) != "012345...")
	{
		lgT.bump(1) << ERR << "truncate(9, '0123456789') gave '"
			<< truncate(10, tStr) << "' not '012345...'\n";
	}
	if (truncate(3, tStr) != "...")
	{
		lgT.bump(1) << ERR << "truncate(3, '0123456789') gave '"
			<< truncate(3, tStr) << "' not '...'\n";
	}
	if (truncate(2, tStr) != "..")
	{
		lgT.bump(1) << ERR << "truncate(2, '0123456789') gave '"
			<< truncate(3, tStr) << "' not '..'\n";
	}
	if (truncate(1, tStr) != ".")
	{
		lgT.bump(1) << ERR << "truncate(1, '0123456789') gave '"
			<< truncate(1, tStr) << "' not '.'\n";
	}
	if (truncate(0, tStr) != "")
	{
		lgT.bump(1) << ERR << "truncate(0, '0123456789') gave '"
			<< truncate(0, tStr) << ". not ''\n";
	}
	handleError();
}

//  ***************************************************************************
void testRemoveComments()
{
	lgT << MNR << "remove comments test\n";
	std::string tStr = " aaa//abz\t\n";
	if (removeComments(tStr) != "aaa")
	{
		lgT.bump(1) << ERR << "(removeComments( aaa//abz\\t\\n) gave '"
			<< removeComments(tStr) << "' not 'aaa'\n";
	}
	tStr = " aaa/abz\t\n";
	if (removeComments(tStr) != "aaa/abz")
	{
		lgT.bump(1) << ERR << "(removeComments( aaa/abz\\t\\n) gave '"
			<< removeComments(tStr) << "' not 'aaa/abz'\n";
	}
	tStr = "aaa  //abz\t\n";
	if (removeComments(tStr) != "aaa")
	{
		lgT.bump(1) << ERR << "(removeComments(aaa  //abz\t\n) gave '"
			<< removeComments(tStr) << "' not 'aaa'\n";
	}
	handleError();
}

//  ***************************************************************************
void testToLower()
{
	lgT << MNR << "toLower test\n";
	std::string tStr = " Aa,; bbBBB";
	if (toLower(tStr) != " aa,; bbbbb")
	{
		lgT.bump(1) << ERR << "(toLower(' Aa,; bbBBB') gave '"
			<< toLower(tStr) << "' not ' aa,; bbbbb'\n";
	}
	handleError();
}

//  ***************************************************************************
void testMatchAny()
{
	lgT << MNR << "matchAny test\n";
	std::vector<std::string> okValues{ "aa", "BB" };
	if (!matchAny("aa", okValues))
	{
		lgT.bump(1) << ERR << "Failed to match 'aa' in " << vectorToString(okValues) << '\n';
	}
	if (matchAny("Aa", okValues))
	{
		lgT.bump(1) << ERR << "Matched 'Aa' in " << vectorToString(okValues) << '\n';
	}
	if (matchAny(" aa", okValues))
	{
		lgT.bump(1) << ERR << "Matched ' aa' in " << vectorToString(okValues) << '\n';
	}
	if (!matchAny("BB", okValues))
	{
		lgT.bump(1) << ERR << "Failed to match 'BB' in " << vectorToString(okValues) << '\n';
	}
	handleError();
}

//  ***************************************************************************
void testMyLess()
{
	lgT << MNR << "myLess test\n";
	if (!myLess("a", "b"))
	{
		lgT.bump(1) << ERR << "myLess('a', 'b') is false'\n'";
	}
	if (myLess("b", "b"))
	{
		lgT.bump(1) << ERR << "myLess('b', 'b') is true'\n'";
	}
	if (myLess("b", "a"))
	{
		lgT.bump(1) << ERR << "myLess('b', 'a') is true'\n'";
	}
	if (!myLess("111", "a"))
	{
		lgT.bump(1) << ERR << "myLess('111', 'a') is false'\n'";
	}
	if (myLess(" ", "a"))
	{
		lgT.bump(1) << ERR << "myLess(' ', 'a') is true'\n'";
	}
	if (myLess("A", "a") || myLess("a", "A"))	// not case sensitive
	{
		lgT.bump(1) << ERR << "myLess('A', 'a') is false'\n'";
	}
	if (!myLess("a", "aaa"))
	{
		lgT.bump(1) << ERR << "myLess('a', 'aaa') is false'\n'";
	}
	if (!myLess("0", "1"))
	{
		lgT.bump(1) << ERR << "myLess('0', '1') is false'\n'";
	}
	if (!myLess("2", "11"))
	{
		lgT.bump(1) << ERR << "myLess('2', '1') is false'\n'";
	}
	if (myLess("2", "2"))
	{
		lgT.bump(1) << ERR << "myLess('2', '2') is true'\n'";
	}
	if (!myLess("2", ""))
	{
		lgT.bump(1) << ERR << "myLess('2', '') is false'\n'";
	}
	if (!myLess("2", "2.0001"))
	{
		lgT.bump(1) << ERR << "myLess('2', '2.0001') is false'\n'";
	}
	handleError();
}

//  ***************************************************************************
void runNullableMathTests()
{
	lgT << MNR << "nullable math test\n";
	testToLongNullable();
	testToDoubleNullable();

	//  Check match
	//
	double allowed = getPrecisionForMatching();
	double newError = 0.01;
	setPrecisionForMatching(newError);
 	if (match(6.0, 5.94) != false)
	{
		lgT.bump(1) << ERR << "match(6.0, 5.94) => true. error = 0.01\n";
	}
	if (match(6.0, 5.95) != true)
	{
		lgT.bump(1) << ERR << "match(6.0, 5.94) => false. error = 0.01\n";
	}
	if (match(std::nullopt, std::nullopt) != true)
	{
		lgT.bump(1) << ERR << "match(nullopt, nullopt) => false.\n";
	}
	if (match(3.0, std::nullopt) == true)
	{
		lgT.bump(1) << ERR << "match(3.0, nullopt) => true.\n";
	}
	//  Divide by zero
	auto top = (nDbl)3.0;
	auto bottom  = (nDbl)0;
	auto divided = top / bottom;

	if (divided.has_value() && !std::isinf(divided.value()) )
	{
		lgT.bump(1) << ERR << "nDbl(3.0) / nDbl(0) => " << divided << '\n';
	}
	setPrecisionForMatching(allowed);
	handleError();
}

//  ***************************************************************************
void testToLongNullable()
{
	lgT << MNR << "toLongNullable test\n";
	auto testConvert = toLongNullable(" -6 ");
	if (testConvert.first != true || testConvert.second != -6)
	{
		lgT.bump(1) << ERR << "toLongNullable(' -6 ') => " << testConvert.first <<
			"  " << testConvert.second << '\n';
	}
	testConvert = toLongNullable(" +6");
	if (testConvert.first != true || testConvert.second != 6)
	{
		lgT.bump(1) << ERR << "toLongNullable(' +6') => " << testConvert.first <<
			"  " << testConvert.second << '\n';
	}
	testConvert = toLongNullable("1001");
	if (testConvert.first != true || testConvert.second != 1001)
	{
		lgT.bump(1) << ERR << "toLongNullable('1001') => " << testConvert.first <<
			"  " << testConvert.second << '\n';
	}
	testConvert = toLongNullable("");
	if (testConvert.first != true || testConvert.second != std::nullopt)
	{
		lgT.bump(1) << ERR << "toLongNullable('') => " << testConvert.first <<
			"  " << testConvert.second << '\n';
	}
	testConvert = toLongNullable("1.0");
	if (testConvert.first != false || testConvert.second != std::nullopt)
	{
		lgT.bump(1) << ERR << "toLongNullable('1.0') => " << testConvert.first <<
			"  " << testConvert.second << '\n';
	}
	testConvert = toLongNullable("1.01");
	if (testConvert.first != false || testConvert.second != std::nullopt)
	{
		lgT.bump(1) << ERR << "toLongNullable('1.0') => " << testConvert.first <<
			"  " << testConvert.second << '\n';
	}
	testConvert = toLongNullable(".1");
	if (testConvert.first != false || testConvert.second != std::nullopt)
	{
		lgT.bump(1) << ERR << "toLongNullable('.1') => " << testConvert.first <<
			"  " << testConvert.second << '\n';
	}
	testConvert = toLongNullable("0.1");
	if (testConvert.first != false || testConvert.second != std::nullopt)
	{
		lgT.bump(1) << ERR << "toLongNullable('0.1') => " << testConvert.first <<
			"  " << testConvert.second << '\n';
	}
	testConvert = toLongNullable("cat");
	if (testConvert.first != false || testConvert.second != std::nullopt)
	{
		lgT.bump(1) << ERR << "toLongNullable('cat') => " << testConvert.first <<
			"  " << testConvert.second << '\n';
	}
	testConvert = toLongNullable("123a");
	if (testConvert.first != false || testConvert.second != std::nullopt)
	{
		lgT.bump(1) << ERR << "toLongNullable('123a') => " << testConvert.first <<
			"  " << testConvert.second << '\n';
	}
	handleError();
}

//  ***************************************************************************
void testToDoubleNullable()
{
	lgT << std::boolalpha;
	lgT << MNR << "toDoubleNullable test\n";
	auto testConvert = toDoubleNullable(" -6");
	if (testConvert.first != true || !match(testConvert.second, -6))
	{
		lgT.bump(1) << ERR << "toDoubleNullable(' -6') => " << testConvert.first <<
			"  " << testConvert.second << '\n';
	}
	testConvert = toDoubleNullable(" +6");
	if (testConvert.first != true || !match(testConvert.second, +6))
	{
		lgT.bump(1) << ERR << "toDoubleNullable(' +6') => " << testConvert.first <<
			"  " << testConvert.second << '\n';
	}
	testConvert = toDoubleNullable("1001");
	if (testConvert.first != true || !match(testConvert.second, 1001))
	{
		lgT.bump(1) << ERR << "toLongNullable('1001') => " << testConvert.first <<
			"  " << testConvert.second << '\n';
	}
	testConvert = toDoubleNullable("");
	if (testConvert.first != true || !match(testConvert.second, std::nullopt))
	{
		lgT.bump(1) << ERR << "toLongNullable('') => " << testConvert.first <<
			"  " << testConvert.second << '\n';
	}
	testConvert = toDoubleNullable("1.0");
	if (testConvert.first == false || !match(testConvert.second, 1.0))
	{
		lgT.bump(1) << ERR << "toLongNullable('1.0') => " << testConvert.first <<
			"  " << testConvert.second << '\n';
	}
	testConvert = toDoubleNullable("1.01");
	if (testConvert.first == false || !match(testConvert.second, 1.01))
	{
		lgT.bump(1) << ERR << "toLongNullable('1.01') => " << testConvert.first <<
			"  " << testConvert.second << '\n';
	}
	testConvert = toDoubleNullable(".1");
	if (testConvert.first == false || !match(testConvert.second, 0.1))
	{
		lgT.bump(1) << ERR << "toLongNullable('.1') => " << testConvert.first <<
			"  " << testConvert.second << '\n';
	}
	testConvert = toDoubleNullable("0.1");
	if (testConvert.first == false || !match(testConvert.second, 0.1))
	{
		lgT.bump(1) << ERR << "toLongNullable('0.1') => " << testConvert.first <<
			"  " << testConvert.second << '\n';
	}
	testConvert = toDoubleNullable("cat");
	if (testConvert.first != false || !match(testConvert.second, std::nullopt))
	{
		lgT.bump(1) << ERR << "toLongNullable('cat') => " << testConvert.first <<
			"  " << testConvert.second << '\n';
	}
	testConvert = toDoubleNullable("-.2e5");
	if (testConvert.first == false || !match(testConvert.second, -.2e5))
	{
		lgT.bump(1) << ERR << "toLongNullable('-.2e5') => " << testConvert.first <<
			"  " << testConvert.second << '\n';
	}
	testConvert = toDoubleNullable(".2E5");
	if (testConvert.first == false || !match(testConvert.second, 0.2e5))
	{
		lgT.bump(1) << ERR << "toLongNullable('.2E5') => " << testConvert.first <<
			"  " << testConvert.second << '\n';
	}
	testConvert = toDoubleNullable("123a", false);
	if (testConvert.first != false || testConvert.second != std::nullopt)
	{
		lgT.bump(1) << ERR << "toDoubleNullable('123a') => " << testConvert.first <<
			"  " << testConvert.second << '\n';
	}

	nDbl d1 = 1.0;
	nDbl d2 = 2.0;
	nDbl d3 = 3.0;
	nDbl d4 = 4.0;

	nDbl result = (d3 +d2 -d1) / d4;
	if (!match(result.value(), 1.0))
	{
		lgT.bump(1) << ERR << "Error in nullable double math. 1.0 != " << result;
	}

	handleError();
}


//  ***************************************************************************
void runMaterialTests()
{
	testNames();
	testComponent();
	testComposition();
	testMassAtomConversion();
}

//  ***************************************************************************
void testComponent()
{
	//  Test constructor
	auto cmpnt1 = Component("name", 1.0, std::nullopt);
	if (cmpnt1.getName() != "name" || cmpnt1.getFraction(MASS) != 1.0 ||
		cmpnt1.getFraction(ATOM) != std::nullopt)
	{
		lgT.bump(1) << ERR << "      expected   found    in component test\n";
		lgT << ERR << "name  " << "name     " << cmpnt1.getName() << '\n';
		lgT << ERR << "massF " << "1.0      " << cmpnt1.getFraction(MASS) << '\n';
		lgT << ERR << "atomF " << "-      " << cmpnt1.getFraction(ATOM) << '\n';
	}

	// Test read/write component values
	cmpnt1.setFraction(MASS, std::nullopt);
	cmpnt1.setFraction(ATOM, 1.0);
	if (cmpnt1.getFraction(MASS) != std::nullopt || cmpnt1.getFraction(ATOM) != 1.0)
	{
		lgT.bump(1) << ERR << "      expected   found    in component test\n";
		lgT << ERR << "name  " << "name     " << cmpnt1.getName() << '\n';
		lgT << ERR << "massF " << " -       " << cmpnt1.getFraction(MASS) << '\n';
		lgT << ERR << "atomF " << "1.0      " << cmpnt1.getFraction(ATOM) << '\n';
	}
	std::ostringstream oss;
	oss << cmpnt1;
	auto str = oss.str();
	auto spl = split(str);					// OK if spacing changes
	if (spl.size() != 3)
	{
		lgT.bump(1) << ERR << "in component output, should have three items: name '-' 1 \n";
		lgT << ERR << "had:\n ";
		for (const auto& item : spl)
		{
			lgT << ERR << item << '\n';
		}
	}
	if (spl.size() != 3 || spl[0] != "name" || spl[1] != "-" || spl[2] != "1")
	{
		lgT.bump(1) << ERR << "in component output\n";
		lgT << ERR << "should have name, -, 1\n";
		lgT << ERR << "had '" << spl[0] << ", " << spl[1] << ", " << spl[2] << '\n';
	}
	auto cmpnt2 = Component("name", std::nullopt, std::nullopt);
	if (match(cmpnt1, cmpnt2))
	{
		lgT.bump(2) << ERR << details(cmpnt1) << " == " << details(cmpnt2) << '\n';
	}
	cmpnt2 = Component("name2", 1.0, std::nullopt);
	if (match(cmpnt1, cmpnt2))
	{
		lgT.bump(2) << ERR << details(cmpnt1) << " == " << details(cmpnt2) << '\n';
	}
	// Check setters
	cmpnt2 = Component("name2", 2.0, 2.0);
	cmpnt2.setName("name");
	cmpnt2.setFraction(ATOM, 1.0);
	cmpnt2.setFraction(MASS, std::nullopt);
	if (!match(cmpnt1, cmpnt2))
	{
		lgT.bump(2) << ERR << "Setters did not work.  " <<
			details(cmpnt1) << " != " << details(cmpnt2) << '\n';
	}
	// Component name must be a valid name or valid zzaaa
	cmpnt2.setName(" 22000 ");
	if (cmpnt2.getName() != "22000")
	{
		lgT.bump(1) << "component setName does not accepts '22000' input \n";
	}
	cmpnt2.setName("H 1");
	if (cmpnt2.getName() != "")
	{
		lgT.bump(1) << "component setName accepts 'H 1' input \n";
	}
	cmpnt2.setName("Hydrogen-1");
	if (cmpnt2.getName() != "Hydrogen-1")
	{
		lgT.bump(1) << "component setName does not accepts 'Hydrogen-1' input \n";
	}
	cmpnt2.setName("20");
	if (cmpnt2.getName() != "")
	{
		lgT.bump(1) << "component setName accepts '20' input. (<1000) \n";
	}
	cmpnt2.setName("20z");
	if (cmpnt2.getName() != "")
	{
		lgT.bump(1) << "component setName accepts '20z' input." <<
			" (zzaaa or start with alphabetic char) \n";
	}
	// test bad inputs (negative is rejected. 0.0 or >1 are accepted.)
	auto cmpnt3 = Component("bad", -.01, 1.01);
	if (cmpnt3.getFraction(MASS) != std::nullopt)
	{
		lgT.bump(1) << ERR << "negative component fraction was accepted\n";
	}
	if (!match(cmpnt3.getFraction(ATOM), 1.01))
	{
		lgT.bump(1) << ERR << "component fraction > 1.0 was rejected\n";
	}
	cmpnt3 = Component("bad", 0.0, 0.0);
	if (!match(cmpnt3.getFraction(MASS), 0.0) || !match(cmpnt3.getFraction(ATOM), 0.0))
	{
		lgT.bump(1) << ERR << "component fraction == 0.0 was rejected\n";
	}
	handleError();
}

//  ***************************************************************************
void testComposition()
{
	Composition cmp;
	cmp.addComponent(Component("B", 0.7, 0.1));
	cmp.addComponent(Component("B", 0.1, 0.1));
	cmp.addComponent(Component("A", 0.2, 0.8));
	auto sum = cmp.getSums();
	if (!sum.mass.has_value())
	{
		lgT.bump(1) << ERR << "In testComposition, mass fractions show as not complete.\n";
		lgT << ERR << cmp << '\n';
	}
	if (!sum.atom.has_value())
	{
		lgT.bump(1) << ERR << "In testComposition, atom fractions show as not complete.\n";
		lgT << ERR << cmp;
	}

	Composition cmp1;
	cmp1.addComponent(Component("B", 0.7, 0.7));
	cmp1.addComponent(Component("A", 0.4, 0.2));		// sums != 1.0
	sum = cmp1.getSums();
	if (!sum.mass.has_value())
	{
		lgT.bump(1) << ERR << "mass fractions show as invalid, should be 1.1\n";
		lgT << ERR << cmp1 << '\n';
	}
	else
	{
		if (!match(sum.mass, 1.1))
		{
			lgT.bump(1) << ERR << "mass fractions do not sum to 1.1\n";
			lgT << ERR << cmp1 << '\n';
		}
	}
	if (!sum.atom.has_value())
	{
		lgT.bump(1) << ERR << "mass fractions show as invalid, should be 0.9\n";
		lgT << ERR << cmp1 << '\n';
	}
	else
	{
		if (!match(sum.atom, 0.9))
		{
			lgT.bump(1) << ERR << "atom fractions do not sum to 0.9\n";
			lgT << ERR << cmp1 << '\n';
		}
	}

	Composition cmp2;
	cmp2.addComponent(Component("B", std::nullopt, std::nullopt));
	sum = cmp2.getSums();
	if (sum.mass.has_value())
	{
		lgT.bump(1) << ERR << "In testComposition cmp2, mass fraction sum is valid.\n";
		lgT << STD << cmp2 << '\n';
	}
	if (sum.atom.has_value())
	{
		lgT.bump(1) << ERR << "In testComposition cmp2, atom fractions sum is valid.\n";
		lgT << STD << cmp2 << '\n';
	}
	handleError();
}

//  ***************************************************************************
void testNames()
{
	lgT << MNR << "name test\n";
	auto name3 = Material3Names();
	name3.setLongName("Neon");
	name3.setShortName("Ne");
	name3.setZZAAA("10000");
	if (name3.getLongName() != "Neon" || name3.getShortName() != "Ne" ||
		name3.getStringZZAAA() != "10000")
	{
		lgT.bump(1) << ERR << "(Neon, Ne, 10000) => " << vectorToString(name3.getAllNames()) << "\n";
	}
	name3.setLongName("");
	if (name3.getLongName() != "" || name3.getShortName() != "Ne" ||
		name3.getStringZZAAA() != "10000")
	{
		lgT.bump(1) << ERR << "('', Ne, 10000) => " << vectorToString(name3.getAllNames()) << "\n";
	}
	name3.setLongName("Neon");
	name3.setShortName("None");
	name3.setZZAAA("-");
	if (name3.getLongName() != "Neon" || name3.getShortName() != "" ||
		name3.getStringZZAAA() != "")
	{
		lgT.bump(1) << ERR << "('Neon', '', '') => " << vectorToString(name3.getAllNames()) << "\n";
	}
	name3.setLongName("5555");
	if (name3.getLongName() != "")
	{
		lgT.bump(1) << ERR << "setLongName accepted '5555'\n";
	}
	name3.setLongName("long");
	if (name3.getLongName() != "")
	{
		lgT.bump(1) << ERR << "setLongName accepted 'long'\n";
	}
	name3.setZZAAA("1001 1");
	if (name3.getStringZZAAA() != "")
	{
		lgT.bump(1) << ERR << "SetZZAAA accepted name with spaces " << "1001 1" << "\n";
	}
	name3.setLongName("U23 5");
	if (name3.getLongName() != "")
	{
		lgT.bump(1) << ERR << "SetLongName accepted name with spaces " << "U23 5" << "\n";
	}
	handleError();
}

//  ***************************************************************************
void testMassAtomConversion()
{
	lgT << MNR << "Mass and atom density test\n";
	Material mat;
	mat.setAtomicWeight(2.0);
	mat.setAtomDensity(1.0 * AVOGADRO);
	mat.setMassDensity(2.0);
	nDbl massD = mat.getAtomDensity() * mat.getAtomicWeight() / AVOGADRO;
	if (massD != mat.getMassDensity())
	{
		lgT.bump(1) << ERR << "Incorrect conversion from atom to mass density.\n";
		lgT << ERR << "Found: " << massD << "   Should be: " << mat.getMassDensity();
	}
	nDbl atomD = mat.getMassDensity() * AVOGADRO / mat.getAtomicWeight();
	if (atomD != mat.getAtomDensity())
	{
		lgT.bump(1) << ERR << "Incorrect conversion from mass to atom density.\n";
		lgT << ERR << "Found: " << atomD << "   Should be: " << mat.getAtomDensity();
	}
	handleError();
}

//  ***************************************************************************
void runFunctionalTests()
{
	// Functional tests require a material
	//
	functionalTest1();
	functionalTest2();
	functionalTest3();
}

//  ***************************************************************************
//  Test of a basic material
//
void functionalTest1()
{
	lgT << MNR << "functional test 1\n";

	MaterialFileReader materials;
	auto fileName = "test_in.txt";
	if (!materials.readMaterialFile("test_in.txt"))
	{
		lgT.absorb(lg);
		lgT.bump(1) << ERR << "Could not read material file " << fileName << '\n';
		lgT << STD << "Test aborted.\n";
		return;
	}

	//  Basic test of material read.
	auto mat = materialMap->find("Al");
	if (mat == nullptr)
	{
		lgT.bump(5) << ERR << "Could not find 'Al'\n";
		lgT.absorb(lg);
	}
	else
	{
		auto& components = mat->getNeutronComposition().getComponents();
		auto& comp = components[0];
		if (comp.getName() != "13027" || !match(comp.getFraction(MASS), 1.0)
			|| !match(comp.getFraction(ATOM), 1.0))
		{
			lgT.bump(3) << ERR << "Aluminum component should be '13027  1.0  1.0', found '" <<
				comp.getName() << "  " << comp.getFraction(MASS) << "  " <<
				comp.getFraction(ATOM) << "'" << '\n';
		}
		if (!mat->isMaterialSelfConsistent())
		{
			lgT.bump(1) << ERR << "Aluminum mass and atom densities were found inconsistent.\n";
			lgT << STD << "mass density: " << mat->getMassDensity();
			lgT << STD << "    atom density: " << mat->getAtomDensity() << '\n';
		}
		mat->setAtomDensity(1.0);		// not consistent
		if (mat->isMaterialSelfConsistent())
		{
			lgT.bump(1) << ERR << "Incorrect Aluminum mass and atom densities were found consistent.\n";
			lgT << STD << "mass density: " << mat->getMassDensity();
			lgT << STD << "    atom density: " << mat->getAtomDensity() << '\n';
		}
		mat->setAtomDensity(std::nullopt);		// nullopt is consistent
 		bool err = mat->areDensitiesConsistent();
		if (!err)
		{
			lgT.bump(1) << ERR << "Aluminum mass and atom densities were found inconsistent.\n";
			lgT << STD << "mass density: " << mat->getMassDensity();
			lgT << STD << "    atom density: " << mat->getAtomDensity() << '\n';
		}

		mat->fillInMissingData();		// compute atom density
		if (!match(0.060238, mat->getAtomDensity()))
		{
			lgT.bump(1) << ERR << "Failed to compute aluminum atom density\n";
			lgT << STD << "correct atom density = 0.060238\n";
			lgT << STD << "    computed atom density: " << mat->getAtomDensity() << '\n';
		}
		mat->setMassDensity(std::nullopt);				// round trip for density
		mat->fillInMissingData();
		if (!match(2.6989, mat->getMassDensity()))
		{
			lgT.bump(1) << ERR << "Failed to compute aluminum mass density\n";
			lgT << STD << "correct mass density = 2.6989 ";
			lgT << STD << "    computed mass density: " << mat->getAtomDensity() << '\n';
		}
	}
	handleError();;
}

//  ***************************************************************************
//  Test of a compound material. H = H1 + H2
//
void functionalTest2()
{
	lgT << MNR << "functional test 2\n";

	lg.clear();
	auto mat_H1 = materialMap->find("H1");
	if (mat_H1 == nullptr)
	{
		lgT.bump(5) << ERR << "Could not find 'H1'\n";
		lgT.absorb(lg);
		lgT << STD << "Could not test compound material.";
		return;
	}

	// Test that H1 isotope was created correctly
	auto& neutronComp = mat_H1->getNeutronComposition().getComponents();
	if (neutronComp.size() != 1)
	{
		lgT.bump(1) << ERR << "H1 neutron composition size should be 1. It is  " <<
			neutronComp.size() << '\n';
		return;
	}
	auto cmpnt = neutronComp[0];
	if (cmpnt.getName() != "1001" || !match(cmpnt.getFraction(MASS), 1.0) ||
		!match(cmpnt.getFraction(ATOM), 1.0))
	{
		lgT.bump(1) << ERR << "H1 neutron component should be ('1001', 1.0, 1.0). It is "
			<< cmpnt << '\n';
	}
	auto& photonComp = mat_H1->getNeutronComposition().getComponents();
	if (photonComp.size() != 1)
	{
		lgT.bump(1) << ERR << "H1 photon composition size should be 1. It is  " <<
			photonComp.size() << '\n';
		return;
	}
	cmpnt = photonComp[0];
	if (cmpnt.getName() != "1001" || !match(cmpnt.getFraction(MASS), 1.0) ||
		!match(cmpnt.getFraction(ATOM), 1.0))
	{
		lgT.bump(1) << ERR << "H1 photon component should be ('1001', 1.0, 1.0). It is "
			<< cmpnt << '\n';
	}

	// Test that H2 isotope was created correctly
	lg.clear();
	auto mat_H2 = materialMap->find("H2");
	if (mat_H2 == nullptr)
	{
		lgT.bump(5) << ERR << "Could not find 'H2'\n";
		lgT.absorb(lg);
		lgT<< STD << "Could not test compound material.";
		return;
	}
	nDbl massDensity = mat_H2->getMassDensity();
	if (!massDensity)
	{
		lgT.bump(5) << ERR << "H2 mass density was not computed\n";
		return;
	}
	else
	{
		if (!match(massDensity.value(), 1.66054E-4))
		{
			lgT.bump(5) << ERR << "H2 mass density should be 1.66054E-4, not " 
				<< massDensity.value() << '\n';
		}
	}

	// Test that H isotope was created correctly. It uses H1 ans H2.
	auto mat_H = materialMap->find("H");
	if (mat_H == nullptr)
	{
		lgT.bump(5) << ERR << "Could not find 'H'\n";
		lgT.absorb(lg);
		lgT << STD << "Could not test compound material.";
		return;
	}
	if (mat_H->isMaterialSelfConsistent())
	{
		lgT.bump(1) << ERR << "As given, Hydrogen is self consistent\n";
		lg << STD << "Photon composition had nullopt filled in.\n";
		lgT << *mat_H << '\n';
	}

	auto& neutronComposition = mat_H->getNeutronComposition();
	auto& components = neutronComposition.getComponents();
	if (components.size() != 2)
	{
		lgT.bump(1) << ERR << "Should have 2 neutron components. Have ";
		lgT << components.size() << '\n';
	}
	else
	{
		auto& cmpnt = components[0];
		if (cmpnt.getName() != "1001" || !match(cmpnt.getFraction(MASS), 0.66667) ||
			!match(cmpnt.getFraction(ATOM), 0.8))
		{
			lgT.bump(1) << ERR << "H neutron component[0] should be {'1001', 0.66667, 0.8}. It is {"
				<< cmpnt << "}\n";
		}
		auto& cmpnt1 = components[1];
		if (cmpnt1.getName() != "1002" || !match(cmpnt1.getFraction(MASS), 0.33334) ||
			!match(cmpnt1.getFraction(ATOM), 0.2))
		{
			lgT.bump(1) << ERR << "H neutron component[1] should be ('1001', 0.33334, 0.2). It is {"
				<< cmpnt1 << "}\n ";
		}
	}
	auto& photonComposition = mat_H->getPhotonComposition();
	auto& photonComponents = photonComposition.getComponents();
	if (photonComponents.size() != 1)
	{
		lgT.bump(1) << ERR << "Should have 1 photon components. Have ";
		lgT << photonComponents.size() << '\n';
	}
	else
	{
		auto& cmpnt = photonComponents[0];
		if (cmpnt.getName() != "1000" || !match(cmpnt.getFraction(MASS), std::nullopt) ||
			!match(cmpnt.getFraction(ATOM), 1.0))
		{
			lgT.bump(1) << ERR << "H photon component[0] should be ('1000', nullopt , 1.0). It is {"
				<< cmpnt << "}\n";
		}
	}
	handleError();
}
	

//  ***************************************************************************
void functionalTest3()
{
	lgT << MNR << "functional test 3\n";
	//  functionalTest1 already read file
	auto mat = materialMap->find("circular");	// should not work
	if (mat)
	{
		lgT.bump(1) << ERR << " Read 'circular' and did not return nullptr.\n";
	}
	mat = materialMap->find("circRef");	// should not work
	if (mat)
	{
		lgT.bump(1) << ERR << " Read 'circRef' and did not return nullptr.\n";
	}
	mat = materialMap->find("circ2");	// should not work
	if (mat)
	{
		lgT.bump(1) << ERR << " Read 'circRef' and did not return nullptr.\n";
	}
	handleError();
}