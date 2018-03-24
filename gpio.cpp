#include "gpio.h"

// For blocking read
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include <fstream>
#include <thread>

#include <string.h>

GPIOException::GPIOException(const char* func, const char* file, int line, int errorNumber, const char *what) :
    basetype(func, file, line, errorNumber, what)
{}

const char* GPIOException::what() const noexcept
{
    return basetype::what();
}

GPIO::GPIO(const std::string &name) :
    m_name(name)
{
}

GPIO::~GPIO()
{}

std::string GPIO::getName() const
{
    return m_name;
}

// SYS FS
const std::string GPIOSysFs::s_basePath = "/sys/class/gpio";


GPIOSysFs::GPIOSysFs(const std::string &name, int gpioNumber) :
    basetype(name),
    m_gpioNumber(gpioNumber)
{
    std::ofstream fileExport;
    std::string fileExportPath = s_basePath + "/export";
    fileExport.open(fileExportPath);
    if (!fileExport.is_open())
        throw GPIOException(__PRETTY_FUNCTION__, __FILE__, __LINE__, 0,
                            ("Can not open file: " + fileExportPath + " to export GPIO" + std::to_string(m_gpioNumber)).c_str());

    fileExport << m_gpioNumber << std::endl;
    fileExport.flush();
}

GPIOSysFs::GPIOSysFs(const std::string &name, int gpioNumber, Direction d, bool value) :
    GPIOSysFs(name, gpioNumber)
{
    setDirection(d);
    setValue(value);
}

GPIOSysFs::~GPIOSysFs()
{
    std::ofstream fileUnexport;
    std::string fileUnexportPath = s_basePath + "/unexport";
    fileUnexport.open(fileUnexportPath);
    fileUnexport << m_gpioNumber << std::endl;
    fileUnexport.flush();
}

void GPIOSysFs::setDirection(Direction d)
{
    std::ofstream fileDirection;
    std::string fileDirectionPath = s_basePath + "/gpio" + std::to_string(m_gpioNumber) + "/direction";
    fileDirection.open(fileDirectionPath);
    if (!fileDirection.is_open())
        throw GPIOException(__PRETTY_FUNCTION__, __FILE__, __LINE__, 0,
                            ("Can not open file: " + fileDirectionPath + " to set direction for GPIO" + std::to_string(m_gpioNumber)).c_str());

    fileDirection << (d == DirectionIn ? "in" : "out") << std::endl;
    fileDirection.flush();
}

void GPIOSysFs::setValue(bool v)
{
    std::ofstream fileValue;
    std::string fileValuePath = s_basePath + "/gpio" + std::to_string(m_gpioNumber) + "/value";
    fileValue.open(fileValuePath);
    if (!fileValue.is_open())
        throw GPIOException(__PRETTY_FUNCTION__, __FILE__, __LINE__, 0,
                            ("Can not open file: " + fileValuePath + " to set value for GPIO" + std::to_string(m_gpioNumber)).c_str());

    fileValue << (v ? "1" : "0") << std::endl;
    fileValue.flush();
}

GPIO::Direction GPIOSysFs::getDirection() const
{
    std::ifstream fileDirection;
    std::string fileDirectionPath = s_basePath + "/gpio" + std::to_string(m_gpioNumber) + "/direction";
    fileDirection.open(fileDirectionPath);
    if (!fileDirection.is_open())
        throw GPIOException(__PRETTY_FUNCTION__, __FILE__, __LINE__, 0,
                            ("Can not open file: " + fileDirectionPath + " to get direction for GPIO" + std::to_string(m_gpioNumber)).c_str());

    std::string line;
    std::getline(fileDirection, line);

    if (line != "in" && line != "out")
        throw GPIOException(__PRETTY_FUNCTION__, __FILE__, __LINE__, 0,
                            ("Invalid value (" + line + ") while reading direction for GPIO" + std::to_string(m_gpioNumber)).c_str());

    return (line == "in" ? DirectionIn : DirectionOut);
}

bool GPIOSysFs::getValue() const
{
    std::ifstream fileValue;
    std::string fileValuePath = s_basePath + "/gpio" + std::to_string(m_gpioNumber) + "/value";
    fileValue.open(fileValuePath);
    if (!fileValue.is_open())
        throw GPIOException(__PRETTY_FUNCTION__, __FILE__, __LINE__, 0,
                            ("Can not open file: " + fileValuePath + " to get value for GPIO" + std::to_string(m_gpioNumber)).c_str());

    std::string line;
    std::getline(fileValue, line);

    if (line != "1" && line != "0")
        throw GPIOException(__PRETTY_FUNCTION__, __FILE__, __LINE__, 0,
                            ("Invalid value (" + line + ") while reading dvaliue for GPIO" + std::to_string(m_gpioNumber)).c_str());

    return (line == "1" ? true : false);
}

bool GPIOSysFs::getValueBlocking() const
{
    std::string fileValuePath = s_basePath + "/gpio" + std::to_string(m_gpioNumber) + "/value";
    int fd = open(fileValuePath.c_str(), O_RDONLY);
    if (fd < 0)
        throw GPIOException(__PRETTY_FUNCTION__, __FILE__, __LINE__, 0,
                            ("Can not open file: " + fileValuePath + " to get value for GPIO" + std::to_string(m_gpioNumber)).c_str());

    uint8_t buffer[2];
    int ret = read(fd, buffer, 2);

    if (ret < 0 || (buffer[0] != '1' && buffer[0] != '0'))
        throw GPIOException(__PRETTY_FUNCTION__, __FILE__, __LINE__, 0,
                            ("Invalid value (" + std::to_string(buffer[0]) + ") while reading dvaliue for GPIO" + std::to_string(m_gpioNumber)).c_str());

    return (buffer[0] == '1' ? true : false);

}


SharedGPIOHandle createGPIO(const std::string &name, int gpioNumber, GPIO::Direction d, bool value)
{
    return std::unique_ptr<GPIO>(new GPIOSysFs(name, gpioNumber, d, value));
}

// Helpers
SharedGPIOHandle getGPIOForName(std::list<SharedGPIOHandle> gpios, const std::string &name)
{
    for (auto it=gpios.begin(); it!=gpios.end(); ++it) {
        if ((*it)->getName() == name)
            return *it;
    }

    throw GPIOException(__PRETTY_FUNCTION__, __FILE__, __LINE__, 0,
                        ("GPIO with name \"" + name +  "\" does not exist!").c_str());
}
