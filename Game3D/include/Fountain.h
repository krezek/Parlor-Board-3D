#ifndef _FOUNTAIN_H_
#define _FOUNTAIN_H_

#include <AbstractWindow.h>
#include <GameTimer.h>
#include <Model.h>
#include <FrameResource.h>
#include <RenderItem.h>

class Fountain
{
public:
	Fountain();

	DirectX::XMFLOAT4X4 _InitialWorld;

	void LoadModel();
	void BuildGeometry(ID3D12Device* devicePtr,
		ID3D12GraphicsCommandList* commandListPtr,
		std::unordered_map<std::string, std::unique_ptr<MeshGeometry>>& geometries);

	void BuildRenderItems(std::unordered_map<std::string, std::unique_ptr<MeshGeometry>>& geometries,
		std::unordered_map<std::string, std::unique_ptr<Material>>& materials,
		std::vector<std::unique_ptr<RenderItem>>& allRitems,
		std::vector<RenderItem*>& skinnedRenderItems);

	void Update(AbstractWindow* wnd, FrameResource* CurrFrameResource, const GameTimer& gt);

protected:
	std::unique_ptr<Model> _Model_Fountain;
};


#endif /* _FOUNTAIN_H_ */
