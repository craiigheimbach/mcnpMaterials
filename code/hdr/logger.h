#pragma once
#include <iostream> 
#include <sstream>
#include <fstream>
#include "nullableMath.h"

const std::string STD  = "   ";
const std::string ERR   = "  -";
const std::string SEC   = "** ";
const std::string MNR   = " * ";

static const int MAX_VERBOSITY = 5;
static const int MIN_VERBOSITY = 0;


//  ***************************************************************************
//  Usd to store logging information to be used later.
//  This helps to not show unimportant logging information
//
class TempLogger 
{
public:
	TempLogger();
	virtual ~TempLogger();

	virtual inline TempLogger& bump(int verb)		// can only increase verbosity
	{
		if (verb > currentVerbosity) setCurrentVerbosity(verb);
		return *this;
	}
	virtual inline int getVerbosity() const { return currentVerbosity; };
	virtual void clear();						// clear buffer. zero verbosity.
	virtual TempLogger& prepend(const std::string& text);
	void absorb(TempLogger& temp);				// temp is cleared
	const std::string str() const { return oss.str(); };

protected:
	std::ostringstream oss;
	int currentVerbosity = 0;

	int toIntVerb(std::string verb);
	void setCurrentVerbosity(int verb);

	template <class T>
	friend TempLogger& operator<< (TempLogger& tempLogger, const T& rhs);
};

//  ***************************************************************************
//  Logs rhs string (or anything that implements "<<")
//
template <class T>
TempLogger& operator<< (TempLogger& tempLogger, const T& rhs)
{
	tempLogger.oss << rhs;
	return tempLogger;
}


//  ***************************************************************************
class Logger : public TempLogger
{
public:
	Logger();
	virtual ~Logger();

	void setLoggerFileName(const std::string& name);		//  Deletes old file
	Logger& setCoutVerbosity(int verbosity);
	Logger& setCoutVerbosity(std::string verbosity);		// sets threshold for output
	int getCoutVerbosity() const { return coutThreshold; };
	Logger& setFileVerbosity(int verbosity);
	Logger& setFileVerbosity(const std::string& verbosity);
	int getFileVerbosity() const { return fileThreshold; };
	void writeToLog();						// send to output, clear buffer, and zero verbosity
	void inline forceWriteToLog()
	{
		setCurrentVerbosity(MAX_VERBOSITY);
		writeToLog();
	}

private:
	std::ofstream ofs;
	std::string fileName = "";
	inline static const std::string defaultLogFileName = "mcnpMaterials.log";
	// When writing to cout or file, will output if currentVerbosity >= threshold
	static const int defaultThreshold = 3;
	int coutThreshold = defaultThreshold;
	int fileThreshold = defaultThreshold;
};





