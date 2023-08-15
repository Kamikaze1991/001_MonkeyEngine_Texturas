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
	std::vector<std::unique_ptr<FrameResource>> mFrameResources;
	FrameResource* mCurrFrameResource = nullptr;
	ComPtr<ID3D12DescriptorHeap> mSrvDescriptorHeap = nullptr;
	ComPtr<ID3D12DescriptorHeap> mSrvLmguiDescriptorHeap = nullptr;
	int mCurrFrameResourceIndex = 0;

	//render items
	std::unordered_map<std::string, std::unique_ptr<CoreMeshGeometry >> mGeometries;
	std::vector<std::unique_ptr<CoreRenderItem>> mRenderItemStack;
	std::vector<CoreRenderItem*> mRenderLayerItem[(int)RenderLayer::Count];


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
	void BuildFrameResurces();
	void BuildLocalDescriptorHeap();
	void BuilduserInterface();
};

#endif

