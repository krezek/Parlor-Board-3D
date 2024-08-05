#ifndef _DICE_H_
#define _DICE_H_

#include <AbstractWindow.h>
#include <GameTimer.h>

struct DiceCube
{
	RenderItem* _dice1;
	RenderItem* _dice2;
	RenderItem* _dice3;
	RenderItem* _dice4;
	RenderItem* _dice5;
	RenderItem* _dice6;

	float _ax = 0, _ay = 0, _az = 0, _at = 0;
	float _axMax = 0, _ayMax = 0, _azMax = 0, _atMax = 0;
	bool _need_update = false;
};

class Dice
{
public:
	Dice() = default;

	DiceCube _dice_cube_p1, _dice_cube_p2;

	void BuildGeometry(ID3D12Device* devicePtr,
		ID3D12GraphicsCommandList* commandListPtr,
		std::unordered_map<std::string, std::unique_ptr<MeshGeometry>>& geometries);

	void BuildRenderItems(std::unordered_map<std::string, std::unique_ptr<MeshGeometry>>& geometries,
		std::unordered_map<std::string, std::unique_ptr<Material>>& materials,
		std::vector<std::unique_ptr<RenderItem>>& allRitems,
		std::vector<RenderItem*>& opaqueRenderItems);

	void Update(AbstractWindow* wnd, const GameTimer& gt);

protected:
	void BuildRenderItems_p1(std::unordered_map<std::string, std::unique_ptr<MeshGeometry>>& geometries,
		std::unordered_map<std::string, std::unique_ptr<Material>>& materials,
		std::vector<std::unique_ptr<RenderItem>>& allRitems,
		std::vector<RenderItem*>& opaqueRenderItems);
	void BuildRenderItems_p2(std::unordered_map<std::string, std::unique_ptr<MeshGeometry>>& geometries,
		std::unordered_map<std::string, std::unique_ptr<Material>>& materials,
		std::vector<std::unique_ptr<RenderItem>>& allRitems,
		std::vector<RenderItem*>& opaqueRenderItems);

	void UpdateP1Cube(AbstractWindow* wnd, const GameTimer& gt);
	void UpdateP2Cube(AbstractWindow* wnd, const GameTimer& gt);
};


#endif /* _DICE_H_ */
