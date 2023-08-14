#include "Crate.h"

void Crate::OnUpdate()
{
	mCurrFrameResourceIndex = (mCurrFrameResourceIndex + 1) % 3;
	mCurrFrameResource = mFrameResources[mCurrFrameResourceIndex].get();
	

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

	ID3D12DescriptorHeap* descriptorHeaps[] = { mSrvDescriptorHeap.Get() };
	GetEngineGraphicsCommandList()->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);

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
	BuildFrameResurces();
	BuildLocalDescriptorHeap();
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

void Crate::BuildFrameResurces()
{
	for (int i = 0; i < 3; ++i)
	{
		mFrameResources.push_back(std::make_unique<FrameResource>(mCoreGraphics->mDevice.Get()));
	}
}

void Crate::BuildLocalDescriptorHeap()
{
	D3D12_DESCRIPTOR_HEAP_DESC mSrvDesc = {};
	mSrvDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	mSrvDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	mSrvDesc.NumDescriptors = 1;
	ExceptionFuse(mCoreGraphics->mDevice->CreateDescriptorHeap(&mSrvDesc, IID_PPV_ARGS(mSrvDescriptorHeap.GetAddressOf())));
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
		DXGI_FORMAT_R8G8B8A8_UNORM, mSrvDescriptorHeap.Get(),
		mSrvDescriptorHeap->GetCPUDescriptorHandleForHeapStart(),
		mSrvDescriptorHeap->GetGPUDescriptorHandleForHeapStart());
}
