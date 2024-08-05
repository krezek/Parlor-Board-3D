#include "platform.h"

#define CGLTF_IMPLEMENTATION
#include "cgltf.h"

using namespace DirectX;

struct Material
{
    DirectX::XMFLOAT4   Ambient;
    DirectX::XMFLOAT4   Diffuse;
    DirectX::XMFLOAT4   Specular;
    float               SpecularPower;
    DirectX::XMFLOAT4   Emissive;
    DirectX::XMFLOAT4X4 UVTransform;
};

const uint32_t MAX_TEXTURE = 8;

struct SubMesh
{
    UINT MaterialIndex;
    UINT IndexBufferIndex;
    UINT VertexBufferIndex;
    UINT StartIndex;
    UINT PrimCount;
};

const uint32_t NUM_BONE_INFLUENCES = 4;

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

static_assert(sizeof(Material) == 132, "CMO Mesh structure size incorrect");
static_assert(sizeof(SubMesh) == 20, "CMO Mesh structure size incorrect");
static_assert(sizeof(Vertex) == 52, "CMO Mesh structure size incorrect");
static_assert(sizeof(SkinningVertex) == 32, "CMO Mesh structure size incorrect");
static_assert(sizeof(MeshExtents) == 40, "CMO Mesh structure size incorrect");
static_assert(sizeof(Bone) == 196, "CMO Mesh structure size incorrect");
static_assert(sizeof(Clip) == 12, "CMO Mesh structure size incorrect");
static_assert(sizeof(Keyframe) == 72, "CMO Mesh structure size incorrect");

void write_data(FILE* out, FILE* bin, cgltf_data* data);
void write_submeshes(FILE* out, FILE* bin, cgltf_data* data, cgltf_size mesh);
void write_index_buffers(FILE* out, FILE* bin, cgltf_data* data, cgltf_size mesh);
void write_vertex_buffers(FILE* out, FILE* bin, cgltf_data* data, cgltf_size mesh,BYTE bSkeleton);
void write_animation_data(FILE* out, FILE* bin, cgltf_data* data, cgltf_size mesh, BYTE bSkeleton);
void write_animation_clips(FILE* out, FILE* bin, cgltf_data* data, cgltf_size mesh, BYTE bSkeleton);


int main(int argc, char* argv[])
{
#ifdef _DEBUG
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

	const char* fname = "Dog";
	char ifilename[255], ofilename[255], bfilename[255];
	
	strcpy(ifilename, "Models\\Dog\\");
	strcat(ifilename, fname);
	strcat(ifilename, ".gltf");

	strcpy(ofilename, "Models\\Dog\\");
	strcat(ofilename, fname);
	strcat(ofilename, ".cmo");

    strcpy(bfilename, "Models\\Dog\\");
    strcat(bfilename, fname);
    strcat(bfilename, ".bin");

	cgltf_options options = { 0 };
	cgltf_data* data = NULL;
	cgltf_result result = cgltf_parse_file(&options, ifilename , &data);
	if (result == cgltf_result_success)
	{
        FILE *out, *bin;
		
        out = fopen(ofilename, "wb");
		if (out == NULL)
        {
            cgltf_free(data);
            return -1;
        }

        bin = fopen(bfilename, "rb");
        if (bin == NULL)
        {
            cgltf_free(data);
            return -1;
        }

        write_data(out, bin, data);

        fclose(bin);
        fclose(out);
	}

	return 0;
}

void write_data(FILE* out, FILE* bin, cgltf_data* data)
{
    size_t nb;

    UINT nMesh = static_cast<UINT>(data->meshes_count);
    nb = fwrite(&nMesh, sizeof(UINT), 1, out);
    assert(nb);

    for (cgltf_size mesh = 0; mesh < data->meshes_count; ++mesh)
    {
        UINT nName = static_cast<UINT>(strlen(data->meshes[mesh].name) + 1);
        nb = fwrite(&nName, sizeof(UINT), 1, out);
        assert(nb);

        wchar_t buf[1024];
        wsprintf(buf, L"%S", data->meshes[mesh].name);
        nb = fwrite(buf, sizeof(wchar_t), nName, out);
        assert(nb);

        UINT nMats = static_cast<UINT>(0); // No materials
        nb = fwrite(&nMats, sizeof(UINT), 1, out);
        assert(nb);

        BYTE bSkeleton = 1; // there is no skeletal animation data present
        nb = fwrite(&bSkeleton, sizeof(BYTE), 1, out);
        assert(nb);

        write_submeshes(out, bin, data, mesh);
        write_index_buffers(out, bin, data, mesh);
        write_vertex_buffers(out, bin, data, mesh, bSkeleton);
        write_animation_clips(out, bin, data, mesh, bSkeleton);
    }
}

void write_submeshes(FILE* out, FILE* bin, cgltf_data* data, cgltf_size mesh)
{
    size_t nb;

    // Submeshes

    UINT nSubmesh = 1;
    nb = fwrite(&nSubmesh, sizeof(UINT), 1, out);
    assert(nb);

    SubMesh subMesh = { 0 };
    nb = fwrite(&subMesh, sizeof(SubMesh), 1, out);
    assert(nb);
}

void write_index_buffers(FILE* out, FILE* bin, cgltf_data* data, cgltf_size mesh)
{
    size_t nb;

    // Index buffers
    
    UINT nIBs = static_cast<UINT>(data->meshes[mesh].primitives_count);
    nb = fwrite(&nIBs, sizeof(UINT), 1, out);
    assert(nb);

    for (cgltf_size primitives_idx = 0; primitives_idx < data->meshes[mesh].primitives_count; ++primitives_idx)
    {
        cgltf_accessor* accessor = data->meshes[mesh].primitives[primitives_idx].indices;
        cgltf_buffer_view* buffer_view = accessor->buffer_view;
        cgltf_component_type component_type = accessor->component_type;

        fseek(bin, (long)(accessor->offset + buffer_view->offset), SEEK_SET);

        switch (component_type)
        {
        case cgltf_component_type_r_32u: // 5125 UNSIGNED_INT
        {
            UINT nIndexes = static_cast<UINT>(accessor->count);
            nb = fwrite(&nIndexes, sizeof(UINT), 1, out);
            assert(nb);

            cgltf_type type = accessor->type;
            switch (type)
            {
            case cgltf_type_scalar:
            {
                for (cgltf_size idx = 0; idx < accessor->count; ++idx)
                {
                    UINT value;
                    fread(&value, sizeof(UINT), 1, bin);

                    uint16_t index = static_cast<uint16_t>(value);
                    nb = fwrite(&index, sizeof(uint16_t), 1, out);
                    assert(nb);
                }
            }
            break;
            default:
                fprintf(stderr, "Unsuported accessor type\n");
                return;
            }
        }
        break;
        default:
            fprintf(stderr, "Unsuported component type\n");
            return;
        }
    }
}

void write_vertex_buffers(FILE* out, FILE* bin, cgltf_data* data, cgltf_size mesh, BYTE bSkeleton)
{
    size_t nb;

    // Vertex buffers

    UINT nVBs = static_cast<UINT>(data->meshes[mesh].primitives_count);
    nb = fwrite(&nVBs, sizeof(UINT), 1, out);
    assert(nb);

    for (cgltf_size primitives_idx = 0; primitives_idx < data->meshes[mesh].primitives_count; ++primitives_idx)
    {
        UINT nVerts = static_cast<UINT>(data->meshes[mesh].primitives[primitives_idx].attributes[0].data->count);
        nb = fwrite(&nVerts, sizeof(UINT), 1, out);
        assert(nb);

        std::vector<DirectX::XMFLOAT3> position;
        std::vector<DirectX::XMFLOAT3> normal;
        std::vector<DirectX::XMFLOAT4> tangent;
        std::vector<DirectX::XMFLOAT2> texcoord;
        std::vector<DirectX::XMUINT4> joints;
        std::vector<DirectX::XMFLOAT4> weights;

        for (cgltf_size attr_idx = 0; attr_idx < data->meshes[mesh].primitives[primitives_idx].attributes_count; ++attr_idx)
        {
            cgltf_attribute attr = data->meshes[mesh].primitives[primitives_idx].attributes[attr_idx];

            switch (attr.type)
            {
            case cgltf_attribute_type_weights:
            {
                cgltf_accessor* accessor = data->meshes[mesh].primitives[primitives_idx].attributes[attr_idx].data;
                cgltf_buffer_view* buffer_view = accessor->buffer_view;
                cgltf_component_type component_type = accessor->component_type;

                fseek(bin, (long)(accessor->offset + buffer_view->offset), SEEK_SET);

                switch (component_type)
                {
                case cgltf_component_type_r_32f: // FLOAT
                {
                    cgltf_type type = accessor->type;
                    switch (type)
                    {
                    case cgltf_type_vec4:
                    {
                        for (cgltf_size idx = 0; idx < accessor->count; ++idx)
                        {
                            float v1, v2, v3, v4;
                            fread(&v1, sizeof(float), 1, bin);
                            fread(&v2, sizeof(float), 1, bin);
                            fread(&v3, sizeof(float), 1, bin);
                            fread(&v4, sizeof(float), 1, bin);

                            weights.push_back(DirectX::XMFLOAT4(v1, v2, v3, v4));
                        }
                    }
                    break;
                    default:
                        fprintf(stderr, "Unsuported accessor type\n");
                        return;
                    }
                }
                break;
                default:
                    fprintf(stderr, "Unsuported component type\n");
                    return;
                }
            }
            break;

            case cgltf_attribute_type_joints:
            {
                cgltf_accessor* accessor = data->meshes[mesh].primitives[primitives_idx].attributes[attr_idx].data;
                cgltf_buffer_view* buffer_view = accessor->buffer_view;
                cgltf_component_type component_type = accessor->component_type;

                fseek(bin, (long)(accessor->offset + buffer_view->offset), SEEK_SET);

                switch (component_type)
                {
                case cgltf_component_type_r_16u: // UNSIGNED_SHORT
                {
                    cgltf_type type = accessor->type;
                    switch (type)
                    {
                    case cgltf_type_vec4:
                    {
                        for (cgltf_size idx = 0; idx < accessor->count; ++idx)
                        {
                            unsigned short v1, v2, v3, v4;
                            fread(&v1, sizeof(unsigned short), 1, bin);
                            fread(&v2, sizeof(unsigned short), 1, bin);
                            fread(&v3, sizeof(unsigned short), 1, bin);
                            fread(&v4, sizeof(unsigned short), 1, bin);

                            joints.push_back(DirectX::XMUINT4(v1, v2, v3, v4));
                        }
                    }
                    break;
                    default:
                        fprintf(stderr, "Unsuported accessor type\n");
                        return;
                    }
                }
                break;
                default:
                    fprintf(stderr, "Unsuported component type\n");
                    return;
                }
            }
            break;

            case cgltf_attribute_type_texcoord:
            {
                cgltf_accessor* accessor = data->meshes[mesh].primitives[primitives_idx].attributes[attr_idx].data;
                cgltf_buffer_view* buffer_view = accessor->buffer_view;
                cgltf_component_type component_type = accessor->component_type;

                fseek(bin, (long)(accessor->offset + buffer_view->offset), SEEK_SET);

                switch (component_type)
                {
                case cgltf_component_type_r_32f: // 5126 FLOAT
                {
                    cgltf_type type = accessor->type;
                    switch (type)
                    {
                    case cgltf_type_vec2:
                    {
                        for (cgltf_size idx = 0; idx < accessor->count; ++idx)
                        {
                            float v1, v2;
                            fread(&v1, sizeof(float), 1, bin);
                            fread(&v2, sizeof(float), 1, bin);

                            texcoord.push_back(DirectX::XMFLOAT2(v1, v2));
                        }
                    }
                    break;
                    default:
                        fprintf(stderr, "Unsuported accessor type\n");
                        return;
                    }
                }
                break;
                default:
                    fprintf(stderr, "Unsuported component type\n");
                    return;
                }
            }
            break;

            case cgltf_attribute_type_tangent:
            {
                cgltf_accessor* accessor = data->meshes[mesh].primitives[primitives_idx].attributes[attr_idx].data;
                cgltf_buffer_view* buffer_view = accessor->buffer_view;
                cgltf_component_type component_type = accessor->component_type;

                fseek(bin, (long)(accessor->offset + buffer_view->offset), SEEK_SET);

                switch (component_type)
                {
                case cgltf_component_type_r_32f: // 5126 FLOAT
                {
                    cgltf_type type = accessor->type;
                    switch (type)
                    {
                    case cgltf_type_vec4:
                    {
                        for (cgltf_size idx = 0; idx < accessor->count; ++idx)
                        {
                            float v1, v2, v3, v4;
                            fread(&v1, sizeof(float), 1, bin);
                            fread(&v2, sizeof(float), 1, bin);
                            fread(&v3, sizeof(float), 1, bin);
                            fread(&v4, sizeof(float), 1, bin);

                            tangent.push_back(DirectX::XMFLOAT4(v1, v2, v3, v4));
                        }
                    }
                    break;
                    default:
                        fprintf(stderr, "Unsuported accessor type\n");
                        return;
                    }
                }
                break;
                default:
                    fprintf(stderr, "Unsuported component type\n");
                    return;
                }
            }
            break;

            case cgltf_attribute_type_normal:
            {
                cgltf_accessor* accessor = data->meshes[mesh].primitives[primitives_idx].attributes[attr_idx].data;
                cgltf_buffer_view* buffer_view = accessor->buffer_view;
                cgltf_component_type component_type = accessor->component_type;

                fseek(bin, (long)(accessor->offset + buffer_view->offset), SEEK_SET);

                switch (component_type)
                {
                case cgltf_component_type_r_32f: // 5126 FLOAT
                {
                    cgltf_type type = accessor->type;
                    switch (type)
                    {
                    case cgltf_type_vec3:
                    {
                        for (cgltf_size idx = 0; idx < accessor->count; ++idx)
                        {
                            float v1, v2, v3;
                            fread(&v1, sizeof(float), 1, bin);
                            fread(&v2, sizeof(float), 1, bin);
                            fread(&v3, sizeof(float), 1, bin);

                            normal.push_back(DirectX::XMFLOAT3(v1, v2, v3));
                        }
                    }
                    break;
                    default:
                        fprintf(stderr, "Unsuported accessor type\n");
                        return;
                    }
                }
                break;
                default:
                    fprintf(stderr, "Unsuported component type\n");
                    return;
                }
            }
            break;

            case cgltf_attribute_type_position:
            {
                cgltf_accessor* accessor = data->meshes[mesh].primitives[primitives_idx].attributes[attr_idx].data;
                cgltf_buffer_view* buffer_view = accessor->buffer_view;
                cgltf_component_type component_type = accessor->component_type;

                fseek(bin, (long)(accessor->offset + buffer_view->offset), SEEK_SET);

                switch (component_type)
                {
                case cgltf_component_type_r_32f: // 5126 FLOAT
                {
                    cgltf_type type = accessor->type;
                    switch (type)
                    {
                    case cgltf_type_vec3:
                    {
                        for (cgltf_size idx = 0; idx < accessor->count; ++idx)
                        {
                            float v1, v2, v3;
                            fread(&v1, sizeof(float), 1, bin);
                            fread(&v2, sizeof(float), 1, bin);
                            fread(&v3, sizeof(float), 1, bin);

                            position.push_back(DirectX::XMFLOAT3(v1, v2, v3));
                        }
                    }
                    break;
                    default:
                        fprintf(stderr, "Unsuported accessor type\n");
                        return;
                    }
                }
                break;
                default:
                    fprintf(stderr, "Unsuported component type\n");
                    return;
                }
            }
            break;

            }
        }

        for (UINT idx = 0; idx < nVerts; ++idx)
        {
            Vertex vertex = { position[idx],
            normal[idx],
            tangent[idx],
            0,
            texcoord[idx]
            };

            fwrite(&vertex, sizeof(Vertex), 1, out);
        }

        // Skinning vertex buffers
        UINT nSkinVBs = nVBs;
        nb = fwrite(&nSkinVBs, sizeof(UINT), 1, out);
        assert(nb);

        nb = fwrite(&nVerts, sizeof(UINT), 1, out);
        assert(nb);

        for (UINT idx = 0; idx < nVerts; ++idx)
        {
            SkinningVertex vertex = { joints[idx].x, joints[idx].y, joints[idx].z, joints[idx].w,
                weights[idx].x, weights[idx].y, weights[idx].z, weights[idx].w };

            fwrite(&vertex, sizeof(SkinningVertex), 1, out);
        }

        // Extents
        MeshExtents extents = { 0 };
        nb = fwrite(&extents, sizeof(MeshExtents), 1, out);
        assert(nb);

        write_animation_data(out, bin, data, mesh, bSkeleton);
    }
}

void write_animation_data(FILE* out, FILE* bin, cgltf_data* data, cgltf_size mesh, BYTE bSkeleton)
{
    size_t nb;

    // Animation data
    if (bSkeleton)
    {
        UINT nBones = static_cast<UINT>(data->skins->joints_count);
        nb = fwrite(&nBones, sizeof(UINT), 1, out);
        assert(nb);

        std::unordered_map<char*, INT> bmap;

        for (UINT j = 0; j < nBones; ++j)
        {
            bmap[data->skins->joints[j]->name] = -1;

            for (UINT k = 0; k < nBones; ++k)
            {
                for (cgltf_size iy = 0; iy < data->skins->joints[k]->children_count; ++iy)
                {
                    if (strcmp(data->skins->joints[k]->children[iy]->name, data->skins->joints[j]->name) == 0)
                        bmap[data->skins->joints[j]->name] = k;
                }
            }
        }

        std::vector<XMFLOAT4X4> InvBindPos;

        {
            cgltf_accessor* accessor = data->skins->inverse_bind_matrices;
            cgltf_buffer_view* buffer_view = accessor->buffer_view;
            cgltf_component_type component_type = accessor->component_type;

            fseek(bin, (long)(accessor->offset + buffer_view->offset), SEEK_SET);

            switch (component_type)
            {
            case cgltf_component_type_r_32f: // 5126 FLOAT
            {
                cgltf_type type = accessor->type;
                switch (type)
                {
                case cgltf_type_mat4:
                {
                    for (UINT j = 0; j < nBones; ++j)
                    {
                        float m00, m01, m02, m03,
                            m10, m11, m12, m13,
                            m20, m21, m22, m23,
                            m30, m31, m32, m33;
                        fread(&m00, sizeof(float), 1, bin);
                        fread(&m01, sizeof(float), 1, bin);
                        fread(&m02, sizeof(float), 1, bin);
                        fread(&m03, sizeof(float), 1, bin);

                        fread(&m10, sizeof(float), 1, bin);
                        fread(&m11, sizeof(float), 1, bin);
                        fread(&m12, sizeof(float), 1, bin);
                        fread(&m13, sizeof(float), 1, bin);

                        fread(&m20, sizeof(float), 1, bin);
                        fread(&m21, sizeof(float), 1, bin);
                        fread(&m22, sizeof(float), 1, bin);
                        fread(&m23, sizeof(float), 1, bin);

                        fread(&m30, sizeof(float), 1, bin);
                        fread(&m31, sizeof(float), 1, bin);
                        fread(&m32, sizeof(float), 1, bin);
                        fread(&m33, sizeof(float), 1, bin);

                        InvBindPos.push_back(XMFLOAT4X4(m00, m01, m02, m03,
                            m10, m11, m12, m13,
                            m20, m21, m22, m23,
                            m30, m31, m32, m33));
                    }
                }
                break;
                default:
                    fprintf(stderr, "Unsuported accessor type\n");
                    return;
                }
            }
            break;
            default:
                fprintf(stderr, "Unsuported component type\n");
                return;
            }
        }

        for (UINT j = 0; j < nBones; ++j)
        {
            wchar_t buf[1024];

            // Bone name
            UINT nName = static_cast<UINT>(strlen(data->skins->joints[j]->name) + 1);
            nb = fwrite(&nName, sizeof(UINT), 1, out);
            assert(nb);

            wsprintf(buf, L"%S", data->skins->joints[j]->name);
            nb = fwrite(buf, sizeof(wchar_t), nName, out);
            assert(nb);

            Bone bone = { 0 };
            bone.ParentIndex = bmap[data->skins->joints[j]->name];

            cgltf_float out_matrix_local[16];
            cgltf_node_transform_local(data->skins->joints[j], out_matrix_local);
            XMMATRIX lm(out_matrix_local);
            XMStoreFloat4x4(&bone.LocalTransform, lm);

            bone.InvBindPos = InvBindPos[j];
            bone.BindPos = XMFLOAT4X4();

            nb = fwrite(&bone, sizeof(Bone), 1, out);
            assert(nb);
        }
    }
}

void write_animation_clips(FILE* out, FILE* bin, cgltf_data* data, cgltf_size mesh, BYTE bSkeleton)
{
    size_t nb;

    // Animation Clips
    //UINT nClips = static_cast<UINT>(data->animations_count);
    UINT nClips = static_cast<UINT>(1);
    nb = fwrite(&nClips, sizeof(UINT), 1, out);
    assert(nb);

    for (cgltf_size animation_idx = 0; animation_idx < data->animations_count; ++animation_idx)
    {
        UINT nName = static_cast<UINT>(strlen(data->animations[animation_idx].name) + 1);
        nb = fwrite(&nName, sizeof(UINT), 1, out);
        assert(nb);

        wchar_t buf[1024];
        wsprintf(buf, L"%S", data->animations[animation_idx].name);
        nb = fwrite(buf, sizeof(wchar_t), nName, out);
        assert(nb);

        std::vector<Keyframe> keyframes;
        float end_time = 0.0f;

        cgltf_animation_channel* channels = data->animations[animation_idx].channels;
        cgltf_size channels_count = data->animations[animation_idx].channels_count;
        cgltf_animation_sampler* samplers = data->animations[animation_idx].samplers;

        cgltf_size time_interval_points = 500;// samplers[0].input->count;

        for (cgltf_size frame_idx = 0; frame_idx < time_interval_points; ++frame_idx)
        {
            for (cgltf_size channels_idx = 0; channels_idx < channels_count - 3 * 2; channels_idx += 3)
            {
                Keyframe list_keyframe[3] = { 0 };

                for (cgltf_size step = 0; step < 3; ++step)
                {
                    char* target_name = channels[channels_idx + step].target_node->name;
                    for (UINT j = 0; j < data->skins->joints_count; ++j)
                    {
                        if (strcmp(data->skins->joints[j]->name, target_name) == 0)
                        {
                            list_keyframe[step].BoneIndex = j;
                            break;
                        }
                    }
                }

                for (cgltf_size step = 0; step < 3; ++step) 
                {
                    cgltf_accessor* accessor = samplers[channels_idx + step].input;
                    cgltf_buffer_view* buffer_view = accessor->buffer_view;
                    cgltf_component_type component_type = accessor->component_type;

                    cgltf_size offset = accessor->offset + buffer_view->offset + frame_idx * accessor->stride;
                    if (offset < accessor->offset + buffer_view->offset + buffer_view->size)
                    {
                        fseek(bin, (long)(offset), SEEK_SET);

                        switch (component_type)
                        {
                        case cgltf_component_type_r_32f:
                        {
                            float val;
                            fread(&val, sizeof(float), 1, bin);

                            if (val > end_time) end_time = val;
                            list_keyframe[step].Time = val;
                        }
                        break;
                        default:
                            fprintf(stderr, "Unsuported component type\n");
                            return;
                        }
                    }
                }

                XMMATRIX TRS = XMMatrixIdentity();

                for (cgltf_size step = 0; step < 3; ++step)
                {
                    cgltf_accessor* accessor = samplers[channels_idx + step].output;
                    cgltf_buffer_view* buffer_view = accessor->buffer_view;
                    cgltf_component_type component_type = accessor->component_type;

                    cgltf_size offset = accessor->offset + buffer_view->offset + frame_idx * accessor->stride;
                    if (offset < accessor->offset + buffer_view->offset + buffer_view->size)
                    {
                        fseek(bin, (long)(offset), SEEK_SET);

                        switch (component_type)
                        {
                        case cgltf_component_type_r_32f: // 5126 FLOAT
                        {
                            float v1 = 0.0f, v2 = 0.0f, v3 = 0.0f, v4 = 0.0f;

                            if (accessor->type == cgltf_type_vec3)
                            {
                                fread(&v1, sizeof(float), 1, bin);
                                fread(&v2, sizeof(float), 1, bin);
                                fread(&v3, sizeof(float), 1, bin);
                            }
                            else if (accessor->type == cgltf_type_vec4)
                            {
                                fread(&v1, sizeof(float), 1, bin);
                                fread(&v2, sizeof(float), 1, bin);
                                fread(&v3, sizeof(float), 1, bin);
                                fread(&v4, sizeof(float), 1, bin);
                            }
                            else if (accessor->type == cgltf_type_scalar)
                            {
                                fread(&v1, sizeof(float), 1, bin);
                            }
                            else
                            {
                                fprintf(stderr, "Unsuported component type\n");
                                return;
                            }

                            XMMATRIX T = XMMatrixIdentity();
                            XMMATRIX R = XMMatrixIdentity();
                            XMMATRIX S = XMMatrixIdentity();

                            switch (channels[channels_idx + step].target_path)
                            {
                            case cgltf_animation_path_type_translation:
                            {
                                T = XMMatrixTranslation(v1, v2, v3);
                            }
                            break;

                            case cgltf_animation_path_type_rotation:
                            {
                                R = XMMatrixRotationQuaternion(XMVectorSet(v1, v2, v3, v4));
                            }
                            break;

                            case cgltf_animation_path_type_scale:
                            {
                                S = XMMatrixScaling(v1, v1, v1);
                            }
                            break;

                            case cgltf_animation_path_type_weights:
                            {
                                
                            }
                            break;

                            default:
                                fprintf(stderr, "Unsuported component type\n");
                                return;
                            }

                            TRS = T * R * S * TRS;

                        }
                        break;
                        default:
                            fprintf(stderr, "Unsuported component type\n");
                            return;
                        }
                    }
                }

                XMStoreFloat4x4(&list_keyframe[0].Transform, TRS);
                XMStoreFloat4x4(&list_keyframe[1].Transform, TRS);
                XMStoreFloat4x4(&list_keyframe[2].Transform, TRS);
                TRS = XMMatrixIdentity();

                keyframes.push_back(list_keyframe[0]);
                keyframes.push_back(list_keyframe[1]);
                keyframes.push_back(list_keyframe[2]);
            }
        }

        Clip clip = { 0 };

        clip.keys = static_cast<UINT>(keyframes.size());
        clip.StartTime = 0.0f;
        clip.EndTime = end_time;
        nb = fwrite(&clip, sizeof(Clip), 1, out);
        assert(nb);

        for (UINT iy = 0; iy < keyframes.size(); ++iy)
        {
            nb = fwrite(&keyframes[iy], sizeof(Keyframe), 1, out);
            assert(nb);
        }
    }
}
