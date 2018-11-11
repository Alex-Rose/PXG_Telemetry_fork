#ifndef LOGGER_H
#define LOGGER_H

#include <QString>



class LogInterface
{
public:
	virtual ~LogInterface();
	virtual void log(const QString& text) = 0;
};

class Logger
{
public:
	static Logger* instance();

	void log(const QString& text);

	void setInterface(LogInterface* interface);

private:
	Logger();
	LogInterface* _interface;
};

#endif // LOGGER_H
