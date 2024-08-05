#ifndef _BIRD_H_
#define _BIRD_H_

#include <AbstractWindow.h>
#include <GameTimer.h>
#include <Model.h>
#include <FrameResource.h>
#include <RenderItem.h>

struct StoneP2
{
	std::vector<RenderItem*> _rItem;
	int _x, _z;
	bool _initialized = false;
	bool _need_update = false;
};

struct BirdSkinned
{
	std::unique_ptr<SkinnedModelInstance> _SkinnedModelInst;

	std::vector<DirectX::XMFLOAT4X4> _ToParentTransforms[BIRD_MESH_COUNT];
	std::vector<DirectX::XMFLOAT4X4> _ToRootTransforms[BIRD_MESH_COUNT];
};

class Bird
{
public:
	Bird();

	DirectX::XMFLOAT4X4 _InitialWorld;

	StoneP2 _stones_p2[4];
	BirdSkinned _BirdSkinned[4];

	void LoadModel();
	void BuildSkinnedModel(ID3D12Device* devicePtr,
		ID3D12GraphicsCommandList* commandListPtr,
		std::unordered_map<std::string, std::unique_ptr<MeshGeometry>>& geometries);

	void BuildStonsRenderItems(UINT idx, int x0, int z0,
		std::unordered_map<std::string, std::unique_ptr<MeshGeometry>>& geometries,
		std::unordered_map<std::string, std::unique_ptr<Material>>& materials,
		std::vector<std::unique_ptr<RenderItem>>& allRitems,
		std::vector<RenderItem*>& skinnedRenderItems);

	void BuildRenderItems(std::unordered_map<std::string, std::unique_ptr<MeshGeometry>>& geometries,
		std::unordered_map<std::string, std::unique_ptr<Material>>& materials,
		std::vector<std::unique_ptr<RenderItem>>& allRitems,
		std::vector<RenderItem*>& skinnedRenderItems);

	void ResetSkinnedTransform(UINT idx, const GameTimer& gt);
	void InterpolateAnimationTransform(UINT idx, const GameTimer& gt);
	void ProcessTransform(UINT idx, const GameTimer& gt);

	void UpdateSkinnedCBs(FrameResource* CurrFrameResource, UINT idx, const GameTimer& gt);

	void UpdateStoneP2(UINT idx, StoneP2* stone, const GameTimer& gt);
	void UpdateStones(const GameTimer& gt);

	void Update(AbstractWindow* wnd, FrameResource* CurrFrameResource, const GameTimer& gt);

protected:
	std::unique_ptr<Model> _Model_Bird;

	void SetRootTransforms(UINT idx);
	void SetRootTransforms(UINT idx, const int numBones, const int meshIdx, int rootIndex);
};


#endif /* _BIRD_H_ */
