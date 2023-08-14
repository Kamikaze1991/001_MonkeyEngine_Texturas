#ifndef _CORE_UPLOAD_BUFFER_H_
#define _CORE_UPLOAD_BUFFER_H_
#include "CoreUtil.h"

template<typename T>
class CoreUploadBuffer {
private:
	Microsoft::WRL::ComPtr<ID3D12Resource> mUploadBuffer;
	BYTE* mMappedData = nullptr;
	UINT mElementByteSize = 0;
	bool mIsConstantBuffer = false;
public:
	CoreUploadBuffer(ID3D12Device *device, UINT elementCount, bool isConstantBuffer);
	~CoreUploadBuffer();
	void CopyData(int elementIndex, const T& data);
	ID3D12Resource* GetUploadBuffer();
};
#endif


