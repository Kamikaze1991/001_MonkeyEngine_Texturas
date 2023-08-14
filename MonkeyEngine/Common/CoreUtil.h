#ifndef _CORE_UTIL_H_
#define _CORE_UTIL_H_

#include <Windows.h>
#include <iostream>
#include <string>
#include <comdef.h>
#include <d3d12.h>
#include <dxgi1_4.h>
#include <wrl.h>
#include "ThirdParty/d3dx12.h"
#include "CoreException.h"

using namespace Microsoft::WRL;

class CoreUtil {
public:
    static std::wstring AnsiToWString(const std::string& str);
    static UINT CalcConstantBufferByteSize(UINT byteSize);
    static ComPtr<ID3D12Resource> CreateDefaultBuffer(ComPtr<ID3D12GraphicsCommandList>& graphicsCommandList, ComPtr<ID3D12Device>& device, UINT64 byteSize, const void* initData, ComPtr<ID3D12Resource>& intermediateBuffer);

};


#ifndef ExceptionFuse
#define ExceptionFuse(x)                                              \
{                                                                     \
    HRESULT hr__ = (x);                                               \
    std::wstring wfn = CoreUtil::AnsiToWString(__FILE__);                       \
    if(FAILED(hr__)) { throw CoreException(hr__, L#x, wfn, __LINE__); } \
}
#endif

#endif
