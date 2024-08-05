#include "pch.h"
#include "platform.h"

#include <d3dUtil.h>
#include <GeometryGenerator.h>
#include <FrameResource.h>
#include <RenderItem.h>
#include <Board.h>

using namespace DirectX;

extern int g_ObjCBIndex;

void Board::BuildGeometry(ID3D12Device* devicePtr,
	ID3D12GraphicsCommandList* commandListPtr,
	std::unordered_map<std::string, std::unique_ptr<MeshGeometry>>& geometries)
{
	GeometryGenerator geoGen;
	GeometryGenerator::MeshData grid = geoGen.CreateGrid(12.0f, 12.0f, 12, 12);
	GeometryGenerator::MeshData disk = geoGen.CreateSector(0.3f, 0.3f, 0.0f, XM_2PI, 0.1f, 50, 2, 2);
	GeometryGenerator::MeshData ring = geoGen.CreateSector(0.32f, 0.02f, 0.0f, XM_2PI, 0.1f, 50, 2, 2);
	GeometryGenerator::MeshData line = geoGen.CreateCylinder(0.04f, 4.0f, 0.0f, XM_2PI, 50, 50);
	
	size_t totalSize = grid.Vertices.size() +
		disk.Vertices.size() +
		ring.Vertices.size() +
		line.Vertices.size();
	std::vector<Vertex> vertices(totalSize);

	UINT k = 0;
	for (size_t i = 0; i < grid.Vertices.size(); ++i, ++k)
	{
		auto& p = grid.Vertices[i].Position;
		vertices[k].Pos = p;
		vertices[k].Normal = grid.Vertices[i].Normal;
		vertices[k].TexC = grid.Vertices[i].TexC;
	}

	BoundingBox grid_bounds;
	BoundingBox::CreateFromPoints(grid_bounds, grid.Vertices.size(),
		&vertices[0].Pos, sizeof(Vertex));

	for (size_t i = 0; i < disk.Vertices.size(); ++i, ++k)
	{
		auto& p = disk.Vertices[i].Position;
		vertices[k].Pos = p;
		vertices[k].Normal = disk.Vertices[i].Normal;
		vertices[k].TexC = disk.Vertices[i].TexC;
	}

	BoundingBox disk_bounds;
	BoundingBox::CreateFromPoints(disk_bounds, disk.Vertices.size(),
		&vertices[grid.Vertices.size()].Pos, sizeof(Vertex));

	for (size_t i = 0; i < ring.Vertices.size(); ++i, ++k)
	{
		auto& p = ring.Vertices[i].Position;
		vertices[k].Pos = p;
		vertices[k].Normal = ring.Vertices[i].Normal;
		vertices[k].TexC = ring.Vertices[i].TexC;
	}

	BoundingBox ring_bounds;
	BoundingBox::CreateFromPoints(ring_bounds, ring.Vertices.size(),
		&vertices[grid.Vertices.size() +
		disk.Vertices.size()].Pos, sizeof(Vertex));

	for (size_t i = 0; i < line.Vertices.size(); ++i, ++k)
	{
		auto& p = line.Vertices[i].Position;
		vertices[k].Pos = p;
		vertices[k].Normal = line.Vertices[i].Normal;
		vertices[k].TexC = line.Vertices[i].TexC;
	}

	BoundingBox line_bounds;
	BoundingBox::CreateFromPoints(line_bounds, line.Vertices.size(),
		&vertices[grid.Vertices.size() +
		disk.Vertices.size() +
		ring.Vertices.size()].Pos, sizeof(Vertex));

	const UINT vbByteSize = (UINT)vertices.size() * sizeof(Vertex);

	std::vector<std::uint16_t> indices;
	indices.insert(indices.end(), std::begin(grid.GetIndices16()), std::end(grid.GetIndices16()));
	indices.insert(indices.end(), std::begin(disk.GetIndices16()), std::end(disk.GetIndices16()));
	indices.insert(indices.end(), std::begin(ring.GetIndices16()), std::end(ring.GetIndices16()));
	indices.insert(indices.end(), std::begin(line.GetIndices16()), std::end(line.GetIndices16()));
	
	const UINT ibByteSize = (UINT)indices.size() * sizeof(std::uint16_t);

	auto geo = std::make_unique<MeshGeometry>();
	geo->Name = "boardGeo";

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

	SubmeshGeometry gridSubmesh;
	gridSubmesh.IndexCount = (UINT)grid.Indices32.size();
	gridSubmesh.StartIndexLocation = 0;
	gridSubmesh.BaseVertexLocation = 0;
	gridSubmesh.Bounds = grid_bounds;

	SubmeshGeometry diskSubmesh;
	diskSubmesh.IndexCount = (UINT)disk.Indices32.size();
	diskSubmesh.StartIndexLocation = (UINT)grid.Indices32.size();
	diskSubmesh.BaseVertexLocation = (UINT)grid.Vertices.size();
	diskSubmesh.Bounds = disk_bounds;

	SubmeshGeometry ringSubmesh;
	ringSubmesh.IndexCount = (UINT)ring.Indices32.size();
	ringSubmesh.StartIndexLocation = (UINT)grid.Indices32.size() +
		(UINT)disk.Indices32.size();
	ringSubmesh.BaseVertexLocation = (UINT)grid.Vertices.size() +
		(UINT)disk.Vertices.size();
	ringSubmesh.Bounds = ring_bounds;

	SubmeshGeometry lineSubmesh;
	lineSubmesh.IndexCount = (UINT)line.Indices32.size();
	lineSubmesh.StartIndexLocation = (UINT)grid.Indices32.size() +
		(UINT)disk.Indices32.size() +
		(UINT)ring.Indices32.size();
	lineSubmesh.BaseVertexLocation = (UINT)grid.Vertices.size() +
		(UINT)disk.Vertices.size() +
		(UINT)ring.Vertices.size();
	lineSubmesh.Bounds = line_bounds;

	geo->DrawArgs["grid"] = gridSubmesh;
	geo->DrawArgs["disk"] = diskSubmesh;
	geo->DrawArgs["ring"] = ringSubmesh;
	geo->DrawArgs["line"] = lineSubmesh;
	
	geometries[geo->Name] = std::move(geo);
}

void Board::BuildRenderItems_grid(std::unordered_map<std::string, std::unique_ptr<MeshGeometry>>& geometries,
	std::unordered_map<std::string, std::unique_ptr<Material>>& materials,
	std::vector<std::unique_ptr<RenderItem>>& allRitems,
	std::vector<RenderItem*>& opaqueRenderItems)
{
	auto gridRitem = std::make_unique<RenderItem>();
	gridRitem->World = MathHelper::Identity4x4();
	XMStoreFloat4x4(&gridRitem->TexTransform, DirectX::XMMatrixScaling(1.0f, 1.0f, 1.0f));
	gridRitem->ObjCBIndex = g_ObjCBIndex++;
	gridRitem->Geo = geometries["boardGeo"].get();
	gridRitem->Mat = materials["ground0"].get();
	gridRitem->PrimitiveType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	gridRitem->IndexCount = gridRitem->Geo->DrawArgs["grid"].IndexCount;
	gridRitem->StartIndexLocation = gridRitem->Geo->DrawArgs["grid"].StartIndexLocation;
	gridRitem->BaseVertexLocation = gridRitem->Geo->DrawArgs["grid"].BaseVertexLocation;

	opaqueRenderItems.push_back(gridRitem.get());
	allRitems.push_back(std::move(gridRitem));
}

void Board::BuildRenderItems_lines(std::unordered_map<std::string, std::unique_ptr<MeshGeometry>>& geometries,
	std::unordered_map<std::string, std::unique_ptr<Material>>& materials,
	std::vector<std::unique_ptr<RenderItem>>& allRitems,
	std::vector<RenderItem*>& opaqueRenderItems)
{
	auto lineRitem12 = std::make_unique<RenderItem>();
	XMStoreFloat4x4(&lineRitem12->World,
		DirectX::XMMatrixRotationX(DirectX::XM_PIDIV2) *
		DirectX::XMMatrixTranslation(1.0f * -1, 0.15f, 1.0f * -5));
	//XMStoreFloat4x4(&lineRitem->TexTransform, DirectX::XMMatrixScaling(0.0f, 0.0f, 0.0f));
	lineRitem12->ObjCBIndex = g_ObjCBIndex++;
	lineRitem12->Geo = geometries["boardGeo"].get();
	lineRitem12->Mat = materials["ring0"].get();
	lineRitem12->PrimitiveType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	lineRitem12->IndexCount = lineRitem12->Geo->DrawArgs["line"].IndexCount;
	lineRitem12->StartIndexLocation = lineRitem12->Geo->DrawArgs["line"].StartIndexLocation;
	lineRitem12->BaseVertexLocation = lineRitem12->Geo->DrawArgs["line"].BaseVertexLocation;

	opaqueRenderItems.push_back(lineRitem12.get());
	allRitems.push_back(std::move(lineRitem12));

	auto lineRitem11 = std::make_unique<RenderItem>();
	XMStoreFloat4x4(&lineRitem11->World, DirectX::XMMatrixRotationX(-DirectX::XM_PIDIV2) *
		DirectX::XMMatrixTranslation(1.0f * -1, 0.15f, 1.0f * 5));
	//XMStoreFloat4x4(&lineRitem->TexTransform, DirectX::XMMatrixScaling(0.0f, 0.0f, 0.0f));
	lineRitem11->ObjCBIndex = g_ObjCBIndex++;
	lineRitem11->Geo = geometries["boardGeo"].get();
	lineRitem11->Mat = materials["ring0"].get();
	lineRitem11->PrimitiveType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	lineRitem11->IndexCount = lineRitem11->Geo->DrawArgs["line"].IndexCount;
	lineRitem11->StartIndexLocation = lineRitem11->Geo->DrawArgs["line"].StartIndexLocation;
	lineRitem11->BaseVertexLocation = lineRitem11->Geo->DrawArgs["line"].BaseVertexLocation;

	opaqueRenderItems.push_back(lineRitem11.get());
	allRitems.push_back(std::move(lineRitem11));

	auto lineRitem10 = std::make_unique<RenderItem>();
	XMStoreFloat4x4(&lineRitem10->World, DirectX::XMMatrixRotationX(DirectX::XM_PIDIV2) *
		DirectX::XMMatrixRotationY(DirectX::XM_PIDIV2) *
		DirectX::XMMatrixTranslation(1.0f * -5, 0.15f, 1.0f * 1));
	//XMStoreFloat4x4(&lineRitem->TexTransform, DirectX::XMMatrixScaling(0.0f, 0.0f, 0.0f));
	lineRitem10->ObjCBIndex = g_ObjCBIndex++;
	lineRitem10->Geo = geometries["boardGeo"].get();
	lineRitem10->Mat = materials["ring0"].get();
	lineRitem10->PrimitiveType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	lineRitem10->IndexCount = lineRitem10->Geo->DrawArgs["line"].IndexCount;
	lineRitem10->StartIndexLocation = lineRitem10->Geo->DrawArgs["line"].StartIndexLocation;
	lineRitem10->BaseVertexLocation = lineRitem10->Geo->DrawArgs["line"].BaseVertexLocation;

	opaqueRenderItems.push_back(lineRitem10.get());
	allRitems.push_back(std::move(lineRitem10));

	auto lineRitem9 = std::make_unique<RenderItem>();
	XMStoreFloat4x4(&lineRitem9->World, DirectX::XMMatrixRotationX(-DirectX::XM_PIDIV2) *
		DirectX::XMMatrixRotationY(DirectX::XM_PIDIV2) *
		DirectX::XMMatrixTranslation(1.0f * 5, 0.15f, 1.0f * 1));
	//XMStoreFloat4x4(&lineRitem->TexTransform, DirectX::XMMatrixScaling(0.0f, 0.0f, 0.0f));
	lineRitem9->ObjCBIndex = g_ObjCBIndex++;
	lineRitem9->Geo = geometries["boardGeo"].get();
	lineRitem9->Mat = materials["ring0"].get();
	lineRitem9->PrimitiveType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	lineRitem9->IndexCount = lineRitem9->Geo->DrawArgs["line"].IndexCount;
	lineRitem9->StartIndexLocation = lineRitem9->Geo->DrawArgs["line"].StartIndexLocation;
	lineRitem9->BaseVertexLocation = lineRitem9->Geo->DrawArgs["line"].BaseVertexLocation;

	opaqueRenderItems.push_back(lineRitem9.get());
	allRitems.push_back(std::move(lineRitem9));

	auto lineRitem8 = std::make_unique<RenderItem>();
	XMStoreFloat4x4(&lineRitem8->World, DirectX::XMMatrixRotationX(-DirectX::XM_PIDIV2) *
		DirectX::XMMatrixRotationY(DirectX::XM_PIDIV2) *
		DirectX::XMMatrixTranslation(1.0f * -1, 0.15f, 1.0f * -1));
	//XMStoreFloat4x4(&lineRitem->TexTransform, DirectX::XMMatrixScaling(0.0f, 0.0f, 0.0f));
	lineRitem8->ObjCBIndex = g_ObjCBIndex++;
	lineRitem8->Geo = geometries["boardGeo"].get();
	lineRitem8->Mat = materials["ring0"].get();
	lineRitem8->PrimitiveType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	lineRitem8->IndexCount = lineRitem8->Geo->DrawArgs["line"].IndexCount;
	lineRitem8->StartIndexLocation = lineRitem8->Geo->DrawArgs["line"].StartIndexLocation;
	lineRitem8->BaseVertexLocation = lineRitem8->Geo->DrawArgs["line"].BaseVertexLocation;

	opaqueRenderItems.push_back(lineRitem8.get());
	allRitems.push_back(std::move(lineRitem8));

	auto lineRitem7 = std::make_unique<RenderItem>();
	XMStoreFloat4x4(&lineRitem7->World, DirectX::XMMatrixScaling(1.0f, 0.5f, 1.0f) * 
		DirectX::XMMatrixRotationX(DirectX::XM_PIDIV2) *
		DirectX::XMMatrixRotationY(DirectX::XM_PIDIV2) *
		DirectX::XMMatrixTranslation(1.0f * -1, 0.15f, 1.0f * 5));
	//XMStoreFloat4x4(&lineRitem->TexTransform, DirectX::XMMatrixScaling(0.0f, 0.0f, 0.0f));
	lineRitem7->ObjCBIndex = g_ObjCBIndex++;
	lineRitem7->Geo = geometries["boardGeo"].get();
	lineRitem7->Mat = materials["ring0"].get();
	lineRitem7->PrimitiveType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	lineRitem7->IndexCount = lineRitem7->Geo->DrawArgs["line"].IndexCount;
	lineRitem7->StartIndexLocation = lineRitem7->Geo->DrawArgs["line"].StartIndexLocation;
	lineRitem7->BaseVertexLocation = lineRitem7->Geo->DrawArgs["line"].BaseVertexLocation;

	opaqueRenderItems.push_back(lineRitem7.get());
	allRitems.push_back(std::move(lineRitem7));

	auto lineRitem6 = std::make_unique<RenderItem>();
	XMStoreFloat4x4(&lineRitem6->World, DirectX::XMMatrixRotationX(-DirectX::XM_PIDIV2) *
		DirectX::XMMatrixTranslation(1.0f * 1, 0.15f, 1.0f * -1));
	//XMStoreFloat4x4(&lineRitem->TexTransform, DirectX::XMMatrixScaling(0.0f, 0.0f, 0.0f));
	lineRitem6->ObjCBIndex = g_ObjCBIndex++;
	lineRitem6->Geo = geometries["boardGeo"].get();
	lineRitem6->Mat = materials["ring0"].get();
	lineRitem6->PrimitiveType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	lineRitem6->IndexCount = lineRitem6->Geo->DrawArgs["line"].IndexCount;
	lineRitem6->StartIndexLocation = lineRitem6->Geo->DrawArgs["line"].StartIndexLocation;
	lineRitem6->BaseVertexLocation = lineRitem6->Geo->DrawArgs["line"].BaseVertexLocation;

	opaqueRenderItems.push_back(lineRitem6.get());
	allRitems.push_back(std::move(lineRitem6));

	auto lineRitem5 = std::make_unique<RenderItem>();
	XMStoreFloat4x4(&lineRitem5->World, DirectX::XMMatrixRotationX(-DirectX::XM_PIDIV2) *
		DirectX::XMMatrixTranslation(1.0f * 1, 0.15f, 1.0f * 5));
	//XMStoreFloat4x4(&lineRitem->TexTransform, DirectX::XMMatrixScaling(0.0f, 0.0f, 0.0f));
	lineRitem5->ObjCBIndex = g_ObjCBIndex++;
	lineRitem5->Geo = geometries["boardGeo"].get();
	lineRitem5->Mat = materials["ring0"].get();
	lineRitem5->PrimitiveType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	lineRitem5->IndexCount = lineRitem5->Geo->DrawArgs["line"].IndexCount;
	lineRitem5->StartIndexLocation = lineRitem5->Geo->DrawArgs["line"].StartIndexLocation;
	lineRitem5->BaseVertexLocation = lineRitem5->Geo->DrawArgs["line"].BaseVertexLocation;

	opaqueRenderItems.push_back(lineRitem5.get());
	allRitems.push_back(std::move(lineRitem5));

	auto lineRitem4 = std::make_unique<RenderItem>();
	XMStoreFloat4x4(&lineRitem4->World, DirectX::XMMatrixRotationX(DirectX::XM_PIDIV2) *
		DirectX::XMMatrixRotationY(DirectX::XM_PIDIV2)*
		DirectX::XMMatrixTranslation(1.0f * 1, 0.15f, 1.0f * -1));
	//XMStoreFloat4x4(&lineRitem->TexTransform, DirectX::XMMatrixScaling(0.0f, 0.0f, 0.0f));
	lineRitem4->ObjCBIndex = g_ObjCBIndex++;
	lineRitem4->Geo = geometries["boardGeo"].get();
	lineRitem4->Mat = materials["ring0"].get();
	lineRitem4->PrimitiveType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	lineRitem4->IndexCount = lineRitem4->Geo->DrawArgs["line"].IndexCount;
	lineRitem4->StartIndexLocation = lineRitem4->Geo->DrawArgs["line"].StartIndexLocation;
	lineRitem4->BaseVertexLocation = lineRitem4->Geo->DrawArgs["line"].BaseVertexLocation;

	opaqueRenderItems.push_back(lineRitem4.get());
	allRitems.push_back(std::move(lineRitem4));

	auto lineRitem3 = std::make_unique<RenderItem>();
	XMStoreFloat4x4(&lineRitem3->World, DirectX::XMMatrixScaling(1.0f, 0.5f, 1.0f)* 
		DirectX::XMMatrixRotationX(-DirectX::XM_PIDIV2) *
		DirectX::XMMatrixTranslation(1.0f * -5, 0.15f, 1.0f * 1));
	//XMStoreFloat4x4(&lineRitem->TexTransform, DirectX::XMMatrixScaling(0.0f, 0.0f, 0.0f));
	lineRitem3->ObjCBIndex = g_ObjCBIndex++;
	lineRitem3->Geo = geometries["boardGeo"].get();
	lineRitem3->Mat = materials["ring0"].get();
	lineRitem3->PrimitiveType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	lineRitem3->IndexCount = lineRitem3->Geo->DrawArgs["line"].IndexCount;
	lineRitem3->StartIndexLocation = lineRitem3->Geo->DrawArgs["line"].StartIndexLocation;
	lineRitem3->BaseVertexLocation = lineRitem3->Geo->DrawArgs["line"].BaseVertexLocation;

	opaqueRenderItems.push_back(lineRitem3.get());
	allRitems.push_back(std::move(lineRitem3));

	auto lineRitem2 = std::make_unique<RenderItem>();
	XMStoreFloat4x4(&lineRitem2->World, DirectX::XMMatrixScaling(1.0f, 0.5f, 1.0f)* 
		DirectX::XMMatrixRotationX(DirectX::XM_PIDIV2) *
		DirectX::XMMatrixTranslation(1.0f * 5, 0.15f, 1.0f * -1));
	//XMStoreFloat4x4(&lineRitem->TexTransform, DirectX::XMMatrixScaling(0.0f, 0.0f, 0.0f));
	lineRitem2->ObjCBIndex = g_ObjCBIndex++;
	lineRitem2->Geo = geometries["boardGeo"].get();
	lineRitem2->Mat = materials["ring0"].get();
	lineRitem2->PrimitiveType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	lineRitem2->IndexCount = lineRitem2->Geo->DrawArgs["line"].IndexCount;
	lineRitem2->StartIndexLocation = lineRitem2->Geo->DrawArgs["line"].StartIndexLocation;
	lineRitem2->BaseVertexLocation = lineRitem2->Geo->DrawArgs["line"].BaseVertexLocation;

	opaqueRenderItems.push_back(lineRitem2.get());
	allRitems.push_back(std::move(lineRitem2));

	auto lineRitem1 = std::make_unique<RenderItem>();
	XMStoreFloat4x4(&lineRitem1->World, DirectX::XMMatrixScaling(1.0f, 0.5f, 1.0f)* 
		DirectX::XMMatrixRotationX(DirectX::XM_PIDIV2) *
		DirectX::XMMatrixRotationY(DirectX::XM_PIDIV2)*
		DirectX::XMMatrixTranslation(1.0f * -1, 0.15f, 1.0f * -5));
	//XMStoreFloat4x4(&lineRitem->TexTransform, DirectX::XMMatrixScaling(0.0f, 0.0f, 0.0f));
	lineRitem1->ObjCBIndex = g_ObjCBIndex++;
	lineRitem1->Geo = geometries["boardGeo"].get();
	lineRitem1->Mat = materials["ring0"].get();
	lineRitem1->PrimitiveType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	lineRitem1->IndexCount = lineRitem1->Geo->DrawArgs["line"].IndexCount;
	lineRitem1->StartIndexLocation = lineRitem1->Geo->DrawArgs["line"].StartIndexLocation;
	lineRitem1->BaseVertexLocation = lineRitem1->Geo->DrawArgs["line"].BaseVertexLocation;

	opaqueRenderItems.push_back(lineRitem1.get());
	allRitems.push_back(std::move(lineRitem1));
}

void Board::BuildRenderItems_disk(std::unordered_map<std::string, std::unique_ptr<MeshGeometry>>& geometries,
	std::unordered_map<std::string, std::unique_ptr<Material>>& materials,
	std::vector<std::unique_ptr<RenderItem>>& allRitems,
	std::vector<RenderItem*>& opaqueRenderItems,
	int x0, int z0)
{
	auto diskRitem = std::make_unique<RenderItem>();
	XMStoreFloat4x4(&diskRitem->World, DirectX::XMMatrixTranslation(1.0f * x0, 0.1f, 1.0f * z0));
	XMStoreFloat4x4(&diskRitem->TexTransform, DirectX::XMMatrixScaling(0.0f, 0.0f, 0.0f));
	diskRitem->ObjCBIndex = g_ObjCBIndex++;
	diskRitem->Geo = geometries["boardGeo"].get();
	diskRitem->Mat = materials["diskRoad0"].get();
	diskRitem->PrimitiveType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	diskRitem->IndexCount = diskRitem->Geo->DrawArgs["disk"].IndexCount;
	diskRitem->StartIndexLocation = diskRitem->Geo->DrawArgs["disk"].StartIndexLocation;
	diskRitem->BaseVertexLocation = diskRitem->Geo->DrawArgs["disk"].BaseVertexLocation;

	opaqueRenderItems.push_back(diskRitem.get());
	allRitems.push_back(std::move(diskRitem));

	auto ringRitem = std::make_unique<RenderItem>();
	XMStoreFloat4x4(&ringRitem->World, DirectX::XMMatrixTranslation(1.0f * x0, 0.1f, 1.0f * z0));
	XMStoreFloat4x4(&ringRitem->TexTransform, DirectX::XMMatrixScaling(0.0f, 0.0f, 0.0f));
	ringRitem->ObjCBIndex = g_ObjCBIndex++;
	ringRitem->Geo = geometries["boardGeo"].get();
	ringRitem->Mat = materials["ring0"].get();
	ringRitem->PrimitiveType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	ringRitem->IndexCount = ringRitem->Geo->DrawArgs["ring"].IndexCount;
	ringRitem->StartIndexLocation = ringRitem->Geo->DrawArgs["ring"].StartIndexLocation;
	ringRitem->BaseVertexLocation = ringRitem->Geo->DrawArgs["ring"].BaseVertexLocation;

	opaqueRenderItems.push_back(ringRitem.get());
	allRitems.push_back(std::move(ringRitem));
}

void Board::BuildRenderItems_smallDisk(std::unordered_map<std::string, std::unique_ptr<MeshGeometry>>& geometries,
	std::unordered_map<std::string, std::unique_ptr<Material>>& materials,
	std::vector<std::unique_ptr<RenderItem>>& allRitems,
	std::vector<RenderItem*>& opaqueRenderItems,
	int x0, int z0, const char* matStr)
{
	auto s_diskRitem = std::make_unique<RenderItem>();
	XMStoreFloat4x4(&s_diskRitem->World,
		DirectX::XMMatrixScaling(0.8f, 1.0f, 0.8f) *
		DirectX::XMMatrixTranslation(1.0f * x0, 0.1f, 1.0f * z0));
	XMStoreFloat4x4(&s_diskRitem->TexTransform, DirectX::XMMatrixScaling(1.0f, 1.0f, 1.0f));
	s_diskRitem->ObjCBIndex = g_ObjCBIndex++;
	s_diskRitem->Geo = geometries["boardGeo"].get();
	s_diskRitem->Mat = materials[matStr].get();
	s_diskRitem->PrimitiveType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	s_diskRitem->IndexCount = s_diskRitem->Geo->DrawArgs["disk"].IndexCount;
	s_diskRitem->StartIndexLocation = s_diskRitem->Geo->DrawArgs["disk"].StartIndexLocation;
	s_diskRitem->BaseVertexLocation = s_diskRitem->Geo->DrawArgs["disk"].BaseVertexLocation;

	opaqueRenderItems.push_back(s_diskRitem.get());
	allRitems.push_back(std::move(s_diskRitem));
}

void Board::BuildRenderItems(std::unordered_map<std::string, std::unique_ptr<MeshGeometry>>& geometries,
	std::unordered_map<std::string, std::unique_ptr<Material>>& materials,
	std::vector<std::unique_ptr<RenderItem>>& allRitems, std::vector<RenderItem*>& opaqueRenderItems)
{
	BuildRenderItems_grid(geometries, materials, allRitems, opaqueRenderItems);
	BuildRenderItems_lines(geometries, materials, allRitems, opaqueRenderItems);

	// ------------- disks ---------------------------------

	BuildRenderItems_disk(geometries, materials, allRitems, opaqueRenderItems, 5, -1);
	BuildRenderItems_disk(geometries, materials, allRitems, opaqueRenderItems, 4, -1);
	BuildRenderItems_disk(geometries, materials, allRitems, opaqueRenderItems, 3, -1);
	BuildRenderItems_disk(geometries, materials, allRitems, opaqueRenderItems, 2, -1);
	BuildRenderItems_disk(geometries, materials, allRitems, opaqueRenderItems, 1, -1);

	BuildRenderItems_disk(geometries, materials, allRitems, opaqueRenderItems, 1, -2);
	BuildRenderItems_disk(geometries, materials, allRitems, opaqueRenderItems, 1, -3);
	BuildRenderItems_disk(geometries, materials, allRitems, opaqueRenderItems, 1, -4);
	BuildRenderItems_disk(geometries, materials, allRitems, opaqueRenderItems, 1, -5);

	BuildRenderItems_disk(geometries, materials, allRitems, opaqueRenderItems, 0, -5);
	BuildRenderItems_disk(geometries, materials, allRitems, opaqueRenderItems, -1, -5);

	BuildRenderItems_disk(geometries, materials, allRitems, opaqueRenderItems, -1, -4);
	BuildRenderItems_disk(geometries, materials, allRitems, opaqueRenderItems, -1, -3);
	BuildRenderItems_disk(geometries, materials, allRitems, opaqueRenderItems, -1, -2);
	BuildRenderItems_disk(geometries, materials, allRitems, opaqueRenderItems, -1, -1);

	BuildRenderItems_disk(geometries, materials, allRitems, opaqueRenderItems, -2, -1);
	BuildRenderItems_disk(geometries, materials, allRitems, opaqueRenderItems, -3, -1);
	BuildRenderItems_disk(geometries, materials, allRitems, opaqueRenderItems, -4, -1);
	BuildRenderItems_disk(geometries, materials, allRitems, opaqueRenderItems, -5, -1);

	BuildRenderItems_disk(geometries, materials, allRitems, opaqueRenderItems, -5, 0);
	BuildRenderItems_disk(geometries, materials, allRitems, opaqueRenderItems, -5, 1);

	BuildRenderItems_disk(geometries, materials, allRitems, opaqueRenderItems, -4, 1);
	BuildRenderItems_disk(geometries, materials, allRitems, opaqueRenderItems, -3, 1);
	BuildRenderItems_disk(geometries, materials, allRitems, opaqueRenderItems, -2, 1);
	BuildRenderItems_disk(geometries, materials, allRitems, opaqueRenderItems, -1, 1);

	BuildRenderItems_disk(geometries, materials, allRitems, opaqueRenderItems, -1, 2);
	BuildRenderItems_disk(geometries, materials, allRitems, opaqueRenderItems, -1, 3);
	BuildRenderItems_disk(geometries, materials, allRitems, opaqueRenderItems, -1, 4);
	BuildRenderItems_disk(geometries, materials, allRitems, opaqueRenderItems, -1, 5);

	BuildRenderItems_disk(geometries, materials, allRitems, opaqueRenderItems, 0, 5);
	BuildRenderItems_disk(geometries, materials, allRitems, opaqueRenderItems, 1, 5);

	BuildRenderItems_disk(geometries, materials, allRitems, opaqueRenderItems, 1, 4);
	BuildRenderItems_disk(geometries, materials, allRitems, opaqueRenderItems, 1, 3);
	BuildRenderItems_disk(geometries, materials, allRitems, opaqueRenderItems, 1, 2);
	BuildRenderItems_disk(geometries, materials, allRitems, opaqueRenderItems, 1, 1);

	BuildRenderItems_disk(geometries, materials, allRitems, opaqueRenderItems, 2, 1);
	BuildRenderItems_disk(geometries, materials, allRitems, opaqueRenderItems, 3, 1);
	BuildRenderItems_disk(geometries, materials, allRitems, opaqueRenderItems, 4, 1);
	BuildRenderItems_disk(geometries, materials, allRitems, opaqueRenderItems, 5, 1);

	BuildRenderItems_disk(geometries, materials, allRitems, opaqueRenderItems, 5, 0);

	// ------------- small disks ---------------------------------

	BuildRenderItems_smallDisk(geometries, materials, allRitems, opaqueRenderItems, 5, -5, "diskP10");
	BuildRenderItems_smallDisk(geometries, materials, allRitems, opaqueRenderItems, 5, -4, "diskP10");
	BuildRenderItems_smallDisk(geometries, materials, allRitems, opaqueRenderItems, 4, -5, "diskP10");
	BuildRenderItems_smallDisk(geometries, materials, allRitems, opaqueRenderItems, 4, -4, "diskP10");

	BuildRenderItems_smallDisk(geometries, materials, allRitems, opaqueRenderItems, 4, 0, "diskP10");
	BuildRenderItems_smallDisk(geometries, materials, allRitems, opaqueRenderItems, 3, 0, "diskP10");
	BuildRenderItems_smallDisk(geometries, materials, allRitems, opaqueRenderItems, 2, 0, "diskP10");
	BuildRenderItems_smallDisk(geometries, materials, allRitems, opaqueRenderItems, 1, 0, "diskP10");

	BuildRenderItems_smallDisk(geometries, materials, allRitems, opaqueRenderItems, -5, 5, "diskP20");
	BuildRenderItems_smallDisk(geometries, materials, allRitems, opaqueRenderItems, -5, 4, "diskP20");
	BuildRenderItems_smallDisk(geometries, materials, allRitems, opaqueRenderItems, -4, 5, "diskP20");
	BuildRenderItems_smallDisk(geometries, materials, allRitems, opaqueRenderItems, -4, 4, "diskP20");

	BuildRenderItems_smallDisk(geometries, materials, allRitems, opaqueRenderItems, -4, 0, "diskP20");
	BuildRenderItems_smallDisk(geometries, materials, allRitems, opaqueRenderItems, -3, 0, "diskP20");
	BuildRenderItems_smallDisk(geometries, materials, allRitems, opaqueRenderItems, -2, 0, "diskP20");
	BuildRenderItems_smallDisk(geometries, materials, allRitems, opaqueRenderItems, -1, 0, "diskP20");
}
