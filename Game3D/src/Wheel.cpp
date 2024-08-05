#include "pch.h"
#include "platform.h"

#include <d3dUtil.h>
#include <GeometryGenerator.h>
#include <FrameResource.h>
#include <RenderItem.h>
#include <Wheel.h>

using namespace DirectX;

extern int g_ObjCBIndex;

Wheel::Wheel()
{
	XMStoreFloat4x4(&_InitialWorld, XMMatrixRotationX(-XM_PIDIV2) *
		XMMatrixRotationY(-XM_PIDIV4) *
		XMMatrixTranslation(-6.0f, 0.0f, 6.0f));
}

void Wheel::BuildGeometry(ID3D12Device* devicePtr,
	ID3D12GraphicsCommandList* commandListPtr,
	std::unordered_map<std::string,
	std::unique_ptr<MeshGeometry>>&geometries)
{
	GeometryGenerator geoGen;
	GeometryGenerator::MeshData wheel = geoGen.CreateSector(3.0f, 1.5f, 0.0f, XM_2PI, 0.2f, 100, 10, 5);

	size_t totalSize = wheel.Vertices.size();
	std::vector<Vertex> vertices(totalSize);

	UINT k = 0;
	for (size_t i = 0; i < wheel.Vertices.size(); ++i, ++k)
	{
		auto& p = wheel.Vertices[i].Position;
		vertices[k].Pos = p;
		vertices[k].Normal = wheel.Vertices[i].Normal;
		vertices[k].TexC = wheel.Vertices[i].TexC;
	}

	DirectX::BoundingBox button_bounds;
	DirectX::BoundingBox::CreateFromPoints(button_bounds, wheel.Vertices.size(),
		&vertices[0].Pos, sizeof(Vertex));

	const UINT vbByteSize = (UINT)vertices.size() * sizeof(Vertex);

	std::vector<std::uint16_t> indices;
	indices.insert(indices.end(), std::begin(wheel.GetIndices16()), std::end(wheel.GetIndices16()));
	const UINT ibByteSize = (UINT)indices.size() * sizeof(std::uint16_t);

	auto geo = std::make_unique<MeshGeometry>();
	geo->Name = "wheelGeo";

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

	SubmeshGeometry buttonSubmesh;
	buttonSubmesh.IndexCount = (UINT)wheel.Indices32.size();
	buttonSubmesh.StartIndexLocation = 0;
	buttonSubmesh.BaseVertexLocation = 0;
	buttonSubmesh.Bounds = button_bounds;

	geo->DrawArgs["wheel"] = buttonSubmesh;

	geometries[geo->Name] = std::move(geo);
}

void Wheel::BuildRenderItems(std::unordered_map<std::string,
	std::unique_ptr<MeshGeometry>>&geometries,
	std::unordered_map<std::string, std::unique_ptr<Material>>& materials,
	std::vector<std::unique_ptr<RenderItem>>& allRitems,
	std::vector<RenderItem*>& fixedRenderItems)
{
	auto wheelRitem = std::make_unique<RenderItem>();
	_WheelRItem = wheelRitem.get();
	XMStoreFloat4x4(&wheelRitem->World, XMLoadFloat4x4(&_InitialWorld));
	wheelRitem->ObjCBIndex = g_ObjCBIndex++;
	wheelRitem->Geo = geometries["wheelGeo"].get();
	wheelRitem->Mat = materials["logo0"].get();
	wheelRitem->PrimitiveType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	wheelRitem->IndexCount = wheelRitem->Geo->DrawArgs["wheel"].IndexCount;
	wheelRitem->StartIndexLocation = wheelRitem->Geo->DrawArgs["wheel"].StartIndexLocation;
	wheelRitem->BaseVertexLocation = wheelRitem->Geo->DrawArgs["wheel"].BaseVertexLocation;
	wheelRitem->Bounds = wheelRitem->Geo->DrawArgs["button"].Bounds;

	fixedRenderItems.push_back(wheelRitem.get());
	allRitems.push_back(std::move(wheelRitem));
}

void Wheel::Update(AbstractWindow* wnd, const GameTimer& gt)
{
	if (!_visible)
		return;

	float step = 2.0f * gt.DeltaTime();

	XMStoreFloat4x4(&_WheelRItem->World, XMMatrixRotationY(-step) * XMLoadFloat4x4(&_WheelRItem->World));
	_WheelRItem->NumFramesDirty = gNumFrameResources;
}
