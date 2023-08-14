#ifndef _CORE_EXCEPTION_H_
#define _CORE_EXCEPTION_H_
#include <Windows.h>
#include <string>
#include <comdef.h>

class CoreException {
private:
public:
	CoreException() = default;
	CoreException(HRESULT hr, const std::wstring& functionName, const std::wstring& fileName, int lineNumber);
	std::wstring ProcessMessage()const;
public:
	HRESULT mErrorCode = S_OK;
	std::wstring mFunctionName;
	std::wstring mFileName;
	int mLineNumber = -1;
};

#endif


