#ifndef _CORE_SUB_MESH_GEOMETRY_H_
#define _CORE_SUB_MESH_GEOMETRY_H_
#include "..\CoreUtil.h"
#include <DirectXCollision.h>

class CoreSubMeshGeometry {
public:
	CoreSubMeshGeometry() = default;
	UINT mIndexCount=0;
	UINT mStartIndexLocation = 0;
	INT mBaseVertexLocation = 0;
	DirectX::BoundingBox mBound;
};

#endif

