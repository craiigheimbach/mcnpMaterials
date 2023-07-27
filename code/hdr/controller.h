#pragma once
#include <iostream> 
#include <fstream>

#include "materials.h"

//  ***************************************************************************
class Controller
{
public:
	Controller();
	bool setControlFileName(const std::string name);
	void execute();

private:
	std::ifstream ifs;
	MaterialFileReader materials;
	int lineNumber;
	std::string originalLine;

	void setErrorLimit(const std::string& limit);
	void showMaterial(const std::string& target);
	void checkConsistencyMaterial(const std::string& target);
	void showConsistency(std::shared_ptr<Material> mat, bool ok);
	void listNames(const std::string& target);
	void compare(const std::string& target);
	void showMcnpFormat(const std::string& target);
	void normalize(const std::string& target);
};

