#ifndef _FIXED_H_
#define _FIXED_H_

class Fixed
{
public:
	Fixed() = default;

	void BuildGeometry(ID3D12Device* devicePtr,
		ID3D12GraphicsCommandList* commandListPtr,
		std::unordered_map<std::string, std::unique_ptr<MeshGeometry>>& geometries);

	void BuildRenderItems(std::unordered_map<std::string, std::unique_ptr<MeshGeometry>>& geometries,
		std::unordered_map<std::string, std::unique_ptr<Material>>& materials,
		std::vector<std::unique_ptr<RenderItem>>& allRitems,
		std::vector<RenderItem*>& fixedRenderItems);

	RenderItem* _upButton = nullptr;
	RenderItem* _downButton = nullptr;
	RenderItem* _leftButton = nullptr;
	RenderItem* _rightButton = nullptr;
	RenderItem* _zoominButton = nullptr;
	RenderItem* _zoomoutButton = nullptr;

	RenderItem* _newButton = nullptr;
};


#endif /* _FIXED_H_ */
