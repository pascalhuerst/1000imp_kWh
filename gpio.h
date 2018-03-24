#pragma once

#include <memory>
#include <string>
#include <fstream>
#include <list>
#include <chrono>

#include "descriptiveexception.h"

class GPIOException : public DescriptiveException {
public:
	typedef DescriptiveException basetype;

	GPIOException(const char* func, const char* file, int line, int errorNumber, const char* what);
	virtual const char* what() const noexcept;
};

class GPIO
{
public:
	GPIO(const std::string &name);
	virtual ~GPIO();

	enum Direction {
		DirectionIn = 0x01,
		DirectionOut = 0x00
	};

	virtual void setDirection(Direction d) = 0;
	virtual void setValue(bool v) = 0;

	virtual Direction getDirection() const = 0;
	virtual bool getValue() const = 0;
    virtual bool getValueBlocking() const = 0;

	std::string getName() const;

private:
	std::string m_name;
};

typedef std::shared_ptr<GPIO> SharedGPIOHandle;

class GPIOSysFs : public GPIO
{
public:
	typedef GPIO basetype;

	GPIOSysFs(const std::string &name, int gpioNumber);
	GPIOSysFs(const std::string &name, int gpioNumber, Direction d, bool value);
	virtual ~GPIOSysFs();

	virtual void setDirection(Direction d);
	virtual void setValue(bool v);

	virtual Direction getDirection() const;
	virtual bool getValue() const;
    virtual bool getValueBlocking() const;
private:
	int m_gpioNumber;
	const static std::string s_basePath;
};

SharedGPIOHandle createGPIO(const std::string &name, int gpioNumber, GPIO::Direction d, bool value);

// Helpers
SharedGPIOHandle getGPIOForName(std::list<SharedGPIOHandle> gpios, const std::string &name);



