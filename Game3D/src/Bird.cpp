#include "pch.h"
#include "platform.h"

#include <Bird.h>

using namespace DirectX;

extern int g_ObjCBIndex;

Bird::Bird()
{
	XMStoreFloat4x4(&_InitialWorld, XMMatrixRotationY(DirectX::XM_PI) *
		XMMatrixScaling(0.03f, 0.03f, 0.03f));
}

void Bird::LoadModel()
{
	_Model_Bird = std::make_unique<Model>();
	_Model_Bird->LoadCMOFile(MODELS_PATH L"Bird.cmo");
}

void Bird::BuildSkinnedModel(ID3D12Device* devicePtr,
	ID3D12GraphicsCommandList* commandListPtr,
	std::unordered_map<std::string, std::unique_ptr<MeshGeometry>>& geometries)
{
	size_t vs = 0;
	size_t is = 0;

	for (UINT meshIdx = 0; meshIdx < BIRD_MESH_COUNT; ++meshIdx)
	{
		for (UINT subIdx = 0; subIdx < _Model_Bird->Meshes.at(meshIdx).SubMeshes.size(); ++subIdx)
		{
			vs += _Model_Bird->Meshes.at(meshIdx).Vertices.at(subIdx).Vertices.size();
			is += _Model_Bird->Meshes.at(meshIdx).Indices.at(subIdx).Indices16.size();
		}
	}

	std::vector<SkinnedVertex> vertices(vs);
	std::vector<std::uint16_t> indices(is);

	for (UINT meshIdx = 0, k = 0; meshIdx < BIRD_MESH_COUNT; ++meshIdx)
	{
		for (UINT subIdx = 0; subIdx < _Model_Bird->Meshes.at(meshIdx).SubMeshes.size(); ++subIdx)
		{
			for (int ix = 0; ix < _Model_Bird->Meshes.at(meshIdx).Vertices.at(subIdx).Vertices.size(); ++ix)
			{
				vertices[k + ix].Pos = _Model_Bird->Meshes.at(meshIdx).Vertices.at(subIdx).Vertices.at(ix).Position;
				vertices[k + ix].Normal = _Model_Bird->Meshes.at(meshIdx).Vertices.at(subIdx).Vertices.at(ix).Normal;
				vertices[k + ix].TexC = _Model_Bird->Meshes.at(meshIdx).Vertices.at(subIdx).Vertices.at(ix).TextureCoordinates;

				vertices[k + ix].BoneWeights.x = _Model_Bird->Meshes.at(meshIdx).SkinningVertices.at(subIdx).SkinningVertices.at(ix).boneWeight[0];
				vertices[k + ix].BoneWeights.y = _Model_Bird->Meshes.at(meshIdx).SkinningVertices.at(subIdx).SkinningVertices.at(ix).boneWeight[1];
				vertices[k + ix].BoneWeights.z = _Model_Bird->Meshes.at(meshIdx).SkinningVertices.at(subIdx).SkinningVertices.at(ix).boneWeight[2];
				vertices[k + ix].BoneWeights.w = _Model_Bird->Meshes.at(meshIdx).SkinningVertices.at(subIdx).SkinningVertices.at(ix).boneWeight[3];

				vertices[k + ix].BoneIndices[0] = _Model_Bird->Meshes.at(meshIdx).SkinningVertices.at(subIdx).SkinningVertices.at(ix).boneIndex[0];
				vertices[k + ix].BoneIndices[1] = _Model_Bird->Meshes.at(meshIdx).SkinningVertices.at(subIdx).SkinningVertices.at(ix).boneIndex[1];
				vertices[k + ix].BoneIndices[2] = _Model_Bird->Meshes.at(meshIdx).SkinningVertices.at(subIdx).SkinningVertices.at(ix).boneIndex[2];
				vertices[k + ix].BoneIndices[3] = _Model_Bird->Meshes.at(meshIdx).SkinningVertices.at(subIdx).SkinningVertices.at(ix).boneIndex[3];
			}

			k += (UINT)_Model_Bird->Meshes.at(meshIdx).Vertices.at(subIdx).Vertices.size();
		}
	}


	for (UINT meshIdx = 0, k = 0; meshIdx < BIRD_MESH_COUNT; ++meshIdx)
	{
		for (UINT subIdx = 0; subIdx < _Model_Bird->Meshes.at(meshIdx).SubMeshes.size(); ++subIdx)
		{
			for (int ix = 0; ix < _Model_Bird->Meshes.at(meshIdx).Indices.at(subIdx).Indices16.size(); ++ix)
			{
				indices[k + ix] = _Model_Bird->Meshes.at(meshIdx).Indices.at(subIdx).Indices16.at(ix);
			}

			k += (UINT)_Model_Bird->Meshes.at(meshIdx).Indices.at(subIdx).Indices16.size();
		}
	}

	const UINT vbByteSize = (UINT)vertices.size() * sizeof(SkinnedVertex);
	const UINT ibByteSize = (UINT)indices.size() * sizeof(std::uint16_t);

	auto geo = std::make_unique<MeshGeometry>();
	geo->Name = "BirdSkinningGeo";

	ThrowIfFailed(D3DCreateBlob(vbByteSize, &geo->VertexBufferCPU));
	CopyMemory(geo->VertexBufferCPU->GetBufferPointer(), vertices.data(), vbByteSize);

	ThrowIfFailed(D3DCreateBlob(ibByteSize, &geo->IndexBufferCPU));
	CopyMemory(geo->IndexBufferCPU->GetBufferPointer(), indices.data(), ibByteSize);

	geo->VertexBufferGPU = d3dUtil::CreateDefaultBuffer(devicePtr,
		commandListPtr, vertices.data(), vbByteSize, geo->VertexBufferUploader);

	geo->IndexBufferGPU = d3dUtil::CreateDefaultBuffer(devicePtr,
		commandListPtr, indices.data(), ibByteSize, geo->IndexBufferUploader);

	geo->VertexByteStride = sizeof(SkinnedVertex);
	geo->VertexBufferByteSize = vbByteSize;
	geo->IndexFormat = DXGI_FORMAT_R16_UINT;
	geo->IndexBufferByteSize = ibByteSize;

	for (UINT meshIdx = 0, k = 0, j = 0; meshIdx < BIRD_MESH_COUNT; ++meshIdx)
	{
		for (UINT subIdx = 0; subIdx < _Model_Bird->Meshes.at(meshIdx).SubMeshes.size(); ++subIdx)
		{
			std::string name = "Bird_" + std::to_string(meshIdx) + "_" + std::to_string(subIdx);

			SubmeshGeometry submesh;
			submesh.IndexCount = (UINT)_Model_Bird->Meshes.at(meshIdx).Indices.at(subIdx).Indices16.size();
			submesh.StartIndexLocation = j;
			submesh.BaseVertexLocation = k;

			geo->DrawArgs[name.c_str()] = submesh;

			k += (UINT)_Model_Bird->Meshes.at(meshIdx).Vertices.at(subIdx).Vertices.size();
			j += (UINT)_Model_Bird->Meshes.at(meshIdx).Indices.at(subIdx).Indices16.size();
		}
	}

	geometries[geo->Name] = std::move(geo);


	for (UINT k = 0; k < 4; ++k)
	{
		_BirdSkinned[k]._SkinnedModelInst = std::make_unique<SkinnedModelInstance>();
		_BirdSkinned[k]._SkinnedModelInst->FinalTransforms.resize(BONE_COUNT);
		_BirdSkinned[k]._SkinnedModelInst->FinalTransformsInv.resize(BONE_COUNT);
	}
}

void Bird::BuildStonsRenderItems(UINT idx, int x0, int z0,
	std::unordered_map<std::string, std::unique_ptr<MeshGeometry>>& geometries,
	std::unordered_map<std::string, std::unique_ptr<Material>>& materials,
	std::vector<std::unique_ptr<RenderItem>>& allRitems,
	std::vector<RenderItem*>& skinnedRenderItems)
{
	for (UINT meshIdx = 0; meshIdx < BIRD_MESH_COUNT; ++meshIdx)
	{
		for (UINT subIdx = 0; subIdx < _Model_Bird->Meshes.at(meshIdx).SubMeshes.size(); ++subIdx)
		{
			std::string name = "Bird_" + std::to_string(meshIdx) + "_" + std::to_string(subIdx);

			auto birdRitem = std::make_unique<RenderItem>();
			_stones_p2[idx]._rItem.push_back(birdRitem.get());
			XMStoreFloat4x4(&birdRitem->World, XMLoadFloat4x4(&_InitialWorld) *
				DirectX::XMMatrixTranslation(1.0f * x0, 0.2f, 1.0f * z0));
			XMStoreFloat4x4(&birdRitem->TexTransform, DirectX::XMMatrixScaling(1.0f, 1.0f, 1.0f));
			birdRitem->ObjCBIndex = g_ObjCBIndex++;
			birdRitem->Geo = geometries["BirdSkinningGeo"].get();
			birdRitem->Mat = materials["bird0"].get();
			birdRitem->PrimitiveType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
			birdRitem->IndexCount = birdRitem->Geo->DrawArgs[name.c_str()].IndexCount;
			birdRitem->StartIndexLocation = birdRitem->Geo->DrawArgs[name.c_str()].StartIndexLocation;
			birdRitem->BaseVertexLocation = birdRitem->Geo->DrawArgs[name.c_str()].BaseVertexLocation;
			birdRitem->Bounds = birdRitem->Geo->DrawArgs[name.c_str()].Bounds;

			birdRitem->SkinnedCBIndex = MONKEY_MESH_COUNT * 4 + meshIdx + (BIRD_MESH_COUNT * idx);
			birdRitem->SkinnedModelInst = _BirdSkinned[idx]._SkinnedModelInst.get();

			skinnedRenderItems.push_back(birdRitem.get());
			allRitems.push_back(std::move(birdRitem));
		}
	}
}

void Bird::BuildRenderItems(std::unordered_map<std::string, std::unique_ptr<MeshGeometry>>& geometries,
	std::unordered_map<std::string, std::unique_ptr<Material>>& materials,
	std::vector<std::unique_ptr<RenderItem>>& allRitems,
	std::vector<RenderItem*>& skinnedRenderItems)
{
	BuildStonsRenderItems(0, -5, 5, geometries, materials, allRitems, skinnedRenderItems);
	BuildStonsRenderItems(1, -5, 4, geometries, materials, allRitems, skinnedRenderItems);
	BuildStonsRenderItems(2, -4, 5, geometries, materials, allRitems, skinnedRenderItems);
	BuildStonsRenderItems(3, -4, 4, geometries, materials, allRitems, skinnedRenderItems);
}

void Bird::ResetSkinnedTransform(UINT idx, const GameTimer& gt)
{
	_BirdSkinned[idx]._SkinnedModelInst->ClipIdx = 0;
	//_BirdSkinned[idx]._SkinnedModelInst->ClipName = "h_A_post_bake";
	_BirdSkinned[idx]._SkinnedModelInst->TimePos = 0.0f;

	for (UINT ix = 0; ix < BIRD_MESH_COUNT; ++ix)
	{
		UINT numBones = (UINT)_Model_Bird->Meshes.at(ix).BonesInfo.size();

		_BirdSkinned[idx]._ToParentTransforms[ix].resize(numBones);
		_BirdSkinned[idx]._ToRootTransforms[ix].resize(numBones);

		for (UINT i = 0; i < numBones; ++i)
		{
			XMMATRIX m = XMLoadFloat4x4(&_Model_Bird->Meshes.at(ix).BonesInfo.at(i).bone.LocalTransform);
			XMStoreFloat4x4(&_BirdSkinned[idx]._ToParentTransforms[ix][i], m);
		}
	}
}

void Bird::InterpolateAnimationTransform(UINT idx, const GameTimer& gt)
{
	_BirdSkinned[idx]._SkinnedModelInst->TimePos += gt.DeltaTime();

	for (UINT meshIdx = 0; meshIdx < BIRD_MESH_COUNT; ++meshIdx)
	{
		Model::Clip clip = _Model_Bird->Meshes.at(meshIdx).ClipsInfo.at(_BirdSkinned[idx]._SkinnedModelInst->ClipIdx).clip;

		if (_BirdSkinned[idx]._SkinnedModelInst->TimePos > clip.EndTime)
			_BirdSkinned[idx]._SkinnedModelInst->TimePos = 0;

		if (_BirdSkinned[idx]._SkinnedModelInst->TimePos <=
			_Model_Bird->Meshes.at(meshIdx).ClipsInfo.at(_BirdSkinned[idx]._SkinnedModelInst->ClipIdx).Keyframes.front().Time)
		{
			UINT boneIndex =
				_Model_Bird->Meshes.at(meshIdx).ClipsInfo.at(_BirdSkinned[idx]._SkinnedModelInst->ClipIdx).Keyframes.front().BoneIndex;
			_BirdSkinned[idx]._ToParentTransforms[meshIdx][boneIndex] =
				_Model_Bird->Meshes.at(meshIdx).ClipsInfo.at(_BirdSkinned[idx]._SkinnedModelInst->ClipIdx).Keyframes.front().Transform;
		}
		else if (_BirdSkinned[idx]._SkinnedModelInst->TimePos >=
			_Model_Bird->Meshes.at(meshIdx).ClipsInfo.at(_BirdSkinned[idx]._SkinnedModelInst->ClipIdx).Keyframes.back().Time)
		{
			UINT boneIndex =
				_Model_Bird->Meshes.at(meshIdx).ClipsInfo.at(_BirdSkinned[idx]._SkinnedModelInst->ClipIdx).Keyframes.back().BoneIndex;
			_BirdSkinned[idx]._ToParentTransforms[meshIdx][boneIndex] =
				_Model_Bird->Meshes.at(meshIdx).ClipsInfo.at(_BirdSkinned[idx]._SkinnedModelInst->ClipIdx).Keyframes.back().Transform;
		}
		else
		{
			for (UINT i = 0;
				i < _Model_Bird->Meshes.at(meshIdx).ClipsInfo.at(_BirdSkinned[idx]._SkinnedModelInst->ClipIdx).Keyframes.size();
				++i)
			{
				if (_BirdSkinned[idx]._SkinnedModelInst->TimePos >=
					_Model_Bird->Meshes.at(meshIdx).ClipsInfo.at(_BirdSkinned[idx]._SkinnedModelInst->ClipIdx).Keyframes.at(i).Time)
				{
					UINT boneIndex =
						_Model_Bird->Meshes.at(meshIdx).ClipsInfo.at(_BirdSkinned[idx]._SkinnedModelInst->ClipIdx).Keyframes.at(i).BoneIndex;
					_BirdSkinned[idx]._ToParentTransforms[meshIdx][boneIndex] =
						_Model_Bird->Meshes.at(meshIdx).ClipsInfo.at(_BirdSkinned[idx]._SkinnedModelInst->ClipIdx).Keyframes.at(i).Transform;
				}
			}
		}
	}
}

void Bird::ProcessTransform(UINT idx, const GameTimer& gt)
{
	for (UINT meshIdx = 0; meshIdx < BIRD_MESH_COUNT; ++meshIdx)
	{
		UINT numBones = (UINT)_Model_Bird->Meshes.at(meshIdx).BonesInfo.size();
		UINT rootIndex = 0;

		for (UINT i = 0; i < numBones; ++i)
		{
			if (_Model_Bird->Meshes.at(meshIdx).BonesInfo.at(i).bone.ParentIndex < 0)
			{
				rootIndex = i;
				break;
			}
		}

		_BirdSkinned[idx]._ToRootTransforms[meshIdx][rootIndex] = _BirdSkinned[idx]._ToParentTransforms[meshIdx][rootIndex];
	}

	SetRootTransforms(idx);

	for (UINT meshIdx = 0; meshIdx < BIRD_MESH_COUNT; ++meshIdx)
	{
		UINT numBones = (UINT)_Model_Bird->Meshes.at(meshIdx).BonesInfo.size();

		for (UINT i = 0; i < numBones; ++i)
		{
			XMMATRIX offset = XMLoadFloat4x4(&_Model_Bird->Meshes.at(meshIdx).BonesInfo.at(i).bone.InvBindPos);
			XMMATRIX toRoot = XMLoadFloat4x4(&_BirdSkinned[idx]._ToRootTransforms[meshIdx][i]);
			XMMATRIX finalTransform = (offset * toRoot);

			XMStoreFloat4x4(&_BirdSkinned[idx]._SkinnedModelInst->FinalTransforms[i], (finalTransform));
		}

		for (UINT i = 0; i < numBones; ++i)
		{
			XMMATRIX finalTransform = XMLoadFloat4x4(&_BirdSkinned[idx]._SkinnedModelInst->FinalTransforms[i]);
			XMStoreFloat4x4(&_BirdSkinned[idx]._SkinnedModelInst->FinalTransformsInv[i], XMMatrixTranspose(finalTransform));
		}
	}
}

void Bird::UpdateSkinnedCBs(FrameResource* CurrFrameResource, UINT idx, const GameTimer& gt)
{
	auto currSkinnedCB = CurrFrameResource->SkinnedCB.get();

	for (UINT meshIdx = 0; meshIdx < BIRD_MESH_COUNT; ++meshIdx)
	{
		SkinnedConstants skinnedConstants = {};
		std::copy(
			std::begin(_BirdSkinned[idx]._SkinnedModelInst->FinalTransformsInv),
			std::end(_BirdSkinned[idx]._SkinnedModelInst->FinalTransformsInv),
			&skinnedConstants.BoneTransforms[0]);

		currSkinnedCB->CopyData(MONKEY_MESH_COUNT * 4 +
			meshIdx + (BIRD_MESH_COUNT * idx), skinnedConstants);
	}
}

void Bird::SetRootTransforms(UINT idx)
{
	for (UINT meshIdx = 0; meshIdx < BIRD_MESH_COUNT; ++meshIdx)
	{
		UINT numBones = (UINT)_Model_Bird->Meshes.at(meshIdx).BonesInfo.size();
		UINT rootIndex = 0;

		for (UINT i = 0; i < numBones; ++i)
		{
			if (_Model_Bird->Meshes.at(meshIdx).BonesInfo.at(i).bone.ParentIndex < 0)
			{
				rootIndex = i;
				break;
			}
		}

		SetRootTransforms(idx, numBones, meshIdx, rootIndex);
	}
}

void Bird::SetRootTransforms(UINT idx, const int numBones, const int meshIdx, int rootIndex)
{
	int j = 0;
	for (; j < numBones; ++j)
	{
		int parentIndex = _Model_Bird->Meshes.at(meshIdx).BonesInfo.at(j).bone.ParentIndex;
		if (parentIndex == rootIndex)
		{

			{
				XMMATRIX toParent = XMLoadFloat4x4(&_BirdSkinned[idx]._ToParentTransforms[meshIdx][j]);
				int parentIndex = _Model_Bird->Meshes.at(meshIdx).BonesInfo.at(j).bone.ParentIndex;
				XMMATRIX parentToRoot = XMLoadFloat4x4(&_BirdSkinned[idx]._ToRootTransforms[meshIdx][parentIndex]);
				XMMATRIX toRoot = XMMatrixMultiply(toParent, parentToRoot);

				XMStoreFloat4x4(&_BirdSkinned[idx]._ToRootTransforms[meshIdx][j], toRoot);
			}
			SetRootTransforms(idx, numBones, meshIdx, j);
		}
	}
}

void Bird::UpdateStoneP2(UINT idx, StoneP2* stone, const GameTimer& gt)
{
	if (!stone->_need_update)
		return;

	float step = 5.0f * gt.DeltaTime();

	for (UINT ix = 0; ix < stone->_rItem.size(); ++ix)
	{
		DirectX::XMFLOAT4X4 w = stone->_rItem[ix]->World;

		DirectX::XMVECTOR v1 = DirectX::XMVectorSet(w._41, 0.2f, w._43, 0.0f);
		DirectX::XMVECTOR v2 = DirectX::XMVectorSet(1.0f * stone->_x, 0.2f, 1.0f * stone->_z, 0.0f);
		DirectX::XMVECTOR v = v2 - v1;
		DirectX::XMVECTOR n = DirectX::XMVector4Normalize(v);
		DirectX::XMVECTOR t = v1 + step * n;
		DirectX::XMVECTOR epsilon = DirectX::XMVectorSet(step, step, step, step);

		XMVECTOR Z = XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f);
		float angle = XMVectorGetX(XMVector4AngleBetweenNormals(-Z, n));

		if (XMVectorGetY(XMVector4Normalize(XMVector3Cross(-Z, n))) < 0)
			angle = -angle;

		if (!XMVector4NearEqual(t, v2, epsilon))
		{
			XMStoreFloat4x4(&stone->_rItem[ix]->World, XMLoadFloat4x4(&_InitialWorld) * XMMatrixRotationY(angle) *
				DirectX::XMMatrixTranslation(DirectX::XMVectorGetX(t), 0.2f, DirectX::XMVectorGetZ(t)));

			stone->_rItem[ix]->NumFramesDirty = gNumFrameResources;
		}
		else
		{
			t = v2;

			XMStoreFloat4x4(&stone->_rItem[ix]->World, XMLoadFloat4x4(&_InitialWorld) *
				DirectX::XMMatrixTranslation(DirectX::XMVectorGetX(t), 0.2f, DirectX::XMVectorGetZ(t)));

			stone->_rItem[ix]->NumFramesDirty = gNumFrameResources;

			stone->_need_update = false;
			ResetSkinnedTransform(idx, gt);
			ProcessTransform(idx, gt);
		}
	}
}

void Bird::UpdateStones(const GameTimer& gt)
{
	for (int ix = 0; ix < 4; ++ix)
	{
		UpdateStoneP2(ix, &_stones_p2[ix], gt);
	}
}

void Bird::Update(AbstractWindow* wnd, FrameResource* CurrFrameResource, const GameTimer& gt)
{
	UpdateStones(gt);

	for (UINT ix = 0; ix < 4; ++ix)
	{
		if (!_stones_p2[ix]._initialized)
		{
			ResetSkinnedTransform(ix, gt);
			ProcessTransform(ix, gt);

			_stones_p2[ix]._initialized = true;
		}
		else if (_stones_p2[ix]._need_update)
		{
			InterpolateAnimationTransform(ix, gt);
			ProcessTransform(ix, gt);
		}

		UpdateSkinnedCBs(CurrFrameResource, ix, gt);
	}
}
