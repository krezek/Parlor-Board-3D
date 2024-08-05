#ifndef _WHEEL_H_
#define _WHEEL_H_

#include <AbstractWindow.h>
#include <GameTimer.h>

class Wheel
{
public:
	Wheel();

	DirectX::XMFLOAT4X4 _InitialWorld;

	RenderItem* _WheelRItem = nullptr;
	bool _visible = true;

	void BuildGeometry(ID3D12Device* devicePtr,
		ID3D12GraphicsCommandList* commandListPtr,
		std::unordered_map<std::string, std::unique_ptr<MeshGeometry>>& geometries);

	void BuildRenderItems(std::unordered_map<std::string, std::unique_ptr<MeshGeometry>>& geometries,
		std::unordered_map<std::string, std::unique_ptr<Material>>& materials,
		std::vector<std::unique_ptr<RenderItem>>& allRitems,
		std::vector<RenderItem*>& fixedRenderItems);

	void Update(AbstractWindow* wnd, const GameTimer& gt);
};

#endif /* _WHEEL_H_ */
