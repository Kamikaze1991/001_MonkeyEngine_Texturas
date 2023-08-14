#include "CoreException.h"

CoreException::CoreException(HRESULT hr, const std::wstring& functionName, const std::wstring& fileName, int lineNumber):
    mErrorCode(hr),
    mFunctionName(functionName),
    mLineNumber(lineNumber),
    mFileName(fileName)
{
}

std::wstring CoreException::ProcessMessage() const
{
    _com_error error(mErrorCode);

    const char* custom_log_name = "MyLogName";

    // create registry keys for ACLing described on MSDN: http://msdn2.microsoft.com/en-us/library/aa363648.aspx


    const std::wstring customMessage =
        L"FUNCTION NAME     : " + mFunctionName + L"\n" +
        L"FILE NAME         : " + mFileName + L"\n" +
        L"ERROR NUMBER LINE : " + std::to_wstring(mLineNumber) + L"\n" +
        L"MESSAGE           : " + error.ErrorMessage() + L"\n";

    HANDLE event_log = RegisterEventSource(NULL, L"MONKEY_SYSTEM_LOG");
    LPCWSTR message = customMessage.c_str();
    ReportEvent(event_log, EVENTLOG_SUCCESS, 0, 0, NULL, 1, 0, &message, NULL);
    return customMessage;
}
