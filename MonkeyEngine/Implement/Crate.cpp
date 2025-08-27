#include "Crate.h"

void Crate::OnUpdate()
{
	mFrameResourceIndex = (mFrameResourceIndex + 1) % 3;
	mCurrFrameResource = mFrameResourceStack[mFrameResourceIndex].get();
	

	if (mCurrFrameResource->mFrameFenceCount != 0 && mCoreGraphics->mFence->GetCompletedValue() < mCurrFrameResource->mFrameFenceCount)
	{
		HANDLE eventHandle = CreateEventEx(nullptr, FALSE, FALSE, EVENT_ALL_ACCESS);
		ExceptionFuse(mCoreGraphics->mFence->SetEventOnCompletion(mCurrFrameResource->mFrameFenceCount, eventHandle));
		if (eventHandle) {
			WaitForSingleObject(eventHandle, INFINITE);
			CloseHandle(eventHandle);
		}
	}

	//mCoreGraphics->FlushCommandQueue(mCurrFrameResource->mFrameFenceCount);
}

void Crate::OnRender()
{
	PopulateCommands();
}

void Crate::PopulateCommands()
{
	auto mCurrCmd = mCurrFrameResource->mFrameCommandAllocator;
	mCurrCmd->Reset();
	GetEngineGraphicsCommandList()->Reset(mCurrCmd.Get(), nullptr);
	D3D12_RESOURCE_BARRIER rbInitial = CD3DX12_RESOURCE_BARRIER::Transition(mCoreGraphics->mRenderTargetBuffer[mCurrFrame].Get(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
	GetEngineGraphicsCommandList()->ResourceBarrier(1, &rbInitial);
	CD3DX12_CPU_DESCRIPTOR_HANDLE descriptor(mCoreGraphics->mRtvHeap->GetCPUDescriptorHandleForHeapStart(), mCurrFrame, mCoreGraphics->mRtvHeapSize);
	float clearColor[] = { clear_color.x/ clear_color.w,clear_color.y / clear_color.w, clear_color.z / clear_color.w, clear_color.w };
	GetEngineGraphicsCommandList()->ClearRenderTargetView(descriptor, clearColor, 0, nullptr);
	GetEngineGraphicsCommandList()->ClearDepthStencilView(mCoreGraphics->DepthStencilView(), D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);
	D3D12_CPU_DESCRIPTOR_HANDLE currBackBuffer = mCoreGraphics->CurrentBackBufferView(mCurrFrame);
	D3D12_CPU_DESCRIPTOR_HANDLE currDepthStencil = mCoreGraphics->DepthStencilView();
	GetEngineGraphicsCommandList()->OMSetRenderTargets(1, &currBackBuffer, true, &currDepthStencil);

	ID3D12DescriptorHeap* descriptorHeaps[] = { mCbvHeap.Get() };
	GetEngineGraphicsCommandList()->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);

	//Lmgui descriptor heap
	ID3D12DescriptorHeap* descriptorHeapsLmgui[] = { mSrvLmguiHeap.Get() };
	GetEngineGraphicsCommandList()->SetDescriptorHeaps(_countof(descriptorHeapsLmgui), descriptorHeapsLmgui);

	//GetEngineGraphicsCommandList()->SetDescriptorHeaps(1, &mCoreGraphics->mCbvSrvUavHeap);
	ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), GetEngineGraphicsCommandList().Get());
	rbInitial = CD3DX12_RESOURCE_BARRIER::Transition(mCoreGraphics->mRenderTargetBuffer[mCurrFrame].Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
	GetEngineGraphicsCommandList()->ResourceBarrier(1, &rbInitial);
	GetEngineGraphicsCommandList()->Close();

	ID3D12CommandList* mList[] = { GetEngineGraphicsCommandList().Get() };
	GetEngineCommandQueue()->ExecuteCommandLists(_countof(mList), mList);
	GetEngineSwapChain()->Present(1, 0);
	mCurrFrameResource->mFrameFenceCount = ++mCoreGraphics->mFenceCount;

	GetEngineCommandQueue()->Signal(mCoreGraphics->mFence.Get(), mCoreGraphics->mFenceCount);

	//mCoreGraphics->FlushCommandQueue(mCurrFrameResource->mFrameFenceCount);
}

Crate::~Crate()
{
		
}

Crate::Crate(int among):CoreEngine()
{
	
}

void Crate::OnInitialize()
{
	LoadTextures();
	BuildRootSignature();
	BuildGeometries();
	BuildRenderItem();
	BuildFrameResurces();
	BuildLocalDescriptorHeap();
	BuildConstantBufferView();
	BuilduserInterface();
	mCoreGraphics->FlushCommandQueue();
}

void Crate::OnInitializeUi()
{
	ImGui_ImplDX12_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();
	if (show_demo_window)
		ImGui::ShowDemoWindow(&show_demo_window);

	{
		static float f = 0.0f;
		static int counter = 0;

		ImGui::Begin("Hello, world!x");                          // Create a window called "Hello, world!" and append into it.

		ImGui::Text("This is some useful text.");               // Display some text (you can use a format strings too)
		ImGui::Checkbox("Demo Window", &show_demo_window);      // Edit bools storing our window open/close state
		ImGui::Checkbox("Another Window", &show_another_window);

		ImGui::SliderFloat("float", &f, 0.0f, 1.0f);            // Edit 1 float using a slider from 0.0f to 1.0f
		ImGui::ColorEdit3("clear color", (float*)&clear_color); // Edit 3 floats representing a color

		if (ImGui::Button("Button"))                            // Buttons return true when clicked (most widgets return true when edited/activated)
			counter++;
		ImGui::SameLine();
		ImGui::Text("counter = %d", counter);

		ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io->Framerate, io->Framerate);
		ImGui::End();
	}

	if (show_another_window)
	{
		ImGui::Begin("Another Window", &show_another_window);   // Pass a pointer to our bool variable (the window will have a closing button that will clear the bool when clicked)
		ImGui::Text("Hello from another window!");
		if (ImGui::Button("Close Me"))
			show_another_window = false;
		ImGui::End();
	}
	ImGui::Render();
	//ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), g_pd3dCommandList);

}

void Crate::BuildGeometries()
{
#pragma region Generar las geometrias con la herramienta de terceros
	GeometryGenerator geometryGenerator;
	MeshData box = geometryGenerator.CreateBox(1.0f, 1.0f, 1.0f, 0);
	MeshData plane = geometryGenerator.CreateGrid(50.0f, 50.0f, 3, 3);
#pragma endregion

#pragma region rellenar los bufferes primitivos
	UINT totalVertexSize = (UINT)box.Vertices.size() + (UINT)plane.Vertices.size();
	std::vector<Vertex> vertexBuffer(totalVertexSize);
	std::vector<uint16_t> indexBuffer;
	int k = 0;
	for (int i = 0; i < box.Vertices.size(); i++, k++) {
		vertexBuffer[k].Position = box.Vertices[i].Position;
		vertexBuffer[k].Color = XMFLOAT4(Colors::Red);
	}
	for (int i = 0; i < plane.Vertices.size(); i++, k++) {
		vertexBuffer[k].Position = plane.Vertices[i].Position;
		vertexBuffer[k].Color = XMFLOAT4(Colors::Red);
	}
	auto l0 = box.GetIndices16();
	auto indexBox = box.GetIndices16();
	auto indexPlane = plane.GetIndices16();

	indexBuffer.insert(indexBuffer.end(), std::begin(box.GetIndices16()), std::end(box.GetIndices16()));
	indexBuffer.insert(indexBuffer.end(), std::begin(plane.GetIndices16()), std::end(plane.GetIndices16()));
#pragma endregion

#pragma region Calculo de offset y sizes totales
	UINT boxVertexOffset = 0;
	UINT planeVertexOffset = (UINT)box.Vertices.size();

	UINT boxIndexOffset = 0;
	UINT planeIndexOffset = (UINT)box.Indices32.size();

	UINT vertexBufferByteSize = sizeof(Vertex) * (UINT)vertexBuffer.size();
	UINT indexBufferByteSize = sizeof(int16_t) * (UINT)indexBuffer.size();
#pragma endregion

	
#pragma region Creacion de los buffer CPU_GPU
	std::unique_ptr<CoreMeshGeometry> meshGeometry = std::make_unique<CoreMeshGeometry>();
	meshGeometry->mName = "WeirdThing";
	D3DCreateBlob(vertexBufferByteSize, &meshGeometry->mCpuVertexBuffer);
	D3DCreateBlob(indexBufferByteSize, &meshGeometry->mCpuIndexBuffer);
	CopyMemory(meshGeometry->mCpuVertexBuffer->GetBufferPointer(), vertexBuffer.data(), vertexBufferByteSize);
	CopyMemory(meshGeometry->mCpuIndexBuffer->GetBufferPointer(), indexBuffer.data(), indexBufferByteSize);
	meshGeometry->mGpuVertexBuffer = CoreUtil::CreateDefaultBuffer(mCoreGraphics->mGraphicsCommandList.Get(), mCoreGraphics->mDevice.Get(), vertexBufferByteSize, vertexBuffer.data(), meshGeometry->mGpuVertexBufferIntermediate);
	meshGeometry->mGpuIndexBuffer = CoreUtil::CreateDefaultBuffer(mCoreGraphics->mGraphicsCommandList.Get(), mCoreGraphics->mDevice.Get(), indexBufferByteSize, indexBuffer.data(), meshGeometry->mGpuIndexBufferIntermediate);
	meshGeometry->mVertexByteStride = sizeof(Vertex);
	meshGeometry->mVertexBufferByteSize = vertexBufferByteSize;
	meshGeometry->mIndexBufferByteSize = indexBufferByteSize;
	meshGeometry->mIndexBufferFormat = DXGI_FORMAT_R16_UINT;
#pragma endregion

#pragma region Creacion de los submesh
	CoreSubMeshGeometry submeshBox;
	submeshBox.mIndexCount = (INT)box.Indices32.size();
	submeshBox.mStartIndexLocation = boxIndexOffset;
	submeshBox.mBaseVertexLocation = boxVertexOffset;

	CoreSubMeshGeometry submeshPlane;
	submeshPlane.mIndexCount = (INT)plane.Indices32.size();
	submeshPlane.mStartIndexLocation = planeIndexOffset;
	submeshPlane.mBaseVertexLocation = planeVertexOffset;

	meshGeometry->mDrawArgs["box"] = submeshBox;
	meshGeometry->mDrawArgs["plane"] = submeshPlane;

	mGeometries[meshGeometry->mName] = std::move(meshGeometry);
#pragma endregion
}

void Crate::BuildRenderItem()
{
	std::unique_ptr<CoreRenderItem> boxRender = std::make_unique<CoreRenderItem>(mNumberFrameResources);
	XMStoreFloat4x4(&boxRender->mWorld, XMMatrixTranslation(0.0f, 0.0f, 0.0f));
	boxRender->mConstantBufferIndex = 0;
	boxRender->mMeshGeometry = mGeometries["WeirdThing"].get();
	boxRender->mIndexCount = boxRender->mMeshGeometry->mDrawArgs["box"].mIndexCount;
	boxRender->mStartIndexLocation = boxRender->mMeshGeometry->mDrawArgs["box"].mStartIndexLocation;
	boxRender->mBaseVertexLocation = boxRender->mMeshGeometry->mDrawArgs["box"].mBaseVertexLocation;
	mRenderLayerItem[(int)RenderLayer::Opaque].push_back(boxRender.get());

	std::unique_ptr<CoreRenderItem> planeRender = std::make_unique<CoreRenderItem>(mNumberFrameResources);
	XMStoreFloat4x4(&planeRender->mWorld, XMMatrixTranslation(0.0f, 0.0f, 0.0f));
	planeRender->mConstantBufferIndex = 1;
	planeRender->mMeshGeometry = mGeometries["WeirdThing"].get();
	planeRender->mIndexCount = planeRender->mMeshGeometry->mDrawArgs["plane"].mIndexCount;
	planeRender->mStartIndexLocation = planeRender->mMeshGeometry->mDrawArgs["plane"].mStartIndexLocation;
	planeRender->mBaseVertexLocation = planeRender->mMeshGeometry->mDrawArgs["plane"].mBaseVertexLocation;
	mRenderLayerItem[(int)RenderLayer::Opaque].push_back(planeRender.get());

	mRenderItemStack.push_back(std::move(boxRender));
	mRenderItemStack.push_back(std::move(planeRender));
}

void Crate::BuildFrameResurces()
{
	mFrameResourceStack.clear();
	for (int i = 0; i < mNumberFrameResources; ++i)
		mFrameResourceStack.push_back(std::make_unique<FrameResource>(mCoreGraphics->mDevice.Get(),(int)mRenderItemStack.size(), 1));
}

void Crate::LoadTextures()
{
	auto woodCrateTex = std::make_unique<Texture>();
	woodCrateTex->Name = "woodCrateTex";
	woodCrateTex->Filename = L"Textures/robo.dds";
	ExceptionFuse(DirectX::CreateDDSTextureFromFile12(mCoreGraphics->mDevice.Get(),
		GetEngineGraphicsCommandList().Get(), woodCrateTex->Filename.c_str(),
		woodCrateTex->Resource, woodCrateTex->UploadHeap));

	mTextures[woodCrateTex->Name] = std::move(woodCrateTex);
}

void Crate::BuildRootSignature()
{
	CD3DX12_DESCRIPTOR_RANGE texTable;
	texTable.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);

	// Root parameter can be a table, root descriptor or root constants.
	CD3DX12_ROOT_PARAMETER slotRootParameter[4];

	// Perfomance TIP: Order from most frequent to least frequent.
	slotRootParameter[0].InitAsDescriptorTable(1, &texTable, D3D12_SHADER_VISIBILITY_PIXEL);
	slotRootParameter[1].InitAsConstantBufferView(0);
	slotRootParameter[2].InitAsConstantBufferView(1);
	slotRootParameter[3].InitAsConstantBufferView(2);

	auto staticSamplers = GetStaticSamplers();

	// A root signature is an array of root parameters.
	CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc(4, slotRootParameter,
		(UINT)staticSamplers.size(), staticSamplers.data(),
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

	// create a root signature with a single slot which points to a descriptor range consisting of a single constant buffer


	ComPtr<ID3DBlob> serializedRootSig = nullptr;
	ComPtr<ID3DBlob> errorBlob = nullptr;
	HRESULT hr = D3D12SerializeRootSignature(&rootSigDesc, D3D_ROOT_SIGNATURE_VERSION_1,
		serializedRootSig.GetAddressOf(), errorBlob.GetAddressOf());

	if (errorBlob != nullptr)
	{
		::OutputDebugStringA((char*)errorBlob->GetBufferPointer());
	}
	ExceptionFuse(hr);

	ExceptionFuse(mCoreGraphics->mDevice->CreateRootSignature(
		0,
		serializedRootSig->GetBufferPointer(),
		serializedRootSig->GetBufferSize(),
		IID_PPV_ARGS(mRootSignature.GetAddressOf())));
}

void Crate::BuildLocalDescriptorHeap()
{
	mCbvHeapSize = mCoreGraphics->mDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	D3D12_DESCRIPTOR_HEAP_DESC mSrvDesc = {};
	mSrvDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	mSrvDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	mSrvDesc.NumDescriptors = ((UINT)mRenderItemStack.size()+1)*mNumberFrameResources;
	ExceptionFuse(mCoreGraphics->mDevice->CreateDescriptorHeap(&mSrvDesc, IID_PPV_ARGS(mCbvHeap.GetAddressOf())));

	D3D12_DESCRIPTOR_HEAP_DESC mSrvLmguiDesc = {};
	mSrvLmguiDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	mSrvLmguiDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	mSrvLmguiDesc.NumDescriptors = 1;
	ExceptionFuse(mCoreGraphics->mDevice->CreateDescriptorHeap(&mSrvLmguiDesc, IID_PPV_ARGS(mSrvLmguiHeap.GetAddressOf())));
}

void Crate::BuilduserInterface()
{
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	io = &ImGui::GetIO(); (void)io;
	io->ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
	io->ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

	ImGui::StyleColorsDark();

	ImGui_ImplWin32_Init(gMainHwnd);
	ImGui_ImplDX12_Init(mCoreGraphics->mDevice.Get(), 3,
		DXGI_FORMAT_R8G8B8A8_UNORM, mSrvLmguiHeap.Get(),
		mSrvLmguiHeap->GetCPUDescriptorHandleForHeapStart(),
		mSrvLmguiHeap->GetGPUDescriptorHandleForHeapStart());
}


void Crate::BuildConstantBufferView()
{
	UINT ConstantObjectByteSize = CoreUtil::CalcConstantBufferByteSize(sizeof(ObjectConstant));
	for (UINT ri = 0; ri < (UINT)mNumberFrameResources; ri++) {
		ComPtr<ID3D12Resource> currUploadBuffer = mFrameResourceStack[ri]->ConstantBufferObject->GetUploadBuffer();
		for (UINT i = 0; i < mRenderItemStack.size(); i++) {
			D3D12_GPU_VIRTUAL_ADDRESS gpuVirtualAddress = currUploadBuffer->GetGPUVirtualAddress();
			gpuVirtualAddress += (i * ConstantObjectByteSize);
			UINT heapIndex = ri * (UINT)mRenderItemStack.size() + i;
			CD3DX12_CPU_DESCRIPTOR_HANDLE handle(mCbvHeap->GetCPUDescriptorHandleForHeapStart());
			handle.Offset(heapIndex, mCbvHeapSize);
			D3D12_CONSTANT_BUFFER_VIEW_DESC cbvd = {};
			cbvd.BufferLocation = gpuVirtualAddress;
			cbvd.SizeInBytes = ConstantObjectByteSize;
			mCoreGraphics->mDevice->CreateConstantBufferView(&cbvd, handle);
		}
	}

	UINT ConstantPassByteSize = CoreUtil::CalcConstantBufferByteSize(sizeof(PassConstant));
	for (UINT i = 0; i < (UINT)mNumberFrameResources; i++) {
		D3D12_GPU_VIRTUAL_ADDRESS passGpuVirtualAddress = mFrameResourceStack[i]->ConstantBufferPass->GetUploadBuffer()->GetGPUVirtualAddress();
		CD3DX12_CPU_DESCRIPTOR_HANDLE passHandle(mCbvHeap->GetCPUDescriptorHandleForHeapStart());
		passHandle.Offset(mConstantPassOffset+i, mCbvHeapSize);
		D3D12_CONSTANT_BUFFER_VIEW_DESC cbvd2 = {};
		cbvd2.BufferLocation = passGpuVirtualAddress;
		cbvd2.SizeInBytes = ConstantPassByteSize;
		mCoreGraphics->mDevice->CreateConstantBufferView(&cbvd2, passHandle);
	}
	
}

std::array<const CD3DX12_STATIC_SAMPLER_DESC, 6> Crate::GetStaticSamplers()
{
	// Applications usually only need a handful of samplers.  So just define them all up front
	// and keep them available as part of the root signature.  

	const CD3DX12_STATIC_SAMPLER_DESC pointWrap(
		0, // shaderRegister
		D3D12_FILTER_MIN_MAG_MIP_POINT, // filter
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressU
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressV
		D3D12_TEXTURE_ADDRESS_MODE_WRAP); // addressW

	const CD3DX12_STATIC_SAMPLER_DESC pointClamp(
		1, // shaderRegister
		D3D12_FILTER_MIN_MAG_MIP_POINT, // filter
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressU
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressV
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP); // addressW

	const CD3DX12_STATIC_SAMPLER_DESC linearWrap(
		2, // shaderRegister
		D3D12_FILTER_MIN_MAG_MIP_LINEAR, // filter
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressU
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressV
		D3D12_TEXTURE_ADDRESS_MODE_WRAP); // addressW

	const CD3DX12_STATIC_SAMPLER_DESC linearClamp(
		3, // shaderRegister
		D3D12_FILTER_MIN_MAG_MIP_LINEAR, // filter
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressU
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressV
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP); // addressW

	const CD3DX12_STATIC_SAMPLER_DESC anisotropicWrap(
		4, // shaderRegister
		D3D12_FILTER_ANISOTROPIC, // filter
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressU
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressV
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressW
		0.0f,                             // mipLODBias
		8);                               // maxAnisotropy

	const CD3DX12_STATIC_SAMPLER_DESC anisotropicClamp(
		5, // shaderRegister
		D3D12_FILTER_ANISOTROPIC, // filter
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressU
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressV
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressW
		0.0f,                              // mipLODBias
		8);                                // maxAnisotropy

	return {
		pointWrap, pointClamp,
		linearWrap, linearClamp,
		anisotropicWrap, anisotropicClamp };
}


