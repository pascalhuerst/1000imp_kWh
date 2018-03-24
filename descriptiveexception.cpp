#include <string.h>
#include <sstream>

#include "descriptiveexception.h"

DescriptiveException::DescriptiveException(const char* func, const char* file, int line, int errorNumber, const char* what) :
	m_line(line),
	m_errno(errorNumber)
{
	size_t l = s_msgLength < strlen(what) ? s_msgLength : strlen(what);
	memcpy(m_msg, what, l);
	m_msg[l] = 0;

	l = s_funcNameLength < strlen(func) ? s_funcNameLength : strlen(func);
	memcpy(m_funcName, func, l);
	m_funcName[l] = 0;

	l = s_fileNameLength < strlen(file) ? s_fileNameLength : strlen(file);
	memcpy(m_fileName, file, l);
	m_fileName[l] = 0;
}

const char* DescriptiveException::what() const noexcept
{
	return m_msg;
}

const char* DescriptiveException::file() const noexcept
{
	return m_fileName;
}

const char* DescriptiveException::func() const noexcept
{
	return m_funcName;
}

int DescriptiveException::line() const noexcept
{
	return m_line;
}

int DescriptiveException::errorNumber() const noexcept
{
	return m_errno;
}

std::string where(const DescriptiveException& de)
{
	std::stringstream ret;
	ret
			<< "File     :" << de.file() << std::endl
			<< "Function :" << de.func() << std::endl
			<< "Line     :" << de.line() << std::endl;

	return ret.str();
}
