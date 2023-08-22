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
	CoreUploadBuffer()=default;
	CoreUploadBuffer(ID3D12Device *device, UINT elementCount, bool isConstantBuffer){
		mIsConstantBuffer = isConstantBuffer;
		mElementByteSize = sizeof(T);
		if (mIsConstantBuffer)
			mElementByteSize = CoreUtil::CalcConstantBufferByteSize(sizeof(T));
		D3D12_HEAP_PROPERTIES heapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
		D3D12_RESOURCE_DESC resourceDesc = CD3DX12_RESOURCE_DESC::Buffer(mElementByteSize*elementCount);
		ExceptionFuse(device->CreateCommittedResource(
			&heapProperties,
			D3D12_HEAP_FLAG_NONE,
			&resourceDesc,
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(mUploadBuffer.GetAddressOf())));
		ExceptionFuse(mUploadBuffer->Map(0, nullptr, reinterpret_cast<void**>(mMappedData)));
	}
	~CoreUploadBuffer() {
		if (mUploadBuffer != nullptr)
			mUploadBuffer->Unmap(0, nullptr);
		mMappedData = nullptr;
	}
	CoreUploadBuffer(const CoreUploadBuffer& rhs) = delete;
	CoreUploadBuffer& operator=(const CoreUploadBuffer& rhs) = delete;
	void CopyData(int elementIndex, const T& data) {
		memcpy(&mMappedData[mElementByteSize * elementIndex], &data, sizeof(T));
	}
	ID3D12Resource* GetUploadBuffer() {
		return mUploadBuffer.Get();
	}
};
#endif


