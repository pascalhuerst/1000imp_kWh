#pragma once

#include <exception>

class DescriptiveException : public std::exception
{
public:
	DescriptiveException(const char* func, const char* file, int line, int errorNumber, const char* what);

	virtual const char* what() const noexcept;
	const char* file() const noexcept;
	const char* func() const noexcept;
	int line() const noexcept;
	int errorNumber() const noexcept;

private:
	const static size_t s_msgLength = 256;
	const static size_t s_funcNameLength = 128;
	const static size_t s_fileNameLength = 128;

	char m_msg[s_msgLength];
	char m_funcName[s_funcNameLength];
	char m_fileName[s_fileNameLength];
	int m_line;
	int m_errno;
};

std::string where(const DescriptiveException& de);
