//***************************************************************************************
// BoxApp.cpp by Frank Luna (C) 2015 All Rights Reserved.
//
// Shows how to draw a box in Direct3D 12.
//
// Controls:
//   Hold the left mouse button down and move the mouse to rotate.
//   Hold the right mouse button down and move the mouse to zoom in and out.
//***************************************************************************************

#include "../../Common/d3dApp.h"
#include "../../Common/MathHelper.h"
#include "../../Common/UploadBuffer.h"

using Microsoft::WRL::ComPtr;
using namespace DirectX;
using namespace DirectX::PackedVector;

struct Vertex
{
    XMFLOAT3 Pos;
    XMFLOAT4 Color;
};

struct VertexAlt
{
	XMFLOAT3 Pos;
	XMCOLOR Color;
};

//6.13 - Question 2 - BEGIN
struct VertexPOS
{
	XMFLOAT3 Pos;
};

struct VertexCOL
{
	XMFLOAT4 Color;
};
//6.13 - Question 2 - END

struct ObjectConstants
{
    XMFLOAT4X4 WorldViewProj = MathHelper::Identity4x4();
	//6.13 - Question 6 - BEGIN
	float gTime = 0.0f;
	//6.13 - Question 6 - END

};

class BoxApp : public D3DApp
{
public:
	BoxApp(HINSTANCE hInstance);
    BoxApp(const BoxApp& rhs) = delete;
    BoxApp& operator=(const BoxApp& rhs) = delete;
	~BoxApp();

	virtual bool Initialize()override;

private:
    virtual void OnResize()override;
    virtual void Update(const GameTimer& gt)override;
    virtual void Draw(const GameTimer& gt)override;

	void DrawCube();
	void DrawPyramid();
	void DrawCombined();

	//6.13 - Question 10 - BEGIN
	void DrawCubeAlt();
	//6.13 - Question 10 - END

    virtual void OnMouseDown(WPARAM btnState, int x, int y)override;
    virtual void OnMouseUp(WPARAM btnState, int x, int y)override;
    virtual void OnMouseMove(WPARAM btnState, int x, int y)override;

    void BuildDescriptorHeaps();
	void BuildConstantBuffers();
    void BuildRootSignature();
    void BuildShadersAndInputLayout();
    void BuildBoxGeometry();
	//6.13 - Question 4 - BEGIN
	void BuildPyramidGeometry();
	//6.13 - Question 4 - END
	//6.13 - Question 7 - BEGIN
	void BuildCombinedGeometry();
	//6.13 - Question 7 - END

	//6.13 - Question 10 - BEGIN
	void BuildBoxGeometryAlt();
	//6.13 - Question 10 - END

    void BuildPSO();

private:
    
    ComPtr<ID3D12RootSignature> mRootSignature = nullptr;
    ComPtr<ID3D12DescriptorHeap> mCbvHeap = nullptr;

    std::unique_ptr<UploadBuffer<ObjectConstants>> mObjectCB = nullptr;

	std::unique_ptr<MeshGeometry> mBoxGeo = nullptr;
	std::unique_ptr<MeshGeometry> mPyramidGeo = nullptr;
	std::unique_ptr<MeshGeometry> mComboGeo = nullptr;

    ComPtr<ID3DBlob> mvsByteCode = nullptr;
    ComPtr<ID3DBlob> mpsByteCode = nullptr;

    std::vector<D3D12_INPUT_ELEMENT_DESC> mInputLayout;

	//6.13 - Question 2 - BEGIN
	std::vector<D3D12_INPUT_ELEMENT_DESC> mInputLayoutPos;
	std::vector<D3D12_INPUT_ELEMENT_DESC> mInputLayoutCol;
	//6.13 - Question 2 - END

	//6.13 - Question 10 - BEGIN
	std::vector<D3D12_INPUT_ELEMENT_DESC> mInputLayoutAlt;
	//6.13 - Question 10 - END


    ComPtr<ID3D12PipelineState> mPSO = nullptr;

    XMFLOAT4X4 mWorld = MathHelper::Identity4x4();
    XMFLOAT4X4 mView = MathHelper::Identity4x4();
    XMFLOAT4X4 mProj = MathHelper::Identity4x4();

    float mTheta = 1.5f*XM_PI;
    float mPhi = XM_PIDIV4;
    float mRadius = 5.0f;

	float mGameTime = 0.0f;

    POINT mLastMousePos;
};

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE prevInstance,
				   PSTR cmdLine, int showCmd)
{
	// Enable run-time memory check for debug builds.
#if defined(DEBUG) | defined(_DEBUG)
	_CrtSetDbgFlag( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
#endif

    try
    {
        BoxApp theApp(hInstance);
        if(!theApp.Initialize())
            return 0;

        return theApp.Run();
    }
    catch(DxException& e)
    {
        MessageBox(nullptr, e.ToString().c_str(), L"HR Failed", MB_OK);
        return 0;
    }
}

BoxApp::BoxApp(HINSTANCE hInstance)
: D3DApp(hInstance) 
{
}

BoxApp::~BoxApp()
{
}

bool BoxApp::Initialize()
{
    if(!D3DApp::Initialize())
		return false;
		
    // Reset the command list to prep for initialization commands.
    ThrowIfFailed(mCommandList->Reset(mDirectCmdListAlloc.Get(), nullptr));
 
    BuildDescriptorHeaps();
	BuildConstantBuffers();
    BuildRootSignature();
    BuildShadersAndInputLayout();
	
	//6.13 - Question 4 - BEGIN
    BuildBoxGeometry();
	//6.13 - Question 4 - END

	//6.13 - Question 5 - BEGIN
	//BuildPyramidGeometry();
	//6.13 - Question 54 - BEGIN

	//6.13 - Question 7 - BEGIN
	//BuildCombinedGeometry();
	//6.13 - Question 7 - END

	//6.13 - Question 10 - BEGIN
	//BuildBoxGeometryAlt();
	//6.13 - Question 10 - END

    BuildPSO();

    // Execute the initialization commands.
    ThrowIfFailed(mCommandList->Close());
	ID3D12CommandList* cmdsLists[] = { mCommandList.Get() };
	mCommandQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);

    // Wait until initialization is complete.
    FlushCommandQueue();

	return true;
}

void BoxApp::OnResize()
{
	D3DApp::OnResize();

    // The window resized, so update the aspect ratio and recompute the projection matrix.
    XMMATRIX P = XMMatrixPerspectiveFovLH(0.25f*MathHelper::Pi, AspectRatio(), 1.0f, 1000.0f);
    XMStoreFloat4x4(&mProj, P);
}

void BoxApp::Update(const GameTimer& gt)
{
    // Convert Spherical to Cartesian coordinates.
    float x = mRadius*sinf(mPhi)*cosf(mTheta);
    float z = mRadius*sinf(mPhi)*sinf(mTheta);
    float y = mRadius*cosf(mPhi);
	
    // Build the view matrix.
    XMVECTOR pos = XMVectorSet(x, y, z, 1.0f);
    XMVECTOR target = XMVectorZero();
    XMVECTOR up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

    XMMATRIX view = XMMatrixLookAtLH(pos, target, up);
    XMStoreFloat4x4(&mView, view);

    XMMATRIX world = XMLoadFloat4x4(&mWorld);
    XMMATRIX proj = XMLoadFloat4x4(&mProj);
    XMMATRIX worldViewProj = world*view*proj;

	// Update the constant buffer with the latest worldViewProj matrix.
	ObjectConstants objConstants;
    XMStoreFloat4x4(&objConstants.WorldViewProj, XMMatrixTranspose(worldViewProj));
	//6.13 - Question 6 - BEGIN
	mGameTime = gt.TotalTime();
	objConstants.gTime = mGameTime;
	//6.13 - Question 6 - END
    mObjectCB->CopyData(0, objConstants);
}

void BoxApp::Draw(const GameTimer& gt)
{
    // Reuse the memory associated with command recording.
    // We can only reset when the associated command lists have finished execution on the GPU.
	ThrowIfFailed(mDirectCmdListAlloc->Reset());

	// A command list can be reset after it has been added to the command queue via ExecuteCommandList.
    // Reusing the command list reuses memory.
    ThrowIfFailed(mCommandList->Reset(mDirectCmdListAlloc.Get(), mPSO.Get()));

    mCommandList->RSSetViewports(1, &mScreenViewport);
    mCommandList->RSSetScissorRects(1, &mScissorRect);

    // Indicate a state transition on the resource usage.
	mCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(CurrentBackBuffer(),
		D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET));

    // Clear the back buffer and depth buffer.
    mCommandList->ClearRenderTargetView(CurrentBackBufferView(), Colors::LightSteelBlue, 0, nullptr);
    mCommandList->ClearDepthStencilView(DepthStencilView(), D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);
	
    // Specify the buffers we are going to render to.
	mCommandList->OMSetRenderTargets(1, &CurrentBackBufferView(), true, &DepthStencilView());

	ID3D12DescriptorHeap* descriptorHeaps[] = { mCbvHeap.Get() };
	mCommandList->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);

	mCommandList->SetGraphicsRootSignature(mRootSignature.Get());

    mCommandList->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	//6.13 - Question 3 - BEGIN
	//mCommandList->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	//mCommandList->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP);
	//mCommandList->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
	//mCommandList->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
	//6.13 - Question 3 - END

    mCommandList->SetGraphicsRootDescriptorTable(0, mCbvHeap->GetGPUDescriptorHandleForHeapStart());

	//6.13 - Question 2 - BEGIN
	DrawCube();
	//6.13 - Question 2 - END

	//6.13 - Question 5 - BEGIN
	//DrawPyramid();
	//6.13 - Question 5 - END

	//6.13 - Question 7 - BEGIN
	//DrawCombined();
	//6.13 - Question 7 - END

	//6.13 - Question 10 - BEGIN
	//DrawCubeAlt();
	//6.13 - Question 10 - END
	
    // Indicate a state transition on the resource usage.
	mCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(CurrentBackBuffer(),
		D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT));

    // Done recording commands.
	ThrowIfFailed(mCommandList->Close());
 
    // Add the command list to the queue for execution.
	ID3D12CommandList* cmdsLists[] = { mCommandList.Get() };
	mCommandQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);
	
	// swap the back and front buffers
	ThrowIfFailed(mSwapChain->Present(0, 0));
	mCurrBackBuffer = (mCurrBackBuffer + 1) % SwapChainBufferCount;

	// Wait until frame commands are complete.  This waiting is inefficient and is
	// done for simplicity.  Later we will show how to organize our rendering code
	// so we do not have to wait per frame.
	FlushCommandQueue();
}

//6.13 - Question 2 - BEGIN
void BoxApp::DrawCube()
{
	//To Draw With interlaced Vertex Data Instead
	//mCommandList->IASetVertexBuffers(0, 1, &mBoxGeo->VertexBufferView());
	
	mCommandList->IASetVertexBuffers(0,1, &mBoxGeo->POSVertexBufferView());
	mCommandList->IASetVertexBuffers(1,1, &mBoxGeo->COLVertexBufferView());
	
	mCommandList->IASetIndexBuffer(&mBoxGeo->IndexBufferView());

	mCommandList->DrawIndexedInstanced(
		mBoxGeo->DrawArgs["box"].IndexCount,
		1, 0, 0, 0);
}
//6.13 - Question 2 - END

//6.13 - Question 5 - BEGIN
void BoxApp::DrawPyramid()
{
	mCommandList->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	mCommandList->IASetVertexBuffers(0, 1, &mPyramidGeo->POSVertexBufferView());
	mCommandList->IASetVertexBuffers(1, 1, &mPyramidGeo->COLVertexBufferView());
	mCommandList->IASetIndexBuffer(&mPyramidGeo->IndexBufferView());

	mCommandList->DrawIndexedInstanced(
		mPyramidGeo->DrawArgs["pyramid"].IndexCount,
		1, 0, 0, 0);
}
//6.13 - Question 5 - END

//6.13 - Question 5 - BEGIN
void BoxApp::DrawCombined()
{
	mCommandList->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	mCommandList->IASetVertexBuffers(0, 1, &mComboGeo->POSVertexBufferView());
	mCommandList->IASetVertexBuffers(1, 1, &mComboGeo->COLVertexBufferView());
	mCommandList->IASetIndexBuffer(&mComboGeo->IndexBufferView());

	mCommandList->DrawIndexedInstanced(
		mComboGeo->DrawArgs["Pyramid"].IndexCount,
		1,
		mComboGeo->DrawArgs["Pyramid"].StartIndexLocation,
		mComboGeo->DrawArgs["Pyramid"].BaseVertexLocation,
		0);

	mCommandList->DrawIndexedInstanced(
		mComboGeo->DrawArgs["Box"].IndexCount,
		1, 
		mComboGeo->DrawArgs["Box"].StartIndexLocation,
		mComboGeo->DrawArgs["Box"].BaseVertexLocation,
		0);
}
//6.13 - Question 5 - END

//6.13 - Question 10 - BEGIN
void BoxApp::DrawCubeAlt()
{
	//To Draw With interlaced Vertex Data Instead
	mCommandList->IASetVertexBuffers(0, 1, &mBoxGeo->VertexBufferView());
	mCommandList->IASetIndexBuffer(&mBoxGeo->IndexBufferView());

	mCommandList->DrawIndexedInstanced(
		mBoxGeo->DrawArgs["box"].IndexCount,
		1, 0, 0, 0);
}
//6.13 - Question 10 - END

void BoxApp::OnMouseDown(WPARAM btnState, int x, int y)
{
    mLastMousePos.x = x;
    mLastMousePos.y = y;

    SetCapture(mhMainWnd);
}

void BoxApp::OnMouseUp(WPARAM btnState, int x, int y)
{
    ReleaseCapture();
}

void BoxApp::OnMouseMove(WPARAM btnState, int x, int y)
{
    if((btnState & MK_LBUTTON) != 0)
    {
        // Make each pixel correspond to a quarter of a degree.
        float dx = XMConvertToRadians(0.25f*static_cast<float>(x - mLastMousePos.x));
        float dy = XMConvertToRadians(0.25f*static_cast<float>(y - mLastMousePos.y));

        // Update angles based on input to orbit camera around box.
        mTheta += dx;
        mPhi += dy;

        // Restrict the angle mPhi.
        mPhi = MathHelper::Clamp(mPhi, 0.1f, MathHelper::Pi - 0.1f);
    }
    else if((btnState & MK_RBUTTON) != 0)
    {
        // Make each pixel correspond to 0.005 unit in the scene.
        float dx = 0.005f*static_cast<float>(x - mLastMousePos.x);
        float dy = 0.005f*static_cast<float>(y - mLastMousePos.y);

        // Update the camera radius based on input.
        mRadius += dx - dy;

        // Restrict the radius.
        mRadius = MathHelper::Clamp(mRadius, 3.0f, 15.0f);
    }

    mLastMousePos.x = x;
    mLastMousePos.y = y;
}

void BoxApp::BuildDescriptorHeaps()
{
    D3D12_DESCRIPTOR_HEAP_DESC cbvHeapDesc;
    cbvHeapDesc.NumDescriptors = 1;
    cbvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
    cbvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	cbvHeapDesc.NodeMask = 0;
    ThrowIfFailed(md3dDevice->CreateDescriptorHeap(&cbvHeapDesc,
        IID_PPV_ARGS(&mCbvHeap)));
}

void BoxApp::BuildConstantBuffers()
{
	mObjectCB = std::make_unique<UploadBuffer<ObjectConstants>>(md3dDevice.Get(), 1, true);

	UINT objCBByteSize = d3dUtil::CalcConstantBufferByteSize(sizeof(ObjectConstants));

	D3D12_GPU_VIRTUAL_ADDRESS cbAddress = mObjectCB->Resource()->GetGPUVirtualAddress();
    // Offset to the ith object constant buffer in the buffer.
    int boxCBufIndex = 0;
	cbAddress += boxCBufIndex*objCBByteSize;

	D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc;
	cbvDesc.BufferLocation = cbAddress;
	cbvDesc.SizeInBytes = d3dUtil::CalcConstantBufferByteSize(sizeof(ObjectConstants));

	md3dDevice->CreateConstantBufferView(
		&cbvDesc,
		mCbvHeap->GetCPUDescriptorHandleForHeapStart());
}

void BoxApp::BuildRootSignature()
{
	// Shader programs typically require resources as input (constant buffers,
	// textures, samplers).  The root signature defines the resources the shader
	// programs expect.  If we think of the shader programs as a function, and
	// the input resources as function parameters, then the root signature can be
	// thought of as defining the function signature.  

	// Root parameter can be a table, root descriptor or root constants.
	CD3DX12_ROOT_PARAMETER slotRootParameter[1];

	// Create a single descriptor table of CBVs.
	CD3DX12_DESCRIPTOR_RANGE cbvTable;
	cbvTable.Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0);
	slotRootParameter[0].InitAsDescriptorTable(1, &cbvTable);

	// A root signature is an array of root parameters.
	CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc(1, slotRootParameter, 0, nullptr, 
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

	// create a root signature with a single slot which points to a descriptor range consisting of a single constant buffer
	ComPtr<ID3DBlob> serializedRootSig = nullptr;
	ComPtr<ID3DBlob> errorBlob = nullptr;
	HRESULT hr = D3D12SerializeRootSignature(&rootSigDesc, D3D_ROOT_SIGNATURE_VERSION_1,
		serializedRootSig.GetAddressOf(), errorBlob.GetAddressOf());

	if(errorBlob != nullptr)
	{
		::OutputDebugStringA((char*)errorBlob->GetBufferPointer());
	}
	ThrowIfFailed(hr);

	ThrowIfFailed(md3dDevice->CreateRootSignature(
		0,
		serializedRootSig->GetBufferPointer(),
		serializedRootSig->GetBufferSize(),
		IID_PPV_ARGS(&mRootSignature)));
}

void BoxApp::BuildShadersAndInputLayout()
{
    HRESULT hr = S_OK;
    
	mvsByteCode = d3dUtil::CompileShader(L"Shaders\\color.hlsl", nullptr, "VS", "vs_5_0");
	mpsByteCode = d3dUtil::CompileShader(L"Shaders\\color.hlsl", nullptr, "PS", "ps_5_0");

	//Interlaced Input Layout
    //mInputLayout =
    //{
    //    { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
    //    { "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
    //};

	//6.13 - Question 2 - BEGIN
	//Individual Buffers Layout
	mInputLayout =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
	};
	//6.13 - Question 2 - END
	
	//6.13 - Question 10 - BEGIN
	mInputLayoutAlt = 
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "COLOR", 0, DXGI_FORMAT_B8G8R8A8_UNORM, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
	};
	//6.13 - Question 10 - END


}

//6.13 - Question 2 - BEGIN
void BoxApp::BuildBoxGeometry()
{
	//Interlaced Vertex Data
    std::array<Vertex, 8> vertices =
    {
        Vertex({ XMFLOAT3(-1.0f, -1.0f, -1.0f), XMFLOAT4(Colors::Brown) }),
		Vertex({ XMFLOAT3(-1.0f, +1.0f, -1.0f), XMFLOAT4(Colors::Black) }),
		Vertex({ XMFLOAT3(+1.0f, +1.0f, -1.0f), XMFLOAT4(Colors::Red) }),
		Vertex({ XMFLOAT3(+1.0f, -1.0f, -1.0f), XMFLOAT4(Colors::Green) }),
		Vertex({ XMFLOAT3(-1.0f, -1.0f, +1.0f), XMFLOAT4(Colors::Blue) }),
		Vertex({ XMFLOAT3(-1.0f, +1.0f, +1.0f), XMFLOAT4(Colors::Yellow) }),
		Vertex({ XMFLOAT3(+1.0f, +1.0f, +1.0f), XMFLOAT4(Colors::Cyan) }),
		Vertex({ XMFLOAT3(+1.0f, -1.0f, +1.0f), XMFLOAT4(Colors::Magenta) })
    };

	//Position Only Vertices
	std::array<VertexPOS, 8> verticesPOS =
	{
		VertexPOS({ XMFLOAT3(-1.0f, -1.0f, -1.0f)}),
		VertexPOS({ XMFLOAT3(-1.0f, +1.0f, -1.0f)}),
		VertexPOS({ XMFLOAT3(+1.0f, +1.0f, -1.0f)}),
		VertexPOS({ XMFLOAT3(+1.0f, -1.0f, -1.0f)}),
		VertexPOS({ XMFLOAT3(-1.0f, -1.0f, +1.0f)}),
		VertexPOS({ XMFLOAT3(-1.0f, +1.0f, +1.0f)}),
		VertexPOS({ XMFLOAT3(+1.0f, +1.0f, +1.0f)}),
		VertexPOS({ XMFLOAT3(+1.0f, -1.0f, +1.0f)})
	};
	
	//Color Only Vertices
	std::array<VertexCOL, 8> verticesCOL =
	{
		VertexCOL({XMFLOAT4(Colors::Green) }),
		VertexCOL({XMFLOAT4(Colors::Blue) }),
		VertexCOL({XMFLOAT4(Colors::Red) }),
		VertexCOL({XMFLOAT4(Colors::Yellow) }),
		VertexCOL({XMFLOAT4(Colors::Cyan) }),
		VertexCOL({XMFLOAT4(Colors::Magenta) }),
		VertexCOL({XMFLOAT4(Colors::Orange) }),
		VertexCOL({XMFLOAT4(Colors::Purple) })
	};
	
	std::array<std::uint16_t, 36> indices =
	{
		// front face
		0, 1, 2,
		0, 2, 3,
	
		// back face
		4, 6, 5,
		4, 7, 6,
	
		// left face
		4, 5, 1,
		4, 1, 0,
	
		// right face
		3, 2, 6,
		3, 6, 7,
	
		// top face
		1, 5, 6,
		1, 6, 2,
	
		// bottom face
		4, 0, 3,
		4, 3, 7
	};

	mBoxGeo = std::make_unique<MeshGeometry>();
	mBoxGeo->Name = "boxGeo";
	
	//Vertex Bufffer Sizes (Interlaced, Pos, Color)
    const UINT vbByteSize = (UINT)vertices.size() * sizeof(Vertex);
	const UINT POSvbByteSize = (UINT)verticesPOS.size() * sizeof(VertexPOS);
	const UINT COLvbByteSize = (UINT)verticesCOL.size() * sizeof(VertexCOL);

	//Index Buyffer Size
	const UINT ibByteSize = (UINT)indices.size() * sizeof(std::uint16_t);
	
	//Create CPU Blob, and populate it with vertex data (Interlaces, Pos, Color, Index)
	ThrowIfFailed(D3DCreateBlob(vbByteSize, &mBoxGeo->VertexBufferCPU));
	CopyMemory(mBoxGeo->VertexBufferCPU->GetBufferPointer(), vertices.data(), vbByteSize);

	ThrowIfFailed(D3DCreateBlob(POSvbByteSize, &mBoxGeo->POSVertexBufferCPU));
	CopyMemory(mBoxGeo->POSVertexBufferCPU->GetBufferPointer(), verticesPOS.data(), POSvbByteSize);
	
	ThrowIfFailed(D3DCreateBlob(COLvbByteSize, &mBoxGeo->COLVertexBufferCPU));
	CopyMemory(mBoxGeo->COLVertexBufferCPU->GetBufferPointer(), verticesCOL.data(), COLvbByteSize);	
	
	ThrowIfFailed(D3DCreateBlob(ibByteSize, &mBoxGeo->IndexBufferCPU));
	CopyMemory(mBoxGeo->IndexBufferCPU->GetBufferPointer(), indices.data(), ibByteSize);

	//Create the GPU Buffers and issue the command to upload the data (Interlaces, Pos, Color, Index)
	mBoxGeo->VertexBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(),
		mCommandList.Get(), vertices.data(), vbByteSize, mBoxGeo->VertexBufferUploader);

	mBoxGeo->POSVertexBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(),
		mCommandList.Get(), verticesPOS.data(), POSvbByteSize, mBoxGeo->POSVertexBufferUploader);

	mBoxGeo->COLVertexBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(),
		mCommandList.Get(), verticesCOL.data(), COLvbByteSize, mBoxGeo->COLVertexBufferUploader);
	
	mBoxGeo->IndexBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(),
		mCommandList.Get(), indices.data(), ibByteSize, mBoxGeo->IndexBufferUploader);
	
	//Interlaced
	mBoxGeo->VertexByteStride = sizeof(Vertex);
	mBoxGeo->VertexBufferByteSize = vbByteSize;

	//POS Buffer
	mBoxGeo->POSVertexByteStride = sizeof(VertexPOS);
	mBoxGeo->POSVertexBufferByteSize = POSvbByteSize;

	//Color Buffer
	mBoxGeo->COLVertexByteStride = sizeof(VertexCOL);
	mBoxGeo->COLVertexBufferByteSize = COLvbByteSize;

	//Index Buffer Data
	mBoxGeo->IndexFormat = DXGI_FORMAT_R16_UINT;
	mBoxGeo->IndexBufferByteSize = ibByteSize;
	
	// Store This "Mesh" data in the Geo
	SubmeshGeometry submesh;
	submesh.IndexCount = (UINT)indices.size();
	submesh.StartIndexLocation = 0;
	submesh.BaseVertexLocation = 0;
	
	mBoxGeo->DrawArgs["box"] = submesh;
}
//6.13 - Question 2 - END

//6.13 - Question 4 - BEGIN
void BoxApp::BuildPyramidGeometry()
{
	std::array<VertexPOS, 5> verticesPOS =
	{
		VertexPOS({ XMFLOAT3(-1.0f, -1.0f, 1.0f)}), //0 - Bottom Left
		VertexPOS({ XMFLOAT3(-1.0f, +1.0f, 1.0f)}), //1 - Top Left
		VertexPOS({ XMFLOAT3(+1.0f, +1.0f, 1.0f)}), //2 - Top Right
		VertexPOS({ XMFLOAT3(+1.0f, -1.0f, 1.0f)}), //3 - Bottom Right
		VertexPOS({ XMFLOAT3(+0.0f, +0.0f,+3.0f)})  //4 - Peak
	};

	std::array<VertexCOL, 5> verticesCOL =
	{
		VertexCOL({XMFLOAT4(Colors::Green) }),   //0 - Bottom Left
		VertexCOL({XMFLOAT4(Colors::Blue) }),	 //1 - Top Left
		VertexCOL({XMFLOAT4(Colors::Yellow) }),	 //2 - Top Right
		VertexCOL({XMFLOAT4(Colors::Purple) }),	 //3 - Bottom Right
		VertexCOL({XMFLOAT4(Colors::Red) })		 //4 - Peak
	};

	std::array<std::uint16_t, 18> indices =
	{
		// Bottom face
		0, 1, 3,
		3, 1, 2,

		// back face
		4, 1, 0, //Correct

		// left face
		4, 2, 1,

		// right face
		4, 3, 2,

		// top face
		4, 0, 3
	};

	mPyramidGeo = std::make_unique<MeshGeometry>();
	mPyramidGeo->Name = "boxGeo";

	const UINT vbPOSByteSize = (UINT)verticesPOS.size() * sizeof(VertexPOS);
	const UINT vbCOLByteSize = (UINT)verticesCOL.size() * sizeof(VertexCOL);
	const UINT ibByteSize = (UINT)indices.size() * sizeof(std::uint16_t);

	//Create CPU Buffers
	ThrowIfFailed(D3DCreateBlob(vbPOSByteSize, &mPyramidGeo->POSVertexBufferCPU));
	CopyMemory(mPyramidGeo->POSVertexBufferCPU->GetBufferPointer(), verticesPOS.data(), vbPOSByteSize);

	ThrowIfFailed(D3DCreateBlob(vbCOLByteSize, &mPyramidGeo->COLVertexBufferCPU));
	CopyMemory(mPyramidGeo->COLVertexBufferCPU->GetBufferPointer(), verticesCOL.data(), vbCOLByteSize);

	ThrowIfFailed(D3DCreateBlob(ibByteSize, &mPyramidGeo->IndexBufferCPU));
	CopyMemory(mPyramidGeo->IndexBufferCPU->GetBufferPointer(), indices.data(), ibByteSize);

	//Create GPU Buffers and Issue Upload
	mPyramidGeo->POSVertexBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(),
		mCommandList.Get(), verticesPOS.data(), vbPOSByteSize, mPyramidGeo->POSVertexBufferUploader);

	mPyramidGeo->COLVertexBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(),
		mCommandList.Get(), verticesCOL.data(), vbCOLByteSize, mPyramidGeo->COLVertexBufferUploader);

	mPyramidGeo->IndexBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(),
		mCommandList.Get(), indices.data(), ibByteSize, mPyramidGeo->IndexBufferUploader);

	//Store Descriptions of Data
	mPyramidGeo->POSVertexByteStride = sizeof(VertexPOS);
	mPyramidGeo->POSVertexBufferByteSize = vbPOSByteSize;

	mPyramidGeo->COLVertexByteStride = sizeof(VertexCOL);
	mPyramidGeo->COLVertexBufferByteSize = vbCOLByteSize;

	mPyramidGeo->IndexFormat = DXGI_FORMAT_R16_UINT;
	mPyramidGeo->IndexBufferByteSize = ibByteSize;

	//Create Submesh
	SubmeshGeometry Trisubmesh;
	Trisubmesh.IndexCount = (UINT)indices.size();
	Trisubmesh.StartIndexLocation = 0;
	Trisubmesh.BaseVertexLocation = 0;

	mPyramidGeo->DrawArgs["pyramid"] = Trisubmesh;
}
//6.13 - Question 4 - END

//6.13 - Question 7 - BEGIN
void BoxApp::BuildCombinedGeometry()
{
	//Position Only Vertices
	std::array<VertexPOS, 13> verticesPOS =
	{
		//Box
		VertexPOS({ XMFLOAT3(-1.0f, -1.0f, -1.0f)}),
		VertexPOS({ XMFLOAT3(-1.0f, +1.0f, -1.0f)}),
		VertexPOS({ XMFLOAT3(+1.0f, +1.0f, -1.0f)}),
		VertexPOS({ XMFLOAT3(+1.0f, -1.0f, -1.0f)}),
		VertexPOS({ XMFLOAT3(-1.0f, -1.0f, +1.0f)}),
		VertexPOS({ XMFLOAT3(-1.0f, +1.0f, +1.0f)}),
		VertexPOS({ XMFLOAT3(+1.0f, +1.0f, +1.0f)}),
		VertexPOS({ XMFLOAT3(+1.0f, -1.0f, +1.0f)}),

		//Pyramid
		VertexPOS({ XMFLOAT3(-1.0f, -1.0f, 1.0f)}), //0 - Bottom Left
		VertexPOS({ XMFLOAT3(-1.0f, +1.0f, 1.0f)}), //1 - Top Left
		VertexPOS({ XMFLOAT3(+1.0f, +1.0f, 1.0f)}), //2 - Top Right
		VertexPOS({ XMFLOAT3(+1.0f, -1.0f, 1.0f)}), //3 - Bottom Right
		VertexPOS({ XMFLOAT3(+0.0f, +0.0f,+3.0f)})  //4 - Peak
	};

	//Color Only Vertices
	std::array<VertexCOL, 13> verticesCOL =
	{
		VertexCOL({XMFLOAT4(Colors::Green) }),
		VertexCOL({XMFLOAT4(Colors::Blue) }),
		VertexCOL({XMFLOAT4(Colors::Green) }),
		VertexCOL({XMFLOAT4(Colors::Blue) }),
		VertexCOL({XMFLOAT4(Colors::Green) }),
		VertexCOL({XMFLOAT4(Colors::Blue) }),
		VertexCOL({XMFLOAT4(Colors::Green) }),
		VertexCOL({XMFLOAT4(Colors::Blue) }),

		//Pyramid
		VertexCOL({XMFLOAT4(Colors::Green) }),   //0 - Bottom Left
		VertexCOL({XMFLOAT4(Colors::Blue) }),	 //1 - Top Left
		VertexCOL({XMFLOAT4(Colors::Yellow) }),	 //2 - Top Right
		VertexCOL({XMFLOAT4(Colors::Purple) }),	 //3 - Bottom Right
		VertexCOL({XMFLOAT4(Colors::Red) })		 //4 - Peak
	};

	std::array<std::uint16_t, 54> indices =
	{
		// front face
		0, 1, 2,
		0, 2, 3,

		// back face
		4, 6, 5,
		4, 7, 6,

		// left face
		4, 5, 1,
		4, 1, 0,

		// right face
		3, 2, 6,
		3, 6, 7,

		// top face
		1, 5, 6,
		1, 6, 2,

		// bottom face
		4, 0, 3,
		4, 3, 7,

		//Pyramid
		0, 1, 3,
		3, 1, 2,
		4, 1, 0,
		4, 2, 1,
		4, 3, 2,
		4, 0, 3
	};

	mComboGeo = std::make_unique<MeshGeometry>();
	mComboGeo->Name = "boxGeo";

	//Vertex Bufffer Sizes (Interlaced, Pos, Color)
	const UINT POSvbByteSize = (UINT)verticesPOS.size() * sizeof(VertexPOS);
	const UINT COLvbByteSize = (UINT)verticesCOL.size() * sizeof(VertexCOL);

	//Index Buyffer Size
	const UINT ibByteSize = (UINT)indices.size() * sizeof(std::uint16_t);

	//Create CPU Blob, and populate it with vertex data (Interlaces, Pos, Color, Index)
	ThrowIfFailed(D3DCreateBlob(POSvbByteSize, &mComboGeo->POSVertexBufferCPU));
	CopyMemory(mComboGeo->POSVertexBufferCPU->GetBufferPointer(), verticesPOS.data(), POSvbByteSize);

	ThrowIfFailed(D3DCreateBlob(COLvbByteSize, &mComboGeo->COLVertexBufferCPU));
	CopyMemory(mComboGeo->COLVertexBufferCPU->GetBufferPointer(), verticesCOL.data(), COLvbByteSize);

	ThrowIfFailed(D3DCreateBlob(ibByteSize, &mComboGeo->IndexBufferCPU));
	CopyMemory(mComboGeo->IndexBufferCPU->GetBufferPointer(), indices.data(), ibByteSize);

	//Create the GPU Buffers and issue the command to upload the data (Interlaces, Pos, Color, Index)
	mComboGeo->POSVertexBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(),
		mCommandList.Get(), verticesPOS.data(), POSvbByteSize, mComboGeo->POSVertexBufferUploader);

	mComboGeo->COLVertexBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(),
		mCommandList.Get(), verticesCOL.data(), COLvbByteSize, mComboGeo->COLVertexBufferUploader);

	mComboGeo->IndexBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(),
		mCommandList.Get(), indices.data(), ibByteSize, mComboGeo->IndexBufferUploader);

	//POS Buffer
	mComboGeo->POSVertexByteStride = sizeof(VertexPOS);
	mComboGeo->POSVertexBufferByteSize = POSvbByteSize;

	//Color Buffer
	mComboGeo->COLVertexByteStride = sizeof(VertexCOL);
	mComboGeo->COLVertexBufferByteSize = COLvbByteSize;

	//Index Buffer Data
	mComboGeo->IndexFormat = DXGI_FORMAT_R16_UINT;
	mComboGeo->IndexBufferByteSize = ibByteSize;

	// Store Box mesh data
	SubmeshGeometry submesh;
	submesh.IndexCount = (UINT)indices.size();
	submesh.StartIndexLocation = 0;
	submesh.BaseVertexLocation = 0;

	mComboGeo->DrawArgs["Box"] = submesh;

	// Store Box mesh data
	{
		SubmeshGeometry submesh;
		submesh.IndexCount = 36;
		submesh.StartIndexLocation = 0;
		submesh.BaseVertexLocation = 0;

		mComboGeo->DrawArgs["Box"] = submesh;
	}
	//Store the Pyramid Submesh
	{
		SubmeshGeometry submesh;
		submesh.IndexCount = 18;
		submesh.StartIndexLocation = 36;
		submesh.BaseVertexLocation = 8;

		mComboGeo->DrawArgs["Pyramid"] = submesh;
	}
	
}
//6.13 - Question 7 - END

//6.13 - Question 10 - BEGIN
void BoxApp::BuildBoxGeometryAlt()
{
	//Interlaced Vertex Data
	std::array<VertexAlt, 8> vertices =
	{
		VertexAlt({ XMFLOAT3(-1.0f, -1.0f, -1.0f), XMCOLOR(Colors::Brown) }),
		VertexAlt({ XMFLOAT3(-1.0f, +1.0f, -1.0f), XMCOLOR(Colors::Black) }),
		VertexAlt({ XMFLOAT3(+1.0f, +1.0f, -1.0f), XMCOLOR(Colors::Red) }),
		VertexAlt({ XMFLOAT3(+1.0f, -1.0f, -1.0f), XMCOLOR(Colors::Green) }),
		VertexAlt({ XMFLOAT3(-1.0f, -1.0f, +1.0f), XMCOLOR(Colors::Blue) }),
		VertexAlt({ XMFLOAT3(-1.0f, +1.0f, +1.0f), XMCOLOR(Colors::Yellow) }),
		VertexAlt({ XMFLOAT3(+1.0f, +1.0f, +1.0f), XMCOLOR(Colors::Cyan) }),
		VertexAlt({ XMFLOAT3(+1.0f, -1.0f, +1.0f), XMCOLOR(Colors::Magenta) })
	};

	std::array<std::uint16_t, 36> indices =
	{
		// front face
		0, 1, 2,
		0, 2, 3,

		// back face
		4, 6, 5,
		4, 7, 6,

		// left face
		4, 5, 1,
		4, 1, 0,

		// right face
		3, 2, 6,
		3, 6, 7,

		// top face
		1, 5, 6,
		1, 6, 2,

		// bottom face
		4, 0, 3,
		4, 3, 7
	};

	mBoxGeo = std::make_unique<MeshGeometry>();
	mBoxGeo->Name = "boxGeo";

	//Vertex Bufffer Sizes (Interlaced, Pos, Color)
	const UINT vbByteSize = (UINT)vertices.size() * sizeof(VertexAlt);
	
	//Index Buyffer Size
	const UINT ibByteSize = (UINT)indices.size() * sizeof(std::uint16_t);

	//Create CPU Blob, and populate it with vertex data (Interlaces, Pos, Color, Index)
	ThrowIfFailed(D3DCreateBlob(vbByteSize, &mBoxGeo->VertexBufferCPU));
	CopyMemory(mBoxGeo->VertexBufferCPU->GetBufferPointer(), vertices.data(), vbByteSize);

	ThrowIfFailed(D3DCreateBlob(ibByteSize, &mBoxGeo->IndexBufferCPU));
	CopyMemory(mBoxGeo->IndexBufferCPU->GetBufferPointer(), indices.data(), ibByteSize);

	//Create the GPU Buffers and issue the command to upload the data (Interlaces, Pos, Color, Index)
	mBoxGeo->VertexBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(),
		mCommandList.Get(), vertices.data(), vbByteSize, mBoxGeo->VertexBufferUploader);

	mBoxGeo->IndexBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(),
		mCommandList.Get(), indices.data(), ibByteSize, mBoxGeo->IndexBufferUploader);

	//Interlaced
	mBoxGeo->VertexByteStride = sizeof(VertexAlt);
	mBoxGeo->VertexBufferByteSize = vbByteSize;

	//Index Buffer Data
	mBoxGeo->IndexFormat = DXGI_FORMAT_R16_UINT;
	mBoxGeo->IndexBufferByteSize = ibByteSize;

	// Store This "Mesh" data in the Geo
	SubmeshGeometry submesh;
	submesh.IndexCount = (UINT)indices.size();
	submesh.StartIndexLocation = 0;
	submesh.BaseVertexLocation = 0;

	mBoxGeo->DrawArgs["box"] = submesh;
}
//6.13 - Question 10 - END

void BoxApp::BuildPSO()
{
    D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc;
    ZeroMemory(&psoDesc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));

	psoDesc.InputLayout = { mInputLayout.data(), (UINT)mInputLayout.size() };

	//6.13 - Question 10 - BEGIN
	//psoDesc.InputLayout = { mInputLayoutAlt.data(), (UINT)mInputLayoutAlt.size() };
	//6.13 - Question 10 - END

    psoDesc.pRootSignature = mRootSignature.Get();
    psoDesc.VS = 
	{ 
		reinterpret_cast<BYTE*>(mvsByteCode->GetBufferPointer()), 
		mvsByteCode->GetBufferSize() 
	};
    psoDesc.PS = 
	{ 
		reinterpret_cast<BYTE*>(mpsByteCode->GetBufferPointer()), 
		mpsByteCode->GetBufferSize() 
	};

	//6.13 - Question 8/9 - BEGIN
	CD3DX12_RASTERIZER_DESC rasterizerDesc(D3D12_DEFAULT);
	//rasterizerDesc.FillMode = D3D12_FILL_MODE_WIREFRAME;
	//rasterizerDesc.CullMode = D3D12_CULL_MODE_NONE;

	psoDesc.RasterizerState = rasterizerDesc; // CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	//6.13 - Question 8/9 - END

    psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
    psoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
    psoDesc.SampleMask = UINT_MAX;
    psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    psoDesc.NumRenderTargets = 1;
    psoDesc.RTVFormats[0] = mBackBufferFormat;
    psoDesc.SampleDesc.Count = m4xMsaaState ? 4 : 1;
    psoDesc.SampleDesc.Quality = m4xMsaaState ? (m4xMsaaQuality - 1) : 0;
    psoDesc.DSVFormat = mDepthStencilFormat;
    ThrowIfFailed(md3dDevice->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&mPSO)));
}