#include "Logger.h"

Logger::Logger()
{
}

Logger *Logger::instance()
{
	static Logger logger;
	return &logger;
}

void Logger::log(const QString &text)
{
	if (_interface)
		_interface->log(text);
}

void Logger::setInterface(LogInterface *interface)
{
	_interface = interface;
}

LogInterface::~LogInterface()
{
}
