#ifndef _MONKEY_H_
#define _MONKEY_H_

#include <AbstractWindow.h>
#include <GameTimer.h>
#include <Model.h>
#include <FrameResource.h>
#include <RenderItem.h>

struct StoneP1
{
	std::vector<RenderItem*> _rItem;
	int _x, _z;
	bool _initialized = false;
	bool _need_update = false;
};

struct MonkeySkinned
{
	std::unique_ptr<SkinnedModelInstance> _SkinnedModelInst;

	std::vector<DirectX::XMFLOAT4X4> _ToParentTransforms[MONKEY_MESH_COUNT];
	std::vector<DirectX::XMFLOAT4X4> _ToRootTransforms[MONKEY_MESH_COUNT];
};

class Monkey
{
public:
	Monkey();

	DirectX::XMFLOAT4X4 _InitialWorld;

	StoneP1 _stones_p1[4];
	MonkeySkinned _MonkeySkinned[4];

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

	void UpdateStoneP1(UINT idx, StoneP1* stone, const GameTimer& gt);
	void UpdateStones(const GameTimer& gt);

	void Update(AbstractWindow* wnd, FrameResource* CurrFrameResource, const GameTimer& gt);

protected:
	std::unique_ptr<Model> _Model_Monkey;

	void SetRootTransforms(UINT idx);
	void SetRootTransforms(UINT idx, const int numBones, const int meshIdx, int rootIndex);
};


#endif /* _MONKEY_H_ */
