#ifndef _MODEL_H_
#define _MODEL_H_

class Model
{
public:
	struct Material
    {
        DirectX::XMFLOAT4   Ambient;
        DirectX::XMFLOAT4   Diffuse;
        DirectX::XMFLOAT4   Specular;
        float               SpecularPower;
        DirectX::XMFLOAT4   Emissive;
        DirectX::XMFLOAT4X4 UVTransform;
    };

    static const uint32_t MAX_TEXTURE = 8;

    struct SubMesh
    {
        UINT MaterialIndex;
        UINT IndexBufferIndex;
        UINT VertexBufferIndex;
        UINT StartIndex;
        UINT PrimCount;
    };

    static const uint32_t NUM_BONE_INFLUENCES = 4;

    struct Vertex
    {
        DirectX::XMFLOAT3 Position;
        DirectX::XMFLOAT3 Normal;
        DirectX::XMFLOAT4 Tangent;
        UINT color;
        DirectX::XMFLOAT2 TextureCoordinates;
    };

    struct SkinningVertex
    {
        UINT boneIndex[NUM_BONE_INFLUENCES];
        float boneWeight[NUM_BONE_INFLUENCES];
    };

    struct MeshExtents
    {
        float CenterX, CenterY, CenterZ;
        float Radius;

        float MinX, MinY, MinZ;
        float MaxX, MaxY, MaxZ;
    };

    struct Bone
    {
        INT ParentIndex;
        DirectX::XMFLOAT4X4 InvBindPos;
        DirectX::XMFLOAT4X4 BindPos;
        DirectX::XMFLOAT4X4 LocalTransform;
    };

    struct Clip
    {
        float StartTime;
        float EndTime;
        UINT  keys;
    };

    struct Keyframe
    {
        UINT BoneIndex;
        float Time;
        DirectX::XMFLOAT4X4 Transform;
    };

    struct Texture
    {
        std::wstring Name;
    };

    struct MaterialInfo
    {
        std::wstring Name;
        std::wstring PSName;
        std::wstring Root;
        Material Mat;
        std::vector<Texture> Textures;
    };

    struct IndexBuffer
    {
        UINT Count;
        std::vector<uint16_t> Indices16;
    };

    struct VertexBuffer
    {
        UINT Count;
        std::vector<Vertex> Vertices;
    };

    struct SkinningVertexBuffer
    {
        UINT Count;
        std::vector<SkinningVertex> SkinningVertices;
    };

    struct BoneInfo
    {
        std::wstring Name;
        Bone bone;
    };

    struct ClipInfo
    {
        std::wstring Name;
        Clip clip;
        std::vector<Keyframe> Keyframes;
    };
    
    struct Mesh
    {
        std::wstring Name;
        std::vector<MaterialInfo> MaterialsInfo;
        std::vector<SubMesh> SubMeshes;
        std::vector<IndexBuffer> Indices;
        std::vector<VertexBuffer> Vertices;
        std::vector<SkinningVertexBuffer> SkinningVertices;
        MeshExtents extents;
        std::vector<BoneInfo> BonesInfo;
        std::vector<ClipInfo> ClipsInfo;
    };

    std::vector<Mesh> Meshes;

    Model() = default;
    ~Model() = default;

    int LoadCMOFile(const wchar_t* filename);

private:
	HRESULT LoadDataFromFile(_In_z_ const wchar_t* fileName,
		std::unique_ptr<uint8_t[]>& data,
		size_t& dataSize);
	void DumpVB(VertexBuffer& vbL, UINT id, const Model::Vertex* vb, size_t count, bool full);
    void DumpSkinVB(SkinningVertexBuffer& svbL, UINT id, const Model::SkinningVertex* vb, size_t count, bool full);
    template<typename index_t>
    void DumpIB(IndexBuffer& ibL, UINT id, const index_t* ib, size_t count, bool full);
};


#endif /* _MODEL_H_ */
