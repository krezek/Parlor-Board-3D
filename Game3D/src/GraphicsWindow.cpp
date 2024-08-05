#include "pch.h"
#include "platform.h"

#include <GraphicsWindow.h>
#include <WUtil.h>
#include <d3dUtil.h>
#include <GeometryGenerator.h>
#include <FrameResource.h>
#include <DDSTextureLoader.h>

using namespace DirectX;

const int gNumFrameResources = 3;
int g_ObjCBIndex = 0;

GraphicsWindow::GraphicsWindow()
{
	_Sky = std::make_unique<Sky>();
	_Fixed = std::make_unique<Fixed>();
	_Board = std::make_unique<Board>();
	_Dice = std::make_unique<Dice>();
	_Wheel = std::make_unique<Wheel>();

	_Monkey = std::make_unique<Monkey>();
	_Bird = std::make_unique<Bird>();

	_Fountain = std::make_unique<Fountain>();
}

void GraphicsWindow::InitDirect3D()
{
	AbstractWindow::InitDirect3D();

	ThrowIfFailed(_CommandList->Reset(_DirectCmdListAlloc.Get(), nullptr));

	_CbvSrvDescriptorSize = _d3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	LoadModels();
	LoadTextures();
	BuildRootSignature();
	BuildShadersAndInputLayout();
	BuildGeometry();
	BuildMaterials();
	BuildRenderItems();
	BuildFrameResources();
	BuildDescriptorHeaps();
	BuildPSOs();

	ThrowIfFailed(_CommandList->Close());
	ID3D12CommandList* cmdsLists[] = { _CommandList.Get() };
	_CommandQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);

	FlushCommandQueue();
}

void GraphicsWindow::Draw()
{
	auto cmdListAlloc = _CurrFrameResource->CmdListAlloc;

	ThrowIfFailed(cmdListAlloc->Reset());

	ThrowIfFailed(_CommandList->Reset(cmdListAlloc.Get(), _PSOs["opaque"].Get()));

	_CommandList->RSSetViewports(1, &_ScreenViewport);
	_CommandList->RSSetScissorRects(1, &_ScissorRect);

	_CommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(CurrentBackBuffer(),
		D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET));

	_CommandList->ClearRenderTargetView(CurrentBackBufferView(), DirectX::Colors::Black, 0, nullptr);
	_CommandList->ClearDepthStencilView(DepthStencilView(), D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);

	_CommandList->OMSetRenderTargets(1, &CurrentBackBufferView(), true, &DepthStencilView());

	ID3D12DescriptorHeap* descriptorHeaps[] = { _SrvDescriptorHeap.Get() };
	_CommandList->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);

	_CommandList->SetGraphicsRootSignature(_RootSignature.Get());

	auto passCB = _CurrFrameResource->PassCB->Resource();
	_CommandList->SetGraphicsRootConstantBufferView(2, passCB->GetGPUVirtualAddress());

	CD3DX12_GPU_DESCRIPTOR_HANDLE skyTexDescriptor(_SrvDescriptorHeap->GetGPUDescriptorHandleForHeapStart());
	skyTexDescriptor.Offset(0, _CbvSrvUavDescriptorSize);
	_CommandList->SetGraphicsRootDescriptorTable(5, skyTexDescriptor);

	_CommandList->SetPipelineState(_PSOs["sky"].Get());
	DrawRenderItems(_CommandList.Get(), _RitemLayer[(int)RenderLayer::Sky]);
	
	_CommandList->SetPipelineState(_PSOs["fixed"].Get());
	DrawRenderItems(_CommandList.Get(), _RitemLayer[(int)RenderLayer::Fixed]);

	_CommandList->SetPipelineState(_PSOs["opaque"].Get());
	DrawRenderItems(_CommandList.Get(), _RitemLayer[(int)RenderLayer::Opaque]);

	_CommandList->SetPipelineState(_PSOs["skinned"].Get());
	DrawRenderItems(_CommandList.Get(), _RitemLayer[(int)RenderLayer::Skinned]);

	
	_CommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(CurrentBackBuffer(),
		D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT));

	ThrowIfFailed(_CommandList->Close());

	ID3D12CommandList* cmdsLists[] = { _CommandList.Get() };
	_CommandQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);

	RenderUI();

	ThrowIfFailed(_SwapChain->Present(0, 0));
	_CurrBackBuffer = (_CurrBackBuffer + 1) % SwapChainBufferCount;

	_CurrFrameResource->Fence = ++_CurrentFence;

	_CommandQueue->Signal(_Fence.Get(), _CurrentFence);
}

void GraphicsWindow::Update()
{
	UpdateCamera(_game_timer);
	UpdateFixedCamera(_game_timer);

	_CurrFrameResourceIndex = (_CurrFrameResourceIndex + 1) % gNumFrameResources;
	_CurrFrameResource = _FrameResources[_CurrFrameResourceIndex].get();

	if (_CurrFrameResource->Fence != 0 && _Fence->GetCompletedValue() < _CurrFrameResource->Fence)
	{
		HANDLE eventHandle = CreateEventEx(nullptr, false, false, EVENT_ALL_ACCESS);
		ThrowIfFailed(_Fence->SetEventOnCompletion(_CurrFrameResource->Fence, eventHandle));
		WaitForSingleObject(eventHandle, INFINITE);
		CloseHandle(eventHandle);
	}

	UpdateObjectCBs(_game_timer);
	UpdateMaterialCBs(_game_timer);
	UpdateMainPassCB(_game_timer);
	
	_Dice->Update(this, _game_timer);
	
	_Monkey->Update(this, _CurrFrameResource, _game_timer);
	_Bird->Update(this, _CurrFrameResource, _game_timer);

	_Wheel->Update(this, _game_timer);
}

LRESULT GraphicsWindow::OnResize()
{
	AbstractWindow::OnResize();

	DirectX::XMMATRIX P = DirectX::XMMatrixPerspectiveFovLH(0.25f * DirectX::XM_PI, AspectRatio(), 1.0f, 1000.0f);
	XMStoreFloat4x4(&_Proj, P);

	return 0;
}

LRESULT GraphicsWindow::OnMouseDown(WPARAM btnState, int x, int y)
{
	if (GetKeyState(VK_CONTROL) < 0)
	{
		_LastMousePos.x = x;
		_LastMousePos.y = y;

		SetCapture(_hWnd);
	}
	else
	{
		Pick(x, y);
		PickFixed(x, y);
		PickMonkey(x, y);
	}

	return 0;
}

LRESULT GraphicsWindow::OnMouseUp(WPARAM btnState, int x, int y)
{
	if (GetKeyState(VK_CONTROL) < 0)
	{
		ReleaseCapture();
	}

	KillTimer(Window(), IDT_TIMER_UP);
	KillTimer(Window(), IDT_TIMER_DOWN);
	KillTimer(Window(), IDT_TIMER_LEFT);
	KillTimer(Window(), IDT_TIMER_RIGHT);
	KillTimer(Window(), IDT_TIMER_IN);
	KillTimer(Window(), IDT_TIMER_OUT);

	return 0;
}

LRESULT GraphicsWindow::OnMouseMove(WPARAM btnState, int x, int y)
{
	if (GetCapture() != _hWnd)
		return 0;
	
	if (GetKeyState(VK_CONTROL) < 0)
	{
		if ((btnState & MK_LBUTTON) != 0)
		{
			float dx = DirectX::XMConvertToRadians(0.25f * static_cast<float>(x - _LastMousePos.x));
			float dy = DirectX::XMConvertToRadians(0.25f * static_cast<float>(y - _LastMousePos.y));

			_Theta += dx;
			_Phi += dy;

			_Phi = MathHelper::Clamp(_Phi, 0.1f, DirectX::XM_PIDIV2 - 0.1f);
		}
		else if ((btnState & MK_RBUTTON) != 0)
		{
			float dx = 0.2f * static_cast<float>(x - _LastMousePos.x);
			float dy = 0.2f * static_cast<float>(y - _LastMousePos.y);

			_Radius += dx - dy;

			_Radius = MathHelper::Clamp(_Radius, 7.0f, 15.0f);
		}

		_LastMousePos.x = x;
		_LastMousePos.y = y;
	}
	
	return 0;
}

void GraphicsWindow::LoadTextures()
{
	std::vector<std::string> texNames =
	{
		"skyTex",

		"upTex",
		"downTex",
		"leftTex",
		"rightTex",
		"zoominTex",
		"zoomoutTex",
		"new_gameTex",

		"groundTex",
		"diskRoadTex",
		"diskP1Tex",
		"diskP2Tex",
		"ringTex",
		
		"dice1Tex",
		"dice2Tex",
		"dice3Tex",
		"dice4Tex",
		"dice5Tex",
		"dice6Tex",
		
		"winTex",
		"lostTex",
		
		"monkeyHTex",
		"monkeyBTex",
		"monkeyETex",
		"birdTex",

		"logoTex",

		"fountainTex"
	};

	std::vector<std::wstring> texFilenames =
	{
		TEXTURE_PATH L"sky.dds",

		TEXTURE_PATH L"up.dds",
		TEXTURE_PATH L"down.dds",
		TEXTURE_PATH L"left.dds",
		TEXTURE_PATH L"right.dds",
		TEXTURE_PATH L"zoomin.dds",
		TEXTURE_PATH L"zoomout.dds",
		TEXTURE_PATH L"new_game.dds",

		TEXTURE_PATH L"ground.dds",
		TEXTURE_PATH L"disk-road.dds",
		TEXTURE_PATH L"disk-p1.dds",
		TEXTURE_PATH L"disk-p2.dds",
		TEXTURE_PATH L"ring.dds",
		
		TEXTURE_PATH L"dice1.dds",
		TEXTURE_PATH L"dice2.dds",
		TEXTURE_PATH L"dice3.dds",
		TEXTURE_PATH L"dice4.dds",
		TEXTURE_PATH L"dice5.dds",
		TEXTURE_PATH L"dice6.dds",
		TEXTURE_PATH L"win.dds",
		TEXTURE_PATH L"lost.dds",
		
		TEXTURE_PATH L"Monkey_Head.dds",
		TEXTURE_PATH L"Monkey_Body.dds",
		TEXTURE_PATH L"Monkey_Eye.dds",
		TEXTURE_PATH L"Bird.dds",

		TEXTURE_PATH L"logo.dds",

		TEXTURE_PATH L"fountain.dds"
	};

	for (int i = 0; i < (int)texNames.size(); ++i)
	{
		auto tex = std::make_unique<Texture>();
		tex->Name = texNames[i];
		tex->Filename = texFilenames[i];
		ThrowIfFailed(DirectX::CreateDDSTextureFromFile12(_d3dDevice.Get(),
			_CommandList.Get(), tex->Filename.c_str(),
			tex->Resource, tex->UploadHeap));

		_Textures[tex->Name] = std::move(tex);
	}
}

void GraphicsWindow::LoadModels()
{
	_Monkey->LoadModel();
	_Bird->LoadModel();

	_Fountain->LoadModel();
}

void GraphicsWindow::BuildRootSignature()
{
	CD3DX12_DESCRIPTOR_RANGE texTable0;
	texTable0.Init(
		D3D12_DESCRIPTOR_RANGE_TYPE_SRV,
		1,
		0);

	CD3DX12_DESCRIPTOR_RANGE texTable1;
	texTable1.Init(
		D3D12_DESCRIPTOR_RANGE_TYPE_SRV,
		1,
		1);

	CD3DX12_ROOT_PARAMETER slotRootParameter[6];

	slotRootParameter[0].InitAsDescriptorTable(1, &texTable0, D3D12_SHADER_VISIBILITY_PIXEL);
	slotRootParameter[1].InitAsConstantBufferView(0);
	slotRootParameter[2].InitAsConstantBufferView(1);
	slotRootParameter[3].InitAsConstantBufferView(2);
	slotRootParameter[4].InitAsConstantBufferView(3);
	slotRootParameter[5].InitAsDescriptorTable(1, &texTable1, D3D12_SHADER_VISIBILITY_PIXEL);

	auto staticSamplers = GetStaticSamplers();

	CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc(6, slotRootParameter,
		(UINT)staticSamplers.size(), staticSamplers.data(),
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

	Microsoft::WRL::ComPtr<ID3DBlob> serializedRootSig = nullptr;
	Microsoft::WRL::ComPtr<ID3DBlob> errorBlob = nullptr;
	HRESULT hr = D3D12SerializeRootSignature(&rootSigDesc, D3D_ROOT_SIGNATURE_VERSION_1,
		serializedRootSig.GetAddressOf(), errorBlob.GetAddressOf());

	if (errorBlob != nullptr)
	{
		::OutputDebugStringA((char*)errorBlob->GetBufferPointer());
	}
	ThrowIfFailed(hr);

	ThrowIfFailed(_d3dDevice->CreateRootSignature(
		0,
		serializedRootSig->GetBufferPointer(),
		serializedRootSig->GetBufferSize(),
		IID_PPV_ARGS(_RootSignature.GetAddressOf())));
}

void GraphicsWindow::BuildShadersAndInputLayout()
{
	const D3D_SHADER_MACRO skinnedDefines[] =
	{
		"SKINNED", "1",
		NULL, NULL
	};

	_Shaders["opaqueVS"] = d3dUtil::LoadBinary(SHADER_PATH L"default_vs.cso");
		//d3dUtil::CompileShader(SHADER_PATH L"Default.hlsl", nullptr, "VS", "vs_5_1");
	_Shaders["opaquePS"] = d3dUtil::LoadBinary(SHADER_PATH L"default_ps.cso");
		//d3dUtil::CompileShader(SHADER_PATH L"Default.hlsl", nullptr, "PS", "ps_5_1");

	_Shaders["skinnedVS"] = d3dUtil::LoadBinary(SHADER_PATH L"skinned_vs.cso");
		//d3dUtil::CompileShader(SHADER_PATH L"Default.hlsl", skinnedDefines, "VS", "vs_5_1");
	_Shaders["skinnedPS"] = d3dUtil::LoadBinary(SHADER_PATH L"skinned_ps.cso");
		//d3dUtil::CompileShader(SHADER_PATH L"Default.hlsl", skinnedDefines, "PS", "ps_5_1");

	_Shaders["fixedVS"] = d3dUtil::LoadBinary(SHADER_PATH L"fixed_vs.cso");
	//d3dUtil::CompileShader(SHADER_PATH L"Fixed.hlsl", nullptr, "VS", "vs_5_1");
	_Shaders["fixedPS"] = d3dUtil::LoadBinary(SHADER_PATH L"fixed_ps.cso");
	//d3dUtil::CompileShader(SHADER_PATH L"Fixed.hlsl", nullptr, "PS", "ps_5_1");

	_Shaders["skyVS"] = d3dUtil::LoadBinary(SHADER_PATH L"sky_vs.cso");
	//d3dUtil::CompileShader(SHADER_PATH L"Sky.hlsl", nullptr, "VS", "vs_5_1");
	_Shaders["skyPS"] = d3dUtil::LoadBinary(SHADER_PATH L"sky_ps.cso");
	//d3dUtil::CompileShader(SHADER_PATH L"Sky.hlsl", nullptr, "PS", "ps_5_1");

	_InputLayout =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
	};

	_SkinnedInputLayout =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "WEIGHTS", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 32, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "BONEINDICES", 0, DXGI_FORMAT_R32G32B32A32_UINT, 0, 48, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
	};
}

void GraphicsWindow::BuildGeometry()
{
	_Sky->BuildGeometry(_d3dDevice.Get(), _CommandList.Get(), _Geometries);
	_Fixed->BuildGeometry(_d3dDevice.Get(), _CommandList.Get(), _Geometries);

	_Board->BuildGeometry(_d3dDevice.Get(), _CommandList.Get(), _Geometries);
	_Dice->BuildGeometry(_d3dDevice.Get(), _CommandList.Get(), _Geometries);
	_Wheel->BuildGeometry(_d3dDevice.Get(), _CommandList.Get(), _Geometries);

	_Monkey->BuildSkinnedModel(_d3dDevice.Get(), _CommandList.Get(), _Geometries);
	_Bird->BuildSkinnedModel(_d3dDevice.Get(), _CommandList.Get(), _Geometries);

	_Fountain->BuildGeometry(_d3dDevice.Get(), _CommandList.Get(), _Geometries);
}

void GraphicsWindow::BuildRenderItems()
{
	_Sky->BuildRenderItems(_Geometries, _Materials, _AllRitems, _RitemLayer[(int)RenderLayer::Sky]);
	_Fixed->BuildRenderItems(_Geometries, _Materials, _AllRitems, _RitemLayer[(int)RenderLayer::Fixed]);

	_Board->BuildRenderItems(_Geometries, _Materials, _AllRitems, _RitemLayer[(int)RenderLayer::Opaque]);
	_Dice->BuildRenderItems(_Geometries, _Materials, _AllRitems, _RitemLayer[(int)RenderLayer::Opaque]);
	_Wheel->BuildRenderItems(_Geometries, _Materials, _AllRitems, _RitemLayer[(int)RenderLayer::Opaque]);

	_Monkey->BuildRenderItems(_Geometries, _Materials, _AllRitems, _RitemLayer[(int)RenderLayer::Skinned]);
	_Bird->BuildRenderItems(_Geometries, _Materials, _AllRitems, _RitemLayer[(int)RenderLayer::Skinned]);

	_Fountain->BuildRenderItems(_Geometries, _Materials, _AllRitems, _RitemLayer[(int)RenderLayer::Opaque]);
}

void GraphicsWindow::BuildPSOs()
{
	D3D12_GRAPHICS_PIPELINE_STATE_DESC opaquePsoDesc;

	//
	// PSO for opaque objects.
	//
	ZeroMemory(&opaquePsoDesc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));
	opaquePsoDesc.InputLayout = { _InputLayout.data(), (UINT)_InputLayout.size() };
	opaquePsoDesc.pRootSignature = _RootSignature.Get();
	opaquePsoDesc.VS =
	{
		reinterpret_cast<BYTE*>(_Shaders["opaqueVS"]->GetBufferPointer()),
		_Shaders["opaqueVS"]->GetBufferSize()
	};
	opaquePsoDesc.PS =
	{
		reinterpret_cast<BYTE*>(_Shaders["opaquePS"]->GetBufferPointer()),
		_Shaders["opaquePS"]->GetBufferSize()
	};
	opaquePsoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	//opaquePsoDesc.RasterizerState.FillMode = D3D12_FILL_MODE_WIREFRAME;
	opaquePsoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	opaquePsoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
	opaquePsoDesc.SampleMask = UINT_MAX;
	opaquePsoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	opaquePsoDesc.NumRenderTargets = 1;
	opaquePsoDesc.RTVFormats[0] = _BackBufferFormat;
	opaquePsoDesc.SampleDesc.Count = _4xMsaaState ? 4 : 1;
	opaquePsoDesc.SampleDesc.Quality = _4xMsaaState ? (_4xMsaaQuality - 1) : 0;
	opaquePsoDesc.DSVFormat = _DepthStencilFormat;
	ThrowIfFailed(_d3dDevice->CreateGraphicsPipelineState(&opaquePsoDesc, IID_PPV_ARGS(&_PSOs["opaque"])));
	
	//
	// PSO for Skinned Objects.
	//
	D3D12_GRAPHICS_PIPELINE_STATE_DESC skinnedPsoDesc = opaquePsoDesc;
	skinnedPsoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
	skinnedPsoDesc.InputLayout = { _SkinnedInputLayout.data(), (UINT)_SkinnedInputLayout.size() };
	skinnedPsoDesc.VS =
	{
		reinterpret_cast<BYTE*>(_Shaders["skinnedVS"]->GetBufferPointer()),
		_Shaders["skinnedVS"]->GetBufferSize()
	};
	skinnedPsoDesc.PS =
	{
		reinterpret_cast<BYTE*>(_Shaders["skinnedPS"]->GetBufferPointer()),
		_Shaders["skinnedPS"]->GetBufferSize()
	};
	ThrowIfFailed(_d3dDevice->CreateGraphicsPipelineState(&skinnedPsoDesc, IID_PPV_ARGS(&_PSOs["skinned"])));

	//
	// PSO for Fixed Objects.
	//
	D3D12_GRAPHICS_PIPELINE_STATE_DESC fixedPsoDesc = opaquePsoDesc;
	fixedPsoDesc.VS =
	{
		reinterpret_cast<BYTE*>(_Shaders["fixedVS"]->GetBufferPointer()),
		_Shaders["fixedVS"]->GetBufferSize()
	};
	fixedPsoDesc.PS =
	{
		reinterpret_cast<BYTE*>(_Shaders["fixedPS"]->GetBufferPointer()),
		_Shaders["fixedPS"]->GetBufferSize()
	};
	ThrowIfFailed(_d3dDevice->CreateGraphicsPipelineState(&fixedPsoDesc, IID_PPV_ARGS(&_PSOs["fixed"])));

	
	//
	// PSO for sky.
	//
	D3D12_GRAPHICS_PIPELINE_STATE_DESC skyPsoDesc = opaquePsoDesc;

	skyPsoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
	skyPsoDesc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
	skyPsoDesc.pRootSignature = _RootSignature.Get();
	skyPsoDesc.VS =
	{
		reinterpret_cast<BYTE*>(_Shaders["skyVS"]->GetBufferPointer()),
		_Shaders["skyVS"]->GetBufferSize()
	};
	skyPsoDesc.PS =
	{
		reinterpret_cast<BYTE*>(_Shaders["skyPS"]->GetBufferPointer()),
		_Shaders["skyPS"]->GetBufferSize()
	};
	ThrowIfFailed(_d3dDevice->CreateGraphicsPipelineState(&skyPsoDesc, IID_PPV_ARGS(&_PSOs["sky"])));
	
}

void GraphicsWindow::BuildFrameResources()
{
	for (int i = 0; i < gNumFrameResources; ++i)
	{
		_FrameResources.push_back(std::make_unique<FrameResource>(_d3dDevice.Get(),
			1, (UINT)_AllRitems.size(), (UINT)_Materials.size(), 
			MONKEY_MESH_COUNT * 4 +
			BIRD_MESH_COUNT * 4));
	}
}

void GraphicsWindow::DrawRenderItems(ID3D12GraphicsCommandList* cmdList, const std::vector<RenderItem*>& ritems)
{
	UINT objCBByteSize = d3dUtil::CalcConstantBufferByteSize(sizeof(ObjectConstants));
	UINT matCBByteSize = d3dUtil::CalcConstantBufferByteSize(sizeof(MaterialConstants));
	UINT skinnedCBByteSize = d3dUtil::CalcConstantBufferByteSize(sizeof(SkinnedConstants));

	auto objectCB = _CurrFrameResource->ObjectCB->Resource();
	auto matCB = _CurrFrameResource->MaterialCB->Resource();
	auto skinnedCB = _CurrFrameResource->SkinnedCB->Resource();

	for (size_t i = 0; i < ritems.size(); ++i)
	{
		auto ri = ritems[i];

		if (!_Wheel->_visible && _Wheel->_WheelRItem == ri)
			continue;

		cmdList->IASetVertexBuffers(0, 1, &ri->Geo->VertexBufferView());
		cmdList->IASetIndexBuffer(&ri->Geo->IndexBufferView());
		cmdList->IASetPrimitiveTopology(ri->PrimitiveType);

		CD3DX12_GPU_DESCRIPTOR_HANDLE tex(_SrvDescriptorHeap->GetGPUDescriptorHandleForHeapStart());
		tex.Offset(ri->Mat->DiffuseSrvHeapIndex, _CbvSrvDescriptorSize);

		D3D12_GPU_VIRTUAL_ADDRESS objCBAddress = objectCB->GetGPUVirtualAddress() + ri->ObjCBIndex * objCBByteSize;
		D3D12_GPU_VIRTUAL_ADDRESS matCBAddress = matCB->GetGPUVirtualAddress() + ri->Mat->MatCBIndex * matCBByteSize;

		cmdList->SetGraphicsRootDescriptorTable(0, tex);
		cmdList->SetGraphicsRootConstantBufferView(1, objCBAddress);
		cmdList->SetGraphicsRootConstantBufferView(3, matCBAddress);

		if (ri->SkinnedModelInst != nullptr)
		{
			D3D12_GPU_VIRTUAL_ADDRESS skinnedCBAddress = skinnedCB->GetGPUVirtualAddress() + ri->SkinnedCBIndex * skinnedCBByteSize;
			cmdList->SetGraphicsRootConstantBufferView(4, skinnedCBAddress);
		}
		else
		{
			cmdList->SetGraphicsRootConstantBufferView(4, 0);
		}

		cmdList->DrawIndexedInstanced(ri->IndexCount, 1, ri->StartIndexLocation, ri->BaseVertexLocation, 0);
	}
}

void GraphicsWindow::UpdateCamera(const GameTimer& gt)
{
	_EyePos.x = _Radius * sinf(_Phi) * cosf(_Theta);
	_EyePos.z = _Radius * sinf(_Phi) * sinf(_Theta);
	_EyePos.y = _Radius * cosf(_Phi);

	DirectX::XMVECTOR pos = DirectX::XMVectorSet(_EyePos.x, _EyePos.y, _EyePos.z, 1.0f);
	DirectX::XMVECTOR target = DirectX::XMVectorZero();
	DirectX::XMVECTOR up = DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

	DirectX::XMMATRIX view = DirectX::XMMatrixLookAtLH(pos, target, up);
	XMStoreFloat4x4(&_View, view);
}

void GraphicsWindow::UpdateFixedCamera(const GameTimer& gt)
{
	DirectX::XMVECTOR pos = DirectX::XMVectorSet(0.0f, 0.0f, -1.0f, 1.0f);
	DirectX::XMVECTOR target = DirectX::XMVectorZero();
	DirectX::XMVECTOR up = DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

	DirectX::XMMATRIX view = DirectX::XMMatrixLookAtLH(pos, target, up);
	XMStoreFloat4x4(&_FixedView, view);
}

void GraphicsWindow::UpdateObjectCBs(const GameTimer& gt)
{
	auto currObjectCB = _CurrFrameResource->ObjectCB.get();
	for (auto& e : _AllRitems)
	{
		if (e->NumFramesDirty > 0)
		{
			DirectX::XMMATRIX world = XMLoadFloat4x4(&e->World);
			DirectX::XMMATRIX texTransform = XMLoadFloat4x4(&e->TexTransform);

			ObjectConstants objConstants;
			XMStoreFloat4x4(&objConstants.World, XMMatrixTranspose(world));
			XMStoreFloat4x4(&objConstants.TexTransform, XMMatrixTranspose(texTransform));

			currObjectCB->CopyData(e->ObjCBIndex, objConstants);

			e->NumFramesDirty--;
		}
	}
}

void GraphicsWindow::UpdateMaterialCBs(const GameTimer& gt)
{
	auto currMaterialCB = _CurrFrameResource->MaterialCB.get();
	for (auto& e : _Materials)
	{
		Material* mat = e.second.get();
		if (mat->NumFramesDirty > 0)
		{
			DirectX::XMMATRIX matTransform = XMLoadFloat4x4(&mat->MatTransform);

			MaterialConstants matConstants;
			matConstants.DiffuseAlbedo = mat->DiffuseAlbedo;
			matConstants.FresnelR0 = mat->FresnelR0;
			matConstants.Roughness = mat->Roughness;
			XMStoreFloat4x4(&matConstants.MatTransform, XMMatrixTranspose(matTransform));

			currMaterialCB->CopyData(mat->MatCBIndex, matConstants);

			mat->NumFramesDirty--;
		}
	}
}

void GraphicsWindow::UpdateMainPassCB(const GameTimer& gt)
{
	DirectX::XMMATRIX view = XMLoadFloat4x4(&_View);
	DirectX::XMMATRIX proj = XMLoadFloat4x4(&_Proj);
	DirectX::XMMATRIX fixedView = XMLoadFloat4x4(&_FixedView);

	DirectX::XMMATRIX viewProj = XMMatrixMultiply(view, proj);
	DirectX::XMMATRIX invView = XMMatrixInverse(&XMMatrixDeterminant(view), view);
	DirectX::XMMATRIX invProj = XMMatrixInverse(&XMMatrixDeterminant(proj), proj);
	DirectX::XMMATRIX invViewProj = XMMatrixInverse(&XMMatrixDeterminant(viewProj), viewProj);

	XMStoreFloat4x4(&_MainPassCB.View, XMMatrixTranspose(view));
	XMStoreFloat4x4(&_MainPassCB.InvView, XMMatrixTranspose(invView));
	XMStoreFloat4x4(&_MainPassCB.Proj, XMMatrixTranspose(proj));
	XMStoreFloat4x4(&_MainPassCB.InvProj, XMMatrixTranspose(invProj));
	XMStoreFloat4x4(&_MainPassCB.ViewProj, XMMatrixTranspose(viewProj));
	XMStoreFloat4x4(&_MainPassCB.InvViewProj, XMMatrixTranspose(invViewProj));
	XMStoreFloat4x4(&_MainPassCB.FixedView, XMMatrixTranspose(fixedView));
	_MainPassCB.EyePosW = _EyePos;
	_MainPassCB.RenderTargetSize = DirectX::XMFLOAT2((float)_ClientWidth, (float)_ClientHeight);
	_MainPassCB.InvRenderTargetSize = DirectX::XMFLOAT2(1.0f / _ClientWidth, 1.0f / _ClientHeight);
	_MainPassCB.NearZ = 1.0f;
	_MainPassCB.FarZ = 1000.0f;
	_MainPassCB.TotalTime = _game_timer.TotalTime();
	_MainPassCB.DeltaTime = _game_timer.DeltaTime();
	_MainPassCB.AmbientLight = { 0.25f, 0.25f, 0.35f, 1.0f };
	_MainPassCB.Lights[0].Direction = { 0.57735f, -0.57735f, 0.57735f };
	_MainPassCB.Lights[0].Strength = { 0.8f, 0.8f, 0.8f };
	_MainPassCB.Lights[1].Direction = { -0.57735f, -0.57735f, 0.57735f };
	_MainPassCB.Lights[1].Strength = { 0.4f, 0.4f, 0.4f };
	_MainPassCB.Lights[2].Direction = { 0.0f, -0.707f, -0.707f };
	_MainPassCB.Lights[2].Strength = { 0.2f, 0.2f, 0.2f };

	auto currPassCB = _CurrFrameResource->PassCB.get();
	currPassCB->CopyData(0, _MainPassCB);
}

void GraphicsWindow::BuildDescriptorHeaps()
{
	D3D12_DESCRIPTOR_HEAP_DESC srvHeapDesc = {};

	srvHeapDesc.NumDescriptors = 27;

	srvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	srvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	ThrowIfFailed(_d3dDevice->CreateDescriptorHeap(&srvHeapDesc, IID_PPV_ARGS(&_SrvDescriptorHeap)));

	CD3DX12_CPU_DESCRIPTOR_HANDLE hDescriptor(_SrvDescriptorHeap->GetCPUDescriptorHandleForHeapStart());

	auto skyTex = _Textures["skyTex"]->Resource;

	std::vector<Microsoft::WRL::ComPtr<ID3D12Resource>> tex2DList =
	{ 
		_Textures["upTex"]->Resource,
		_Textures["downTex"]->Resource,
		_Textures["leftTex"]->Resource,
		_Textures["rightTex"]->Resource,
		_Textures["zoominTex"]->Resource,
		_Textures["zoomoutTex"]->Resource,
		_Textures["new_gameTex"]->Resource,
	
		_Textures["groundTex"]->Resource,
		_Textures["diskRoadTex"]->Resource,
		_Textures["diskP1Tex"]->Resource,
		_Textures["diskP2Tex"]->Resource,
		_Textures["ringTex"]->Resource,
		
		_Textures["dice1Tex"]->Resource,
		_Textures["dice2Tex"]->Resource,
		_Textures["dice3Tex"]->Resource,
		_Textures["dice4Tex"]->Resource,
		_Textures["dice5Tex"]->Resource,
		_Textures["dice6Tex"]->Resource,
		
		_Textures["winTex"]->Resource,
		_Textures["lostTex"]->Resource,
	
		_Textures["monkeyHTex"]->Resource,
		_Textures["monkeyBTex"]->Resource,
		_Textures["monkeyETex"]->Resource,
		_Textures["birdTex"]->Resource,

		_Textures["logoTex"]->Resource,
	
		_Textures["fountainTex"]->Resource
	};

	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.ResourceMinLODClamp = 0.0f;

	srvDesc.Format = skyTex->GetDesc().Format;
	srvDesc.Texture2D.MipLevels = skyTex->GetDesc().MipLevels;
	_d3dDevice->CreateShaderResourceView(skyTex.Get(), &srvDesc, hDescriptor);

	// next descriptor
	hDescriptor.Offset(1, _CbvSrvUavDescriptorSize);

	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	for (UINT i = 0; i < (UINT)tex2DList.size(); ++i)
	{
		srvDesc.Format = tex2DList[i]->GetDesc().Format;
		srvDesc.Texture2D.MipLevels = tex2DList[i]->GetDesc().MipLevels;
		_d3dDevice->CreateShaderResourceView(tex2DList[i].Get(), &srvDesc, hDescriptor);

		// next descriptor
		hDescriptor.Offset(1, _CbvSrvUavDescriptorSize);
	}
}

void GraphicsWindow::BuildMaterials()
{
	auto sky0 = std::make_unique<Material>();
	sky0->Name = "sky0";
	sky0->MatCBIndex = 0;
	sky0->DiffuseSrvHeapIndex = 0;
	sky0->DiffuseAlbedo = DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	sky0->FresnelR0 = DirectX::XMFLOAT3(0.02f, 0.02f, 0.02f);
	sky0->Roughness = 0.3f;

	auto up0 = std::make_unique<Material>();
	up0->Name = "up0";
	up0->MatCBIndex = 1;
	up0->DiffuseSrvHeapIndex = 1;
	up0->DiffuseAlbedo = DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	up0->FresnelR0 = DirectX::XMFLOAT3(0.02f, 0.02f, 0.02f);
	up0->Roughness = 0.3f;

	auto down0 = std::make_unique<Material>();
	down0->Name = "down0";
	down0->MatCBIndex = 2;
	down0->DiffuseSrvHeapIndex = 2;
	down0->DiffuseAlbedo = DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	down0->FresnelR0 = DirectX::XMFLOAT3(0.02f, 0.02f, 0.02f);
	down0->Roughness = 0.3f;

	auto left0 = std::make_unique<Material>();
	left0->Name = "left0";
	left0->MatCBIndex = 3;
	left0->DiffuseSrvHeapIndex = 3;
	left0->DiffuseAlbedo = DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	left0->FresnelR0 = DirectX::XMFLOAT3(0.02f, 0.02f, 0.02f);
	left0->Roughness = 0.3f;

	auto right0 = std::make_unique<Material>();
	right0->Name = "right0";
	right0->MatCBIndex = 4;
	right0->DiffuseSrvHeapIndex = 4;
	right0->DiffuseAlbedo = DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	right0->FresnelR0 = DirectX::XMFLOAT3(0.02f, 0.02f, 0.02f);
	right0->Roughness = 0.3f;

	auto zoomin0 = std::make_unique<Material>();
	zoomin0->Name = "zoomin0";
	zoomin0->MatCBIndex = 5;
	zoomin0->DiffuseSrvHeapIndex = 5;
	zoomin0->DiffuseAlbedo = DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	zoomin0->FresnelR0 = DirectX::XMFLOAT3(0.02f, 0.02f, 0.02f);
	zoomin0->Roughness = 0.3f;

	auto zoomout0 = std::make_unique<Material>();
	zoomout0->Name = "zoomout0";
	zoomout0->MatCBIndex = 6;
	zoomout0->DiffuseSrvHeapIndex = 6;
	zoomout0->DiffuseAlbedo = DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	zoomout0->FresnelR0 = DirectX::XMFLOAT3(0.02f, 0.02f, 0.02f);
	zoomout0->Roughness = 0.3f;

	auto new_game0 = std::make_unique<Material>();
	new_game0->Name = "new_game0";
	new_game0->MatCBIndex = 7;
	new_game0->DiffuseSrvHeapIndex = 7;
	new_game0->DiffuseAlbedo = DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	new_game0->FresnelR0 = DirectX::XMFLOAT3(0.02f, 0.02f, 0.02f);
	new_game0->Roughness = 0.3f;

	auto ground0 = std::make_unique<Material>();
	ground0->Name = "ground0";
	ground0->MatCBIndex = 8;
	ground0->DiffuseSrvHeapIndex = 8;
	ground0->DiffuseAlbedo = DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	ground0->FresnelR0 = DirectX::XMFLOAT3(0.02f, 0.02f, 0.02f);
	ground0->Roughness = 0.3f;

	auto diskRoad0 = std::make_unique<Material>();
	diskRoad0->Name = "diskRoad0";
	diskRoad0->MatCBIndex = 9;
	diskRoad0->DiffuseSrvHeapIndex = 9;
	diskRoad0->DiffuseAlbedo = DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	diskRoad0->FresnelR0 = DirectX::XMFLOAT3(0.02f, 0.02f, 0.02f);
	diskRoad0->Roughness = 0.3f;

	auto diskP10 = std::make_unique<Material>();
	diskP10->Name = "diskP10";
	diskP10->MatCBIndex = 10;
	diskP10->DiffuseSrvHeapIndex = 10;
	diskP10->DiffuseAlbedo = DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	diskP10->FresnelR0 = DirectX::XMFLOAT3(0.02f, 0.02f, 0.02f);
	diskP10->Roughness = 0.1f;

	auto diskP20 = std::make_unique<Material>();
	diskP20->Name = "diskP20";
	diskP20->MatCBIndex = 11;
	diskP20->DiffuseSrvHeapIndex = 11;
	diskP20->DiffuseAlbedo = DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	diskP20->FresnelR0 = DirectX::XMFLOAT3(0.05f, 0.05f, 0.05f);
	diskP20->Roughness = 0.3f;

	auto ring0 = std::make_unique<Material>();
	ring0->Name = "ring0";
	ring0->MatCBIndex = 12;
	ring0->DiffuseSrvHeapIndex = 12;
	ring0->DiffuseAlbedo = DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	ring0->FresnelR0 = DirectX::XMFLOAT3(0.02f, 0.02f, 0.02f);
	ring0->Roughness = 0.3f;

	auto dice10 = std::make_unique<Material>();
	dice10->Name = "dice10";
	dice10->MatCBIndex = 13;
	dice10->DiffuseSrvHeapIndex = 13;
	dice10->DiffuseAlbedo = DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	dice10->FresnelR0 = DirectX::XMFLOAT3(0.02f, 0.02f, 0.02f);
	dice10->Roughness = 0.3f;

	auto dice20 = std::make_unique<Material>();
	dice20->Name = "dice20";
	dice20->MatCBIndex = 14;
	dice20->DiffuseSrvHeapIndex = 14;
	dice20->DiffuseAlbedo = DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	dice20->FresnelR0 = DirectX::XMFLOAT3(0.02f, 0.02f, 0.02f);
	dice20->Roughness = 0.3f;

	auto dice30 = std::make_unique<Material>();
	dice30->Name = "dice30";
	dice30->MatCBIndex = 15;
	dice30->DiffuseSrvHeapIndex = 15;
	dice30->DiffuseAlbedo = DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	dice30->FresnelR0 = DirectX::XMFLOAT3(0.02f, 0.02f, 0.02f);
	dice30->Roughness = 0.3f;

	auto dice40 = std::make_unique<Material>();
	dice40->Name = "dice40";
	dice40->MatCBIndex = 16;
	dice40->DiffuseSrvHeapIndex = 16;
	dice40->DiffuseAlbedo = DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	dice40->FresnelR0 = DirectX::XMFLOAT3(0.02f, 0.02f, 0.02f);
	dice40->Roughness = 0.3f;

	auto dice50 = std::make_unique<Material>();
	dice50->Name = "dice50";
	dice50->MatCBIndex = 17;
	dice50->DiffuseSrvHeapIndex = 17;
	dice50->DiffuseAlbedo = DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	dice50->FresnelR0 = DirectX::XMFLOAT3(0.02f, 0.02f, 0.02f);
	dice50->Roughness = 0.3f;

	auto dice60 = std::make_unique<Material>();
	dice60->Name = "dice60";
	dice60->MatCBIndex = 18;
	dice60->DiffuseSrvHeapIndex = 18;
	dice60->DiffuseAlbedo = DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	dice60->FresnelR0 = DirectX::XMFLOAT3(0.02f, 0.02f, 0.02f);
	dice60->Roughness = 0.3f;

	auto win0 = std::make_unique<Material>();
	win0->Name = "win0";
	win0->MatCBIndex = 19;
	win0->DiffuseSrvHeapIndex = 19;
	win0->DiffuseAlbedo = DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	win0->FresnelR0 = DirectX::XMFLOAT3(0.02f, 0.02f, 0.02f);
	win0->Roughness = 0.3f;

	auto lost0 = std::make_unique<Material>();
	lost0->Name = "lost0";
	lost0->MatCBIndex = 20;
	lost0->DiffuseSrvHeapIndex = 20;
	lost0->DiffuseAlbedo = DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	lost0->FresnelR0 = DirectX::XMFLOAT3(0.02f, 0.02f, 0.02f);
	lost0->Roughness = 0.3f;

	auto monkeyH0 = std::make_unique<Material>();
	monkeyH0->Name = "monkeyH0";
	monkeyH0->MatCBIndex = 21;
	monkeyH0->DiffuseSrvHeapIndex = 21;
	monkeyH0->DiffuseAlbedo = DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	monkeyH0->FresnelR0 = DirectX::XMFLOAT3(0.02f, 0.02f, 0.02f);
	monkeyH0->Roughness = 0.3f;

	auto monkeyB0 = std::make_unique<Material>();
	monkeyB0->Name = "monkeyB0";
	monkeyB0->MatCBIndex = 22;
	monkeyB0->DiffuseSrvHeapIndex = 22;
	monkeyB0->DiffuseAlbedo = DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	monkeyB0->FresnelR0 = DirectX::XMFLOAT3(0.02f, 0.02f, 0.02f);
	monkeyB0->Roughness = 0.3f;

	auto monkeyE0 = std::make_unique<Material>();
	monkeyE0->Name = "monkeyE0";
	monkeyE0->MatCBIndex = 23;
	monkeyE0->DiffuseSrvHeapIndex = 23;
	monkeyE0->DiffuseAlbedo = DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	monkeyE0->FresnelR0 = DirectX::XMFLOAT3(0.02f, 0.02f, 0.02f);
	monkeyE0->Roughness = 0.3f;

	auto bird0 = std::make_unique<Material>();
	bird0->Name = "bird0";
	bird0->MatCBIndex = 24;
	bird0->DiffuseSrvHeapIndex = 24;
	bird0->DiffuseAlbedo = DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	bird0->FresnelR0 = DirectX::XMFLOAT3(0.02f, 0.02f, 0.02f);
	bird0->Roughness = 0.3f;
	
	auto logo0 = std::make_unique<Material>();
	logo0->Name = "logo0";
	logo0->MatCBIndex = 25;
	logo0->DiffuseSrvHeapIndex = 25;
	logo0->DiffuseAlbedo = DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	logo0->FresnelR0 = DirectX::XMFLOAT3(0.02f, 0.02f, 0.02f);
	logo0->Roughness = 0.3f;

	auto fountain0 = std::make_unique<Material>();
	fountain0->Name = "fountain0";
	fountain0->MatCBIndex = 26;
	fountain0->DiffuseSrvHeapIndex = 26;
	fountain0->DiffuseAlbedo = DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	fountain0->FresnelR0 = DirectX::XMFLOAT3(0.02f, 0.02f, 0.02f);
	fountain0->Roughness = 0.3f;

	_Materials["sky0"] = std::move(sky0); 
	
	_Materials["up0"] = std::move(up0);
	_Materials["down0"] = std::move(down0);
	_Materials["left0"] = std::move(left0);
	_Materials["right0"] = std::move(right0);
	_Materials["zoomin0"] = std::move(zoomin0);
	_Materials["zoomout0"] = std::move(zoomout0);
	_Materials["new_game0"] = std::move(new_game0);

	_Materials["ground0"] = std::move(ground0);
	_Materials["diskRoad0"] = std::move(diskRoad0);
	_Materials["diskP10"] = std::move(diskP10);
	_Materials["diskP20"] = std::move(diskP20);
	_Materials["ring0"] = std::move(ring0);
	
	_Materials["dice10"] = std::move(dice10);
	_Materials["dice20"] = std::move(dice20);
	_Materials["dice30"] = std::move(dice30);
	_Materials["dice40"] = std::move(dice40);
	_Materials["dice50"] = std::move(dice50);
	_Materials["dice60"] = std::move(dice60);

	_Materials["win0"] = std::move(win0);
	_Materials["lost0"] = std::move(lost0);
	
	_Materials["monkeyH0"] = std::move(monkeyH0);
	_Materials["monkeyB0"] = std::move(monkeyB0);
	_Materials["monkeyE0"] = std::move(monkeyE0);
	_Materials["bird0"] = std::move(bird0);

	_Materials["logo0"] = std::move(logo0);

	_Materials["fountain0"] = std::move(fountain0);
}

std::array<const CD3DX12_STATIC_SAMPLER_DESC, 6> GraphicsWindow::GetStaticSamplers()
{
	const CD3DX12_STATIC_SAMPLER_DESC pointWrap(
		0,
		D3D12_FILTER_MIN_MAG_MIP_POINT,
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,
		D3D12_TEXTURE_ADDRESS_MODE_WRAP);

	const CD3DX12_STATIC_SAMPLER_DESC pointClamp(
		1,
		D3D12_FILTER_MIN_MAG_MIP_POINT,
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP);

	const CD3DX12_STATIC_SAMPLER_DESC linearWrap(
		2,
		D3D12_FILTER_MIN_MAG_MIP_LINEAR,
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,
		D3D12_TEXTURE_ADDRESS_MODE_WRAP);

	const CD3DX12_STATIC_SAMPLER_DESC linearClamp(
		3,
		D3D12_FILTER_MIN_MAG_MIP_LINEAR,
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP);

	const CD3DX12_STATIC_SAMPLER_DESC anisotropicWrap(
		4,
		D3D12_FILTER_ANISOTROPIC,
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,
		0.0f,
		8);

	const CD3DX12_STATIC_SAMPLER_DESC anisotropicClamp(
		5,
		D3D12_FILTER_ANISOTROPIC,
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,
		0.0f,
		8);

	return {
		pointWrap, pointClamp,
		linearWrap, linearClamp,
		anisotropicWrap, anisotropicClamp };
}

void GraphicsWindow::Pick(int sx, int sy)
{
	for (auto ri : _RitemLayer[(int)RenderLayer::Opaque])
	{
		if (ri == _Dice->_dice_cube_p1._dice1 ||
			ri == _Dice->_dice_cube_p1._dice2 || 
			ri == _Dice->_dice_cube_p1._dice3 || 
			ri == _Dice->_dice_cube_p1._dice4 || 
			ri == _Dice->_dice_cube_p1._dice5 || 
			ri == _Dice->_dice_cube_p1._dice6)
		{
			auto geo = ri->Geo;

			XMFLOAT4X4 P = _Proj;

			float vx = (+2.0f * sx / _ClientWidth - 1.0f) / P(0, 0);
			float vy = (-2.0f * sy / _ClientHeight + 1.0f) / P(1, 1);

			XMVECTOR rayOrigin = XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);
			XMVECTOR rayDir = XMVectorSet(vx, vy, 1.0f, 0.0f);

			XMMATRIX V = XMLoadFloat4x4(&_View);
			XMMATRIX invView = XMMatrixInverse(&XMMatrixDeterminant(V), V);

			XMMATRIX W = XMLoadFloat4x4(&ri->World);
			XMMATRIX invWorld = XMMatrixInverse(&XMMatrixDeterminant(W), W);

			XMMATRIX toLocal = XMMatrixMultiply(invView, invWorld);

			rayOrigin = XMVector3TransformCoord(rayOrigin, toLocal);
			rayDir = XMVector3TransformNormal(rayDir, toLocal);

			rayDir = XMVector3Normalize(rayDir);

			float tmin = 0.0f;
			if (ri->Bounds.Intersects(rayOrigin, rayDir, tmin))
			{
				if (ri == _Dice->_dice_cube_p1._dice1 ||
					ri == _Dice->_dice_cube_p1._dice2 ||
					ri == _Dice->_dice_cube_p1._dice3 ||
					ri == _Dice->_dice_cube_p1._dice4 ||
					ri == _Dice->_dice_cube_p1._dice5 ||
					ri == _Dice->_dice_cube_p1._dice6)
				{
					FireProcessP1Msg(EV_DICE1_CLICKED);
					return;
				}
			}
		}
		
	}
}

void GraphicsWindow::PickMonkey(int sx, int sy)
{
	std::vector<UINT> selStone;

	for (auto ri : _RitemLayer[(int)RenderLayer::Skinned])
	{
		bool p1Stone = false;

		for (UINT ix = 0; ix < 4 && !p1Stone; ++ix)
		{
			for (UINT meshIdx = 0; meshIdx < _Monkey->_stones_p1[ix]._rItem.size(); ++meshIdx)
			{
				if (ri == _Monkey->_stones_p1[ix]._rItem[meshIdx])
				{
					p1Stone = true;
				}
			}
		}

		if (p1Stone)
		{
			auto geo = ri->Geo;

			XMFLOAT4X4 P = _Proj;

			float vx = (+2.0f * sx / _ClientWidth - 1.0f) / P(0, 0);
			float vy = (-2.0f * sy / _ClientHeight + 1.0f) / P(1, 1);

			XMVECTOR rayOrigin = XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);
			XMVECTOR rayDir = XMVectorSet(vx, vy, 1.0f, 0.0f);

			XMMATRIX V = XMLoadFloat4x4(&_View);
			XMMATRIX invView = XMMatrixInverse(&XMMatrixDeterminant(V), V);

			XMMATRIX W = XMLoadFloat4x4(&ri->World);
			XMMATRIX invWorld = XMMatrixInverse(&XMMatrixDeterminant(W), W);

			XMMATRIX toLocal = XMMatrixMultiply(invView, invWorld);

			rayOrigin = XMVector3TransformCoord(rayOrigin, toLocal);
			rayDir = XMVector3TransformNormal(rayDir, toLocal);

			rayDir = XMVector3Normalize(rayDir);

			float tmin = 0.0f;
			if (ri->Bounds.Intersects(rayOrigin, rayDir, tmin))
			{
				bool p1Stone = false;
				for (UINT ix = 0; ix < 4 && !p1Stone; ++ix)
				{
					for (UINT meshIdx = 0; meshIdx < _Monkey->_stones_p1[ix]._rItem.size(); ++meshIdx)
					{
						if (ri == _Monkey->_stones_p1[ix]._rItem[meshIdx])
						{
							p1Stone = true;
							selStone.push_back(ix);
							break;
						}
					}
				}
			}
		}
	}

	if (selStone.size())
	{
		UINT selIdx = selStone[0];

		for (UINT ix = 0; ix < selStone.size(); ++ix)
		{
			if (selStone[ix] != selIdx && _Monkey->_stones_p1[ix]._z < _Monkey->_stones_p1[selIdx]._z)
			{
				selIdx = selStone[ix];
				break;
			}
		}

		FireProcessP1Msg(EV_STONE_P1_SELECTED, (LPARAM)selIdx);
	}
}

void GraphicsWindow::PickFixed(int sx, int sy)
{
	for (auto ri : _RitemLayer[(int)RenderLayer::Fixed])
	{
		auto geo = ri->Geo;

		XMFLOAT4X4 P = _Proj;

		float vx = (+2.0f * sx / _ClientWidth - 1.0f) / P(0, 0);
		float vy = (-2.0f * sy / _ClientHeight + 1.0f) / P(1, 1);

		XMVECTOR rayOrigin = XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);
		XMVECTOR rayDir = XMVectorSet(vx, vy, 1.0f, 0.0f);

		XMMATRIX V = XMLoadFloat4x4(&_FixedView);
		XMMATRIX invView = XMMatrixInverse(&XMMatrixDeterminant(V), V);

		XMMATRIX W = XMLoadFloat4x4(&ri->World);
		XMMATRIX invWorld = XMMatrixInverse(&XMMatrixDeterminant(W), W);

		XMMATRIX toLocal = XMMatrixMultiply(invView, invWorld);

		rayOrigin = XMVector3TransformCoord(rayOrigin, toLocal);
		rayDir = XMVector3TransformNormal(rayDir, toLocal);

		rayDir = XMVector3Normalize(rayDir);

		float tmin = 0.0f;
		if (ri->Bounds.Intersects(rayOrigin, rayDir, tmin))
		{
			if (ri == _Fixed->_newButton)
			{
				OnStartNewGame();
			}
			else if (ri == _Fixed->_upButton)
			{
				OnTimer_Up();
				SetTimer(Window(), IDT_TIMER_UP, TIMER_PERIOD, NULL);
			}
			else if (ri == _Fixed->_downButton)
			{
				OnTimer_Down(); 
				SetTimer(Window(), IDT_TIMER_DOWN, TIMER_PERIOD, NULL);
			}
			else if (ri == _Fixed->_leftButton)
			{
				OnTimer_Left(); 
				SetTimer(Window(), IDT_TIMER_LEFT, TIMER_PERIOD, NULL);
			}
			else if (ri == _Fixed->_rightButton)
			{
				OnTimer_Right(); 
				SetTimer(Window(), IDT_TIMER_RIGHT, TIMER_PERIOD, NULL);
			}
			else if (ri == _Fixed->_zoominButton)
			{
				OnTimer_Zoomin(); 
				SetTimer(Window(), IDT_TIMER_IN, TIMER_PERIOD, NULL);
			}
			else if (ri == _Fixed->_zoomoutButton)
			{
				OnTimer_Zoomout(); 
				SetTimer(Window(), IDT_TIMER_OUT, TIMER_PERIOD, NULL);
			}
		}
	}
}

void GraphicsWindow::MoveStoneP1(StoneP1* s, int x0, int z0)
{
	s->_x = x0;
	s->_z = z0;
	s->_need_update = true;
}

void GraphicsWindow::MoveStoneP2(StoneP2* s, int x0, int z0)
{
	s->_x = x0;
	s->_z = z0;
	s->_need_update = true;
}

void GraphicsWindow::GRollDiceP1(int dice_num)
{
	_Dice->_dice_cube_p1._need_update = true;

	_Dice->_dice_cube_p1._ax = 0;
	_Dice->_dice_cube_p1._ay = 0;
	_Dice->_dice_cube_p1._az = 0;

	if (dice_num == 1)
	{
		_Dice->_dice_cube_p1._axMax = DirectX::XM_PIDIV2;
		_Dice->_dice_cube_p1._ayMax = DirectX::XM_2PI;
		_Dice->_dice_cube_p1._azMax = DirectX::XM_2PI;
	}
	else if (dice_num == 2)
	{
		_Dice->_dice_cube_p1._axMax = DirectX::XM_2PI;
		_Dice->_dice_cube_p1._ayMax = DirectX::XM_2PI;
		_Dice->_dice_cube_p1._azMax = 3 * DirectX::XM_PIDIV2;
	}
	else if (dice_num == 3)
	{
		_Dice->_dice_cube_p1._axMax = DirectX::XM_2PI;
		_Dice->_dice_cube_p1._ayMax = DirectX::XM_2PI;
		_Dice->_dice_cube_p1._azMax = DirectX::XM_2PI;
	}
	else if (dice_num == 4)
	{
		_Dice->_dice_cube_p1._axMax = DirectX::XM_PI;
		_Dice->_dice_cube_p1._ayMax = DirectX::XM_2PI;
		_Dice->_dice_cube_p1._azMax = DirectX::XM_2PI;
	}
	else if (dice_num == 5)
	{
		_Dice->_dice_cube_p1._axMax = DirectX::XM_2PI;
		_Dice->_dice_cube_p1._ayMax = DirectX::XM_2PI;
		_Dice->_dice_cube_p1._azMax = DirectX::XM_PIDIV2;
	}
	else if (dice_num == 6)
	{
		_Dice->_dice_cube_p1._axMax = 3 * DirectX::XM_PIDIV2;
		_Dice->_dice_cube_p1._ayMax = DirectX::XM_2PI;
		_Dice->_dice_cube_p1._azMax = DirectX::XM_2PI;
	}
}

void GraphicsWindow::GRollDiceP2(int dice_num)
{
	_Dice->_dice_cube_p2._need_update = true;

	_Dice->_dice_cube_p2._ax = 0;
	_Dice->_dice_cube_p2._ay = 0;
	_Dice->_dice_cube_p2._az = 0;

	_Dice->_dice_cube_p2._at = 0;
	_Dice->_dice_cube_p2._atMax = 8.0f;

	if (dice_num == 1)
	{
		_Dice->_dice_cube_p2._axMax = DirectX::XM_PIDIV2;
		_Dice->_dice_cube_p2._ayMax = DirectX::XM_2PI;
		_Dice->_dice_cube_p2._azMax = DirectX::XM_2PI;
	}
	else if (dice_num == 2)
	{
		_Dice->_dice_cube_p2._axMax = DirectX::XM_2PI;
		_Dice->_dice_cube_p2._ayMax = DirectX::XM_2PI;
		_Dice->_dice_cube_p2._azMax = 3 * DirectX::XM_PIDIV2;
	}
	else if (dice_num == 3)
	{
		_Dice->_dice_cube_p2._axMax = DirectX::XM_2PI;
		_Dice->_dice_cube_p2._ayMax = DirectX::XM_2PI;
		_Dice->_dice_cube_p2._azMax = DirectX::XM_2PI;
	}
	else if (dice_num == 4)
	{
		_Dice->_dice_cube_p2._axMax = DirectX::XM_PI;
		_Dice->_dice_cube_p2._ayMax = DirectX::XM_2PI;
		_Dice->_dice_cube_p2._azMax = DirectX::XM_2PI;
	}
	else if (dice_num == 5)
	{
		_Dice->_dice_cube_p2._axMax = DirectX::XM_2PI;
		_Dice->_dice_cube_p2._ayMax = DirectX::XM_2PI;
		_Dice->_dice_cube_p2._azMax = DirectX::XM_PIDIV2;
	}
	else if (dice_num == 6)
	{
		_Dice->_dice_cube_p2._axMax = 3 * DirectX::XM_PIDIV2;
		_Dice->_dice_cube_p2._ayMax = DirectX::XM_2PI;
		_Dice->_dice_cube_p2._azMax = DirectX::XM_2PI;
	}
}

LRESULT GraphicsWindow::OnTimer_Up()
{
	_Phi += 0.008f;
	_Phi = MathHelper::Clamp(_Phi, 0.1f, DirectX::XM_PIDIV2 - 0.1f);
	return 0;
}

LRESULT GraphicsWindow::OnTimer_Down()
{
	_Phi -= 0.008f;
	_Phi = MathHelper::Clamp(_Phi, 0.1f, DirectX::XM_PIDIV2 - 0.1f);
	return 0;
}

LRESULT GraphicsWindow::OnTimer_Left()
{
	_Theta += 0.008f;
	return 0;
}

LRESULT GraphicsWindow::OnTimer_Right()
{
	_Theta -= 0.008f;
	return 0;
}

LRESULT GraphicsWindow::OnTimer_Zoomin()
{
	_Radius -= 1.0f;

	_Radius = MathHelper::Clamp(_Radius, 7.0f, 15.0f);
	return 0;
}

LRESULT GraphicsWindow::OnTimer_Zoomout()
{
	_Radius += 1.0f;

	_Radius = MathHelper::Clamp(_Radius, 7.0f, 15.0f);
	return 0;
}

LRESULT GraphicsWindow::OnStartNewGame()
{
	return 0;
}
