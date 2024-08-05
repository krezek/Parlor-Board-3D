#include "pch.h"
#include "platform.h"

#include <d3dUtil.h>
#include <GeometryGenerator.h>
#include <FrameResource.h>
#include <RenderItem.h>
#include <Dice.h>

using namespace DirectX;

extern int g_ObjCBIndex;

void Dice::BuildGeometry(ID3D12Device* devicePtr,
	ID3D12GraphicsCommandList* commandListPtr,
	std::unordered_map<std::string, std::unique_ptr<MeshGeometry>>& geometries)
{
	GeometryGenerator geoGen;
	GeometryGenerator::MeshData dice = geoGen.CreateQuad(-0.5f, 0.5f, 1.0f, 1.0f, 0.0f);

	size_t totalSize = dice.Vertices.size();
	std::vector<Vertex> vertices(totalSize);

	UINT k = 0;
	for (size_t i = 0; i < dice.Vertices.size(); ++i, ++k)
	{
		auto& p = dice.Vertices[i].Position;
		vertices[k].Pos = p;
		vertices[k].Normal = dice.Vertices[i].Normal;
		vertices[k].TexC = dice.Vertices[i].TexC;
	}

	BoundingBox dice_bounds;
	BoundingBox::CreateFromPoints(dice_bounds, dice.Vertices.size(),
		&vertices[0].Pos, sizeof(Vertex));

	const UINT vbByteSize = (UINT)vertices.size() * sizeof(Vertex);

	std::vector<std::uint16_t> indices;
	indices.insert(indices.end(), std::begin(dice.GetIndices16()), std::end(dice.GetIndices16()));
	
	const UINT ibByteSize = (UINT)indices.size() * sizeof(std::uint16_t);

	auto geo = std::make_unique<MeshGeometry>();
	geo->Name = "diceGeo";

	ThrowIfFailed(D3DCreateBlob(vbByteSize, &geo->VertexBufferCPU));
	CopyMemory(geo->VertexBufferCPU->GetBufferPointer(), vertices.data(), vbByteSize);

	ThrowIfFailed(D3DCreateBlob(ibByteSize, &geo->IndexBufferCPU));
	CopyMemory(geo->IndexBufferCPU->GetBufferPointer(), indices.data(), ibByteSize);

	geo->VertexBufferGPU = d3dUtil::CreateDefaultBuffer(devicePtr,
		commandListPtr, vertices.data(), vbByteSize, geo->VertexBufferUploader);

	geo->IndexBufferGPU = d3dUtil::CreateDefaultBuffer(devicePtr,
		commandListPtr, indices.data(), ibByteSize, geo->IndexBufferUploader);

	geo->VertexByteStride = sizeof(Vertex);
	geo->VertexBufferByteSize = vbByteSize;
	geo->IndexFormat = DXGI_FORMAT_R16_UINT;
	geo->IndexBufferByteSize = ibByteSize;

	SubmeshGeometry diceSubmesh;
	diceSubmesh.IndexCount = (UINT)dice.Indices32.size();
	diceSubmesh.StartIndexLocation = 0;
	diceSubmesh.BaseVertexLocation = 0;
	diceSubmesh.Bounds = dice_bounds;

	geo->DrawArgs["dice"] = diceSubmesh;
	
	geometries[geo->Name] = std::move(geo);
}

void Dice::BuildRenderItems_p1(std::unordered_map<std::string, std::unique_ptr<MeshGeometry>>& geometries,
	std::unordered_map<std::string, std::unique_ptr<Material>>& materials,
	std::vector<std::unique_ptr<RenderItem>>& allRitems,
	std::vector<RenderItem*>& opaqueRenderItems)
{
	DirectX::XMMATRIX roMat = DirectX::XMMatrixRotationX(_dice_cube_p1._ax) *
		DirectX::XMMatrixRotationY(_dice_cube_p1._ay) *
		DirectX::XMMatrixRotationZ(_dice_cube_p1._az);

	auto diceRitem1 = std::make_unique<RenderItem>();
	_dice_cube_p1._dice1 = diceRitem1.get();
	XMStoreFloat4x4(&diceRitem1->World, DirectX::XMMatrixTranslation(0.0f, 0.0f, -0.5f) * roMat *
		DirectX::XMMatrixTranslation(-4.0f, 0.8f, -4.0f));
	XMStoreFloat4x4(&diceRitem1->TexTransform, DirectX::XMMatrixScaling(1.0f, 1.0f, 1.0f));
	diceRitem1->ObjCBIndex = g_ObjCBIndex++;
	diceRitem1->Geo = geometries["diceGeo"].get();
	diceRitem1->Mat = materials["dice10"].get();
	diceRitem1->PrimitiveType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	diceRitem1->IndexCount = diceRitem1->Geo->DrawArgs["dice"].IndexCount;
	diceRitem1->StartIndexLocation = diceRitem1->Geo->DrawArgs["dice"].StartIndexLocation;
	diceRitem1->BaseVertexLocation = diceRitem1->Geo->DrawArgs["dice"].BaseVertexLocation;
	diceRitem1->Bounds = diceRitem1->Geo->DrawArgs["dice"].Bounds;

	opaqueRenderItems.push_back(diceRitem1.get());
	allRitems.push_back(std::move(diceRitem1));

	auto diceRitem6 = std::make_unique<RenderItem>();
	_dice_cube_p1._dice6 = diceRitem6.get();
	XMStoreFloat4x4(&diceRitem6->World, DirectX::XMMatrixTranslation(0.0f, 0.0f, -0.5f) *
		DirectX::XMMatrixRotationY(DirectX::XM_PI) * roMat *
		DirectX::XMMatrixTranslation(-4.0f, 0.8f, -4.0f));
	XMStoreFloat4x4(&diceRitem6->TexTransform, DirectX::XMMatrixScaling(1.0f, 1.0f, 1.0f));
	diceRitem6->ObjCBIndex = g_ObjCBIndex++;
	diceRitem6->Geo = geometries["diceGeo"].get();
	diceRitem6->Mat = materials["dice60"].get();
	diceRitem6->PrimitiveType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	diceRitem6->IndexCount = diceRitem6->Geo->DrawArgs["dice"].IndexCount;
	diceRitem6->StartIndexLocation = diceRitem6->Geo->DrawArgs["dice"].StartIndexLocation;
	diceRitem6->BaseVertexLocation = diceRitem6->Geo->DrawArgs["dice"].BaseVertexLocation;
	diceRitem6->Bounds = diceRitem6->Geo->DrawArgs["dice"].Bounds;

	opaqueRenderItems.push_back(diceRitem6.get());
	allRitems.push_back(std::move(diceRitem6));

	auto diceRitem5 = std::make_unique<RenderItem>();
	_dice_cube_p1._dice5 = diceRitem5.get();
	XMStoreFloat4x4(&diceRitem5->World, DirectX::XMMatrixRotationY(-DirectX::XM_PIDIV2) *
		DirectX::XMMatrixTranslation(0.5f, 0.0f, 0.0f) * roMat *
		DirectX::XMMatrixTranslation(-4.0f, 0.8f, -4.0f));
	XMStoreFloat4x4(&diceRitem5->TexTransform, DirectX::XMMatrixScaling(1.0f, 1.0f, 1.0f));
	diceRitem5->ObjCBIndex = g_ObjCBIndex++;
	diceRitem5->Geo = geometries["diceGeo"].get();
	diceRitem5->Mat = materials["dice50"].get();
	diceRitem5->PrimitiveType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	diceRitem5->IndexCount = diceRitem5->Geo->DrawArgs["dice"].IndexCount;
	diceRitem5->StartIndexLocation = diceRitem5->Geo->DrawArgs["dice"].StartIndexLocation;
	diceRitem5->BaseVertexLocation = diceRitem5->Geo->DrawArgs["dice"].BaseVertexLocation;
	diceRitem5->Bounds = diceRitem5->Geo->DrawArgs["dice"].Bounds;

	opaqueRenderItems.push_back(diceRitem5.get());
	allRitems.push_back(std::move(diceRitem5));

	auto diceRitem3 = std::make_unique<RenderItem>();
	_dice_cube_p1._dice3 = diceRitem3.get();
	XMStoreFloat4x4(&diceRitem3->World, DirectX::XMMatrixTranslation(0.0f, 0.0f, -0.5f) *
		DirectX::XMMatrixRotationX(DirectX::XM_PIDIV2) * roMat *
		DirectX::XMMatrixTranslation(-4.0f, 0.8f, -4.0f));
	XMStoreFloat4x4(&diceRitem3->TexTransform, DirectX::XMMatrixScaling(1.0f, 1.0f, 1.0f));
	diceRitem3->ObjCBIndex = g_ObjCBIndex++;
	diceRitem3->Geo = geometries["diceGeo"].get();
	diceRitem3->Mat = materials["dice30"].get();
	diceRitem3->PrimitiveType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	diceRitem3->IndexCount = diceRitem3->Geo->DrawArgs["dice"].IndexCount;
	diceRitem3->StartIndexLocation = diceRitem3->Geo->DrawArgs["dice"].StartIndexLocation;
	diceRitem3->BaseVertexLocation = diceRitem3->Geo->DrawArgs["dice"].BaseVertexLocation;
	diceRitem3->Bounds = diceRitem3->Geo->DrawArgs["dice"].Bounds;

	opaqueRenderItems.push_back(diceRitem3.get());
	allRitems.push_back(std::move(diceRitem3));

	auto diceRitem2 = std::make_unique<RenderItem>();
	_dice_cube_p1._dice2 = diceRitem2.get();
	XMStoreFloat4x4(&diceRitem2->World, DirectX::XMMatrixRotationY(DirectX::XM_PIDIV2) *
		DirectX::XMMatrixTranslation(-0.5f, 0.0f, 0.0f) * roMat *
		DirectX::XMMatrixTranslation(-4.0f, 0.8f, -4.0f));
	XMStoreFloat4x4(&diceRitem2->TexTransform, DirectX::XMMatrixScaling(1.0f, 1.0f, 1.0f));
	diceRitem2->ObjCBIndex = g_ObjCBIndex++;
	diceRitem2->Geo = geometries["diceGeo"].get();
	diceRitem2->Mat = materials["dice20"].get();
	diceRitem2->PrimitiveType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	diceRitem2->IndexCount = diceRitem2->Geo->DrawArgs["dice"].IndexCount;
	diceRitem2->StartIndexLocation = diceRitem2->Geo->DrawArgs["dice"].StartIndexLocation;
	diceRitem2->BaseVertexLocation = diceRitem2->Geo->DrawArgs["dice"].BaseVertexLocation;
	diceRitem2->Bounds = diceRitem2->Geo->DrawArgs["dice"].Bounds;

	opaqueRenderItems.push_back(diceRitem2.get());
	allRitems.push_back(std::move(diceRitem2));

	auto diceRitem4 = std::make_unique<RenderItem>();
	_dice_cube_p1._dice4 = diceRitem4.get();
	XMStoreFloat4x4(&diceRitem4->World, DirectX::XMMatrixTranslation(0.0f, 0.0f, -0.5f) *
		DirectX::XMMatrixRotationX(3 * DirectX::XM_PIDIV2) * roMat *
		DirectX::XMMatrixTranslation(-4.0f, 0.8f, -4.0f));
	XMStoreFloat4x4(&diceRitem4->TexTransform, DirectX::XMMatrixScaling(1.0f, 1.0f, 1.0f));
	diceRitem4->ObjCBIndex = g_ObjCBIndex++;
	diceRitem4->Geo = geometries["diceGeo"].get();
	diceRitem4->Mat = materials["dice40"].get();
	diceRitem4->PrimitiveType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	diceRitem4->IndexCount = diceRitem4->Geo->DrawArgs["dice"].IndexCount;
	diceRitem4->StartIndexLocation = diceRitem4->Geo->DrawArgs["dice"].StartIndexLocation;
	diceRitem4->BaseVertexLocation = diceRitem4->Geo->DrawArgs["dice"].BaseVertexLocation;
	diceRitem4->Bounds = diceRitem4->Geo->DrawArgs["dice"].Bounds;

	opaqueRenderItems.push_back(diceRitem4.get());
	allRitems.push_back(std::move(diceRitem4));
}

void Dice::BuildRenderItems_p2(std::unordered_map<std::string, std::unique_ptr<MeshGeometry>>& geometries,
	std::unordered_map<std::string, std::unique_ptr<Material>>& materials,
	std::vector<std::unique_ptr<RenderItem>>& allRitems,
	std::vector<RenderItem*>& opaqueRenderItems)
{
	DirectX::XMMATRIX roMat = DirectX::XMMatrixRotationX(_dice_cube_p2._ax) *
		DirectX::XMMatrixRotationY(_dice_cube_p2._ay) *
		DirectX::XMMatrixRotationZ(_dice_cube_p2._az);

	auto diceRitem1 = std::make_unique<RenderItem>();
	_dice_cube_p2._dice1 = diceRitem1.get();
	XMStoreFloat4x4(&diceRitem1->World, DirectX::XMMatrixTranslation(0.0f, 0.0f, -0.5f) * roMat *
		DirectX::XMMatrixTranslation(4.0f, 0.8f, 4.0f));
	XMStoreFloat4x4(&diceRitem1->TexTransform, DirectX::XMMatrixScaling(1.0f, 1.0f, 1.0f));
	diceRitem1->ObjCBIndex = g_ObjCBIndex++;
	diceRitem1->Geo = geometries["diceGeo"].get();
	diceRitem1->Mat = materials["dice10"].get();
	diceRitem1->PrimitiveType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	diceRitem1->IndexCount = diceRitem1->Geo->DrawArgs["dice"].IndexCount;
	diceRitem1->StartIndexLocation = diceRitem1->Geo->DrawArgs["dice"].StartIndexLocation;
	diceRitem1->BaseVertexLocation = diceRitem1->Geo->DrawArgs["dice"].BaseVertexLocation;
	diceRitem1->Bounds = diceRitem1->Geo->DrawArgs["dice"].Bounds;

	opaqueRenderItems.push_back(diceRitem1.get());
	allRitems.push_back(std::move(diceRitem1));

	auto diceRitem6 = std::make_unique<RenderItem>();
	_dice_cube_p2._dice6 = diceRitem6.get();
	XMStoreFloat4x4(&diceRitem6->World, DirectX::XMMatrixTranslation(0.0f, 0.0f, -0.5f) *
		DirectX::XMMatrixRotationY(DirectX::XM_PI) * roMat *
		DirectX::XMMatrixTranslation(4.0f, 0.8f, 4.0f));
	XMStoreFloat4x4(&diceRitem6->TexTransform, DirectX::XMMatrixScaling(1.0f, 1.0f, 1.0f));
	diceRitem6->ObjCBIndex = g_ObjCBIndex++;
	diceRitem6->Geo = geometries["diceGeo"].get();
	diceRitem6->Mat = materials["dice60"].get();
	diceRitem6->PrimitiveType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	diceRitem6->IndexCount = diceRitem6->Geo->DrawArgs["dice"].IndexCount;
	diceRitem6->StartIndexLocation = diceRitem6->Geo->DrawArgs["dice"].StartIndexLocation;
	diceRitem6->BaseVertexLocation = diceRitem6->Geo->DrawArgs["dice"].BaseVertexLocation;
	diceRitem6->Bounds = diceRitem6->Geo->DrawArgs["dice"].Bounds;

	opaqueRenderItems.push_back(diceRitem6.get());
	allRitems.push_back(std::move(diceRitem6));

	auto diceRitem5 = std::make_unique<RenderItem>();
	_dice_cube_p2._dice5 = diceRitem5.get();
	XMStoreFloat4x4(&diceRitem5->World, DirectX::XMMatrixRotationY(-DirectX::XM_PIDIV2) *
		DirectX::XMMatrixTranslation(0.5f, 0.0f, 0.0f) * roMat *
		DirectX::XMMatrixTranslation(4.0f, 0.8f, 4.0f));
	XMStoreFloat4x4(&diceRitem5->TexTransform, DirectX::XMMatrixScaling(1.0f, 1.0f, 1.0f));
	diceRitem5->ObjCBIndex = g_ObjCBIndex++;
	diceRitem5->Geo = geometries["diceGeo"].get();
	diceRitem5->Mat = materials["dice50"].get();
	diceRitem5->PrimitiveType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	diceRitem5->IndexCount = diceRitem5->Geo->DrawArgs["dice"].IndexCount;
	diceRitem5->StartIndexLocation = diceRitem5->Geo->DrawArgs["dice"].StartIndexLocation;
	diceRitem5->BaseVertexLocation = diceRitem5->Geo->DrawArgs["dice"].BaseVertexLocation;
	diceRitem5->Bounds = diceRitem5->Geo->DrawArgs["dice"].Bounds;

	opaqueRenderItems.push_back(diceRitem5.get());
	allRitems.push_back(std::move(diceRitem5));

	auto diceRitem3 = std::make_unique<RenderItem>();
	_dice_cube_p2._dice3 = diceRitem3.get();
	XMStoreFloat4x4(&diceRitem3->World, DirectX::XMMatrixTranslation(0.0f, 0.0f, -0.5f) *
		DirectX::XMMatrixRotationX(DirectX::XM_PIDIV2) * roMat *
		DirectX::XMMatrixTranslation(4.0f, 0.8f, 4.0f));
	XMStoreFloat4x4(&diceRitem3->TexTransform, DirectX::XMMatrixScaling(1.0f, 1.0f, 1.0f));
	diceRitem3->ObjCBIndex = g_ObjCBIndex++;
	diceRitem3->Geo = geometries["diceGeo"].get();
	diceRitem3->Mat = materials["dice30"].get();
	diceRitem3->PrimitiveType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	diceRitem3->IndexCount = diceRitem3->Geo->DrawArgs["dice"].IndexCount;
	diceRitem3->StartIndexLocation = diceRitem3->Geo->DrawArgs["dice"].StartIndexLocation;
	diceRitem3->BaseVertexLocation = diceRitem3->Geo->DrawArgs["dice"].BaseVertexLocation;
	diceRitem3->Bounds = diceRitem3->Geo->DrawArgs["dice"].Bounds;

	opaqueRenderItems.push_back(diceRitem3.get());
	allRitems.push_back(std::move(diceRitem3));

	auto diceRitem2 = std::make_unique<RenderItem>();
	_dice_cube_p2._dice2 = diceRitem2.get();
	XMStoreFloat4x4(&diceRitem2->World, DirectX::XMMatrixRotationY(DirectX::XM_PIDIV2) *
		DirectX::XMMatrixTranslation(-0.5f, 0.0f, 0.0f) * roMat *
		DirectX::XMMatrixTranslation(4.0f, 0.8f, 4.0f));
	XMStoreFloat4x4(&diceRitem2->TexTransform, DirectX::XMMatrixScaling(1.0f, 1.0f, 1.0f));
	diceRitem2->ObjCBIndex = g_ObjCBIndex++;
	diceRitem2->Geo = geometries["diceGeo"].get();
	diceRitem2->Mat = materials["dice20"].get();
	diceRitem2->PrimitiveType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	diceRitem2->IndexCount = diceRitem2->Geo->DrawArgs["dice"].IndexCount;
	diceRitem2->StartIndexLocation = diceRitem2->Geo->DrawArgs["dice"].StartIndexLocation;
	diceRitem2->BaseVertexLocation = diceRitem2->Geo->DrawArgs["dice"].BaseVertexLocation;
	diceRitem2->Bounds = diceRitem2->Geo->DrawArgs["dice"].Bounds;

	opaqueRenderItems.push_back(diceRitem2.get());
	allRitems.push_back(std::move(diceRitem2));

	auto diceRitem4 = std::make_unique<RenderItem>();
	_dice_cube_p2._dice4 = diceRitem4.get();
	XMStoreFloat4x4(&diceRitem4->World, DirectX::XMMatrixTranslation(0.0f, 0.0f, -0.5f) *
		DirectX::XMMatrixRotationX(3 * DirectX::XM_PIDIV2) * roMat *
		DirectX::XMMatrixTranslation(4.0f, 0.8f, 4.0f));
	XMStoreFloat4x4(&diceRitem4->TexTransform, DirectX::XMMatrixScaling(1.0f, 1.0f, 1.0f));
	diceRitem4->ObjCBIndex = g_ObjCBIndex++;
	diceRitem4->Geo = geometries["diceGeo"].get();
	diceRitem4->Mat = materials["dice40"].get();
	diceRitem4->PrimitiveType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	diceRitem4->IndexCount = diceRitem4->Geo->DrawArgs["dice"].IndexCount;
	diceRitem4->StartIndexLocation = diceRitem4->Geo->DrawArgs["dice"].StartIndexLocation;
	diceRitem4->BaseVertexLocation = diceRitem4->Geo->DrawArgs["dice"].BaseVertexLocation;
	diceRitem4->Bounds = diceRitem4->Geo->DrawArgs["dice"].Bounds;

	opaqueRenderItems.push_back(diceRitem4.get());
	allRitems.push_back(std::move(diceRitem4));
}

void Dice::BuildRenderItems(std::unordered_map<std::string, std::unique_ptr<MeshGeometry>>& geometries,
	std::unordered_map<std::string, std::unique_ptr<Material>>& materials,
	std::vector<std::unique_ptr<RenderItem>>& allRitems, std::vector<RenderItem*>& opaqueRenderItems)
{
	BuildRenderItems_p1(geometries, materials, allRitems, opaqueRenderItems);
	BuildRenderItems_p2(geometries, materials, allRitems, opaqueRenderItems);
}


void Dice::UpdateP1Cube(AbstractWindow* wnd, const GameTimer& gt)
{
	if (!_dice_cube_p1._need_update)
		return;

	float step = 4.0f * gt.DeltaTime();

	_dice_cube_p1._ax += step;
	_dice_cube_p1._ay += step;
	_dice_cube_p1._az += step;

	if (_dice_cube_p1._ax > _dice_cube_p1._axMax)
		_dice_cube_p1._ax = _dice_cube_p1._axMax;

	if (_dice_cube_p1._ay > _dice_cube_p1._ayMax)
		_dice_cube_p1._ay = _dice_cube_p1._ayMax;

	if (_dice_cube_p1._az > _dice_cube_p1._azMax)
		_dice_cube_p1._az = _dice_cube_p1._azMax;

	if (_dice_cube_p1._ax >= _dice_cube_p1._axMax &&
		_dice_cube_p1._ay >= _dice_cube_p1._ayMax &&
		_dice_cube_p1._az >= _dice_cube_p1._azMax)
	{
		_dice_cube_p1._need_update = false;
		wnd->FireProcessP1Msg(EV_DICE1_JUST_ROLLED);
	}

	DirectX::XMMATRIX roMat = DirectX::XMMatrixRotationX(_dice_cube_p1._ax) *
		DirectX::XMMatrixRotationY(_dice_cube_p1._ay) *
		DirectX::XMMatrixRotationZ(_dice_cube_p1._az);

	XMStoreFloat4x4(&_dice_cube_p1._dice1->World,
		DirectX::XMMatrixTranslation(0.0f, 0.0f, -0.5f) * roMat *
		DirectX::XMMatrixTranslation(-4.0f, 0.8f, -4.0f));
	_dice_cube_p1._dice1->NumFramesDirty = gNumFrameResources;

	XMStoreFloat4x4(&_dice_cube_p1._dice5->World,
		DirectX::XMMatrixRotationY(-DirectX::XM_PIDIV2) *
		DirectX::XMMatrixTranslation(0.5f, 0.0f, 0.0f) * roMat *
		DirectX::XMMatrixTranslation(-4.0f, 0.8f, -4.0f));
	_dice_cube_p1._dice5->NumFramesDirty = gNumFrameResources;

	XMStoreFloat4x4(&_dice_cube_p1._dice3->World,
		DirectX::XMMatrixTranslation(0.0f, 0.0f, -0.5f) *
		DirectX::XMMatrixRotationX(DirectX::XM_PIDIV2) * roMat *
		DirectX::XMMatrixTranslation(-4.0f, 0.8f, -4.0f));
	_dice_cube_p1._dice3->NumFramesDirty = gNumFrameResources;

	XMStoreFloat4x4(&_dice_cube_p1._dice6->World,
		DirectX::XMMatrixTranslation(0.0f, 0.0f, -0.5f) *
		DirectX::XMMatrixRotationY(DirectX::XM_PI) * roMat *
		DirectX::XMMatrixTranslation(-4.0f, 0.8f, -4.0f));
	_dice_cube_p1._dice6->NumFramesDirty = gNumFrameResources;

	XMStoreFloat4x4(&_dice_cube_p1._dice4->World,
		DirectX::XMMatrixTranslation(0.0f, 0.0f, -0.5f) *
		DirectX::XMMatrixRotationX(3 * DirectX::XM_PIDIV2) * roMat *
		DirectX::XMMatrixTranslation(-4.0f, 0.8f, -4.0f));
	_dice_cube_p1._dice4->NumFramesDirty = gNumFrameResources;

	XMStoreFloat4x4(&_dice_cube_p1._dice2->World,
		DirectX::XMMatrixRotationY(DirectX::XM_PIDIV2) *
		DirectX::XMMatrixTranslation(-0.5f, 0.0f, 0.0f) * roMat *
		DirectX::XMMatrixTranslation(-4.0f, 0.8f, -4.0f));
	_dice_cube_p1._dice2->NumFramesDirty = gNumFrameResources;
}

void Dice::UpdateP2Cube(AbstractWindow* wnd, const GameTimer& gt)
{
	if (!_dice_cube_p2._need_update)
		return;

	float step = 4.0f * gt.DeltaTime();

	_dice_cube_p2._ax += step;
	_dice_cube_p2._ay += step;
	_dice_cube_p2._az += step;

	_dice_cube_p2._at += step;

	if (_dice_cube_p2._ax > _dice_cube_p2._axMax)
		_dice_cube_p2._ax = _dice_cube_p2._axMax;

	if (_dice_cube_p2._ay > _dice_cube_p2._ayMax)
		_dice_cube_p2._ay = _dice_cube_p2._ayMax;

	if (_dice_cube_p2._az > _dice_cube_p2._azMax)
		_dice_cube_p2._az = _dice_cube_p2._azMax;

	if (_dice_cube_p2._ax >= _dice_cube_p2._axMax &&
		_dice_cube_p2._ay >= _dice_cube_p2._ayMax &&
		_dice_cube_p2._az >= _dice_cube_p2._azMax &&
		_dice_cube_p2._at >= _dice_cube_p2._atMax)
	{
		_dice_cube_p2._need_update = false;
		wnd->FireProcessP2Msg(EV_DICE2_JUST_ROLLED);
	}

	DirectX::XMMATRIX roMat = DirectX::XMMatrixRotationX(_dice_cube_p2._ax) *
		DirectX::XMMatrixRotationY(_dice_cube_p2._ay) *
		DirectX::XMMatrixRotationZ(_dice_cube_p2._az);

	XMStoreFloat4x4(&_dice_cube_p2._dice1->World,
		DirectX::XMMatrixTranslation(0.0f, 0.0f, -0.5f) * roMat *
		DirectX::XMMatrixTranslation(4.0f, 0.8f, 4.0f));
	_dice_cube_p2._dice1->NumFramesDirty = gNumFrameResources;

	XMStoreFloat4x4(&_dice_cube_p2._dice5->World,
		DirectX::XMMatrixRotationY(-DirectX::XM_PIDIV2) *
		DirectX::XMMatrixTranslation(0.5f, 0.0f, 0.0f) * roMat *
		DirectX::XMMatrixTranslation(4.0f, 0.8f, 4.0f));
	_dice_cube_p2._dice5->NumFramesDirty = gNumFrameResources;

	XMStoreFloat4x4(&_dice_cube_p2._dice3->World,
		DirectX::XMMatrixTranslation(0.0f, 0.0f, -0.5f) *
		DirectX::XMMatrixRotationX(DirectX::XM_PIDIV2) * roMat *
		DirectX::XMMatrixTranslation(4.0f, 0.8f, 4.0f));
	_dice_cube_p2._dice3->NumFramesDirty = gNumFrameResources;

	XMStoreFloat4x4(&_dice_cube_p2._dice6->World,
		DirectX::XMMatrixTranslation(0.0f, 0.0f, -0.5f) *
		DirectX::XMMatrixRotationY(DirectX::XM_PI) * roMat *
		DirectX::XMMatrixTranslation(4.0f, 0.8f, 4.0f));
	_dice_cube_p2._dice6->NumFramesDirty = gNumFrameResources;

	XMStoreFloat4x4(&_dice_cube_p2._dice4->World,
		DirectX::XMMatrixTranslation(0.0f, 0.0f, -0.5f) *
		DirectX::XMMatrixRotationX(3 * DirectX::XM_PIDIV2) * roMat *
		DirectX::XMMatrixTranslation(4.0f, 0.8f, 4.0f));
	_dice_cube_p2._dice4->NumFramesDirty = gNumFrameResources;

	XMStoreFloat4x4(&_dice_cube_p2._dice2->World,
		DirectX::XMMatrixRotationY(DirectX::XM_PIDIV2) *
		DirectX::XMMatrixTranslation(-0.5f, 0.0f, 0.0f) * roMat *
		DirectX::XMMatrixTranslation(4.0f, 0.8f, 4.0f));
	_dice_cube_p2._dice2->NumFramesDirty = gNumFrameResources;
}

void Dice::Update(AbstractWindow* wnd, const GameTimer& gt)
{
	UpdateP1Cube(wnd, gt);
	UpdateP2Cube(wnd, gt);
}
