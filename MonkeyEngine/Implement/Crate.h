#ifndef _CRATE_H_
#define _CRATE_H_
#include "../Common/CoreUtil.h"
#include "../Common/CoreEngine.h"
#include "FrameResource.h"
#include "../Common/Geometry/CoreRenderItem.h"

enum class RenderLayer : int
{
	Opaque = 0,
	Count
};

class Crate :public CoreEngine {
private:

#pragma region Constant Object Constant pass Configuration
	std::vector<std::unique_ptr<FrameResource>> mFrameResourceStack;
	FrameResource* mCurrFrameResource = nullptr;
	ComPtr<ID3D12DescriptorHeap> mCbvHeap = nullptr;
	ComPtr<ID3D12DescriptorHeap> mSrvLmguiHeap = nullptr;
	UINT mCbvHeapSize;
	int mFrameResourceIndex = 0;
	int mNumberFrameResources = 3;
	UINT mConstantPassOffset = 0;
#pragma endregion

#pragma region Vertex Buffer Vertex Index Configuration
	std::unordered_map<std::string, std::unique_ptr<CoreMeshGeometry >> mGeometries;
	std::vector<std::unique_ptr<CoreRenderItem>> mRenderItemStack;
	std::vector<CoreRenderItem*> mRenderLayerItem[(int)RenderLayer::Count];
#pragma endregion

public:
	virtual void OnUpdate();
	virtual void OnRender();
	virtual void OnInitialize();
	virtual void OnInitializeUi();
	
	void PopulateCommands();
	~Crate();
	Crate()=default;
	Crate(int among);
	
private:
	//specific funcions
	void BuildGeometries();
	void BuildRenderItem();
	void BuildFrameResurces();
	void BuildLocalDescriptorHeap();
	void BuilduserInterface();
	void BuildConstantBufferView();
};

#endif

