#ifndef _CORE_RENDER_ITEM_H_
#define _CORE_RENDER_ITEM_H_
#include "..\CoreUtil.h"
#include "CoreMeshGeometry.h"
class CoreRenderItem {
public:
	XMFLOAT4X4 mWorld=CoreUtil::Identity4x4();
	INT mDirtyFrames = 3;
	UINT mConstantBufferIndex = -1;
	CoreMeshGeometry* mMeshGeometry=nullptr;
	D3D12_PRIMITIVE_TOPOLOGY mTopology = D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	UINT mStartIndexLocation=0;
	INT mBaseVertexLocation = 0;
	INT mIndexCount = 0;

	CoreRenderItem(INT dirtyFrames);
};

#endif
