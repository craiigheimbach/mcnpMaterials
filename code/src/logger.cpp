#include "logger.h"
#include "nullableMath.h"
#include <fstream>

Logger lg = Logger();

//  **************************************************************************
TempLogger::TempLogger()
{
	oss << "";
}

//  ***************************************************************************
TempLogger::~TempLogger()
{;}

//  ***************************************************************************
void TempLogger::setCurrentVerbosity(int verb)
{
	if (verb < MIN_VERBOSITY) verb = MIN_VERBOSITY;
	if (verb > MAX_VERBOSITY) verb = MAX_VERBOSITY;
	currentVerbosity = verb;
}

int TempLogger::toIntVerb(std::string verb)
{
	okLong vb = toLongNullable(verb);
	if (vb.first == false) return currentVerbosity;
	if (!vb.second.has_value()) return 0;
	if (vb.second.value() <= 0) return 0;
	if (vb.second.value() >= MAX_VERBOSITY) return MAX_VERBOSITY;
	return vb.second.value();
}

//  ***************************************************************************
void TempLogger::clear()
{
	oss.str("");				// Empty stream
	oss.clear();				// clear errors
	currentVerbosity = 0;
}

//  ***************************************************************************
TempLogger& TempLogger::prepend(const std::string& text)
{
	const std::string& temp = oss.str();
	oss.seekp(0);
	oss << text;
	oss << temp;
	return *this;
}

//  ***************************************************************************
void TempLogger::absorb(TempLogger& temp)				// temp is cleared
{
	oss << temp.str();
	bump(temp.getVerbosity());
	temp.clear();
}

//  **************************************************************************
//  **************************************************************************
Logger::Logger()
{
	oss << "";
	currentVerbosity = 0;
	if (fileName.empty())
	{
		setLoggerFileName(defaultLogFileName);
	}
}

//  ***************************************************************************
Logger::~Logger()
{
	writeToLog();
	ofs.close();
}

//  ***************************************************************************
void Logger::setLoggerFileName(const std::string& name)
{
	std::ofstream ofsNew;
	if (name.empty())
	{
		ofsNew.open(defaultLogFileName, std::ios::trunc);
	}
	else
	{
		if (name == fileName) return;			// Already using correct file.
		ofsNew.open(name, std::ios::trunc);
	}
	if (!ofsNew)
	{
		std::cout << "Error opening log file " << name << '\n';
	}
	else
	{
		if (ofs.is_open()) ofs.close();
		std::swap(ofs, ofsNew);
		fileName = name;
	}
}


//  ***************************************************************************
Logger& Logger::setCoutVerbosity(int verbosity)
{
	coutThreshold = MAX_VERBOSITY - std::max(0, std::min(verbosity, MAX_VERBOSITY));
	return *this;
}

//  ***************************************************************************
Logger& Logger::setCoutVerbosity(std::string verb)
{

	return setCoutVerbosity(toIntVerb(verb));
}
//  ***************************************************************************
Logger& Logger::setFileVerbosity(int verbosity)
{
	fileThreshold = MAX_VERBOSITY - std::max(0, std::min(verbosity, MAX_VERBOSITY));
	return *this;
}

//  ***************************************************************************
Logger& Logger::setFileVerbosity(const std::string& verb)
{
	return setFileVerbosity(toIntVerb(verb));
}


//  ***************************************************************************
void Logger::writeToLog()
{
	if (currentVerbosity >= fileThreshold) ofs << oss.str() << std::flush;
	if (currentVerbosity >= coutThreshold) std::cout << oss.str() << std::flush;
	clear();
}


