#include "pch.h"
#include "platform.h"

#include <Fountain.h>

using namespace DirectX;

extern int g_ObjCBIndex;

Fountain::Fountain()
{
	XMStoreFloat4x4(&_InitialWorld, XMMatrixRotationX(XM_PI) *
		XMMatrixScaling(0.08f, 0.08f, 0.08f) *
		XMMatrixTranslation(0.0f, -1.0f, 0.0f));
}

void Fountain::LoadModel()
{
	_Model_Fountain = std::make_unique<Model>();
	_Model_Fountain->LoadCMOFile(MODELS_PATH L"Fountain.cmo");
}

void Fountain::BuildGeometry(ID3D12Device* devicePtr,
	ID3D12GraphicsCommandList* commandListPtr,
	std::unordered_map<std::string, std::unique_ptr<MeshGeometry>>& geometries)
{
	size_t vs = 0;
	size_t is = 0;

	for (UINT meshIdx = 0; meshIdx < FOUNTAIN_MESH_COUNT; ++meshIdx)
	{
		vs += _Model_Fountain->Meshes.at(meshIdx).Vertices.at(0).Vertices.size();
		is += _Model_Fountain->Meshes.at(meshIdx).Indices.at(0).Indices16.size();
	}

	std::vector<Vertex> vertices(vs);
	std::vector<std::uint16_t> indices(is);

	for (UINT meshIdx = 0, k = 0; meshIdx < FOUNTAIN_MESH_COUNT; ++meshIdx)
	{
		for (int ix = 0; ix < _Model_Fountain->Meshes.at(meshIdx).Vertices.at(0).Vertices.size(); ++ix)
		{
			vertices[k + ix].Pos = _Model_Fountain->Meshes.at(meshIdx).Vertices.at(0).Vertices.at(ix).Position;
			vertices[k + ix].Normal = _Model_Fountain->Meshes.at(meshIdx).Vertices.at(0).Vertices.at(ix).Normal;
			vertices[k + ix].TexC = _Model_Fountain->Meshes.at(meshIdx).Vertices.at(0).Vertices.at(ix).TextureCoordinates;
		}

		k += (UINT)_Model_Fountain->Meshes.at(meshIdx).Vertices.at(0).Vertices.size();
	}


	for (UINT meshIdx = 0, k = 0; meshIdx < FOUNTAIN_MESH_COUNT; ++meshIdx)
	{
		for (int ix = 0; ix < _Model_Fountain->Meshes.at(meshIdx).Indices.at(0).Indices16.size(); ++ix)
		{
			indices[k + ix] = _Model_Fountain->Meshes.at(meshIdx).Indices.at(0).Indices16.at(ix);
		}

		k += (UINT)_Model_Fountain->Meshes.at(meshIdx).Indices.at(0).Indices16.size();
	}

	const UINT vbByteSize = (UINT)vertices.size() * sizeof(Vertex);
	const UINT ibByteSize = (UINT)indices.size() * sizeof(std::uint16_t);

	auto geo = std::make_unique<MeshGeometry>();
	geo->Name = "FountainGeo";

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

	for (UINT meshIdx = 0, k = 0, j = 0; meshIdx < FOUNTAIN_MESH_COUNT; ++meshIdx)
	{
		std::string name = "Fountain_" + std::to_string(meshIdx);

		SubmeshGeometry submesh;
		submesh.IndexCount = (UINT)_Model_Fountain->Meshes.at(meshIdx).Indices.at(0).Indices16.size();
		submesh.StartIndexLocation = j;
		submesh.BaseVertexLocation = k;

		geo->DrawArgs[name.c_str()] = submesh;

		k += (UINT)_Model_Fountain->Meshes.at(meshIdx).Vertices.at(0).Vertices.size();
		j += (UINT)_Model_Fountain->Meshes.at(meshIdx).Indices.at(0).Indices16.size();
	}

	geometries[geo->Name] = std::move(geo);
}

void Fountain::BuildRenderItems(std::unordered_map<std::string, std::unique_ptr<MeshGeometry>>& geometries,
	std::unordered_map<std::string, std::unique_ptr<Material>>& materials,
	std::vector<std::unique_ptr<RenderItem>>& allRitems,
	std::vector<RenderItem*>& skinnedRenderItems)
{
	for (UINT meshIdx = 0; meshIdx < FOUNTAIN_MESH_COUNT; ++meshIdx)
	{
		std::string name = "Fountain_" + std::to_string(meshIdx);

		auto fountainRitem = std::make_unique<RenderItem>();
		XMStoreFloat4x4(&fountainRitem->World, XMLoadFloat4x4(&_InitialWorld));
		XMStoreFloat4x4(&fountainRitem->TexTransform, DirectX::XMMatrixScaling(1.0f, 1.0f, 1.0f));
		fountainRitem->ObjCBIndex = g_ObjCBIndex++;
		fountainRitem->Geo = geometries["FountainGeo"].get();
		fountainRitem->Mat = materials["fountain0"].get();
		fountainRitem->PrimitiveType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
		fountainRitem->IndexCount = fountainRitem->Geo->DrawArgs[name.c_str()].IndexCount;
		fountainRitem->StartIndexLocation = fountainRitem->Geo->DrawArgs[name.c_str()].StartIndexLocation;
		fountainRitem->BaseVertexLocation = fountainRitem->Geo->DrawArgs[name.c_str()].BaseVertexLocation;
		fountainRitem->Bounds = fountainRitem->Geo->DrawArgs[name.c_str()].Bounds;

		skinnedRenderItems.push_back(fountainRitem.get());
		allRitems.push_back(std::move(fountainRitem));
	}
}


void Fountain::Update(AbstractWindow* wnd, FrameResource* CurrFrameResource, const GameTimer& gt)
{

}
