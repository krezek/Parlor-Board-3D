//--------------------------------------------------------------------------------------
// File: cmodump.cpp
//
// CMO file content examination utility
//
// .CMO files are built by Visual Studio 2012 and an example renderer is provided
// in the VS Direct3D Starter Kit
// http://code.msdn.microsoft.com/Visual-Studio-3D-Starter-455a15f1
//
// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.
//--------------------------------------------------------------------------------------

#include <Windows.h>

#include <cstdint>
#include <cstdio>
#include <cwchar>
#include <memory>
#include <new>
#include <vector>

#include <DirectXMath.h>
#include <DirectXPackedVector.h>

using namespace DirectX;

//---------------------------------------------------------------------------------
struct handle_closer { void operator()(HANDLE h) noexcept { if (h) CloseHandle(h); } };

using ScopedHandle = std::unique_ptr<void, handle_closer>;

inline HANDLE safe_handle(HANDLE h) noexcept { return (h == INVALID_HANDLE_VALUE) ? nullptr : h; }

//--------------------------------------------------------------------------------------
namespace VSD3DStarter
{
    // .CMO files

    // UINT - Mesh count
    // { [Mesh count]
    //      UINT - Length of name
    //      wchar_t[] - Name of mesh (if length > 0)
    //      UINT - Material count
    //      { [Material count]
    //          UINT - Length of material name
    //          wchar_t[] - Name of material (if length > 0)
    //          Material structure
    //          UINT - Length of pixel shader name
    //          wchar_t[] - Name of pixel shader (if length > 0)
    //          { [8]
    //              UINT - Length of texture name
    //              wchar_t[] - Name of texture (if length > 0)
    //          }
    //      }
    //      BYTE - 1 if there is skeletal animation data present
    //      UINT - SubMesh count
    //      { [SubMesh count]
    //          SubMesh structure
    //      }
    //      UINT - IB Count
    //      { [IB Count]
    //          UINT - Number of USHORTs in IB
    //          USHORT[] - Array of indices
    //      }
    //      UINT - VB Count
    //      { [VB Count]
    //          UINT - Number of verts in VB
    //          Vertex[] - Array of vertices
    //      }
    //      UINT - Skinning VB Count
    //      { [Skinning VB Count]
    //          UINT - Number of verts in Skinning VB
    //          SkinningVertex[] - Array of skinning verts
    //      }
    //      MeshExtents structure
    //      [If skeleton animation data is not present, file ends here]
    //      UINT - Bone count
    //      { [Bone count]
    //          UINT - Length of bone name
    //          wchar_t[] - Bone name (if length > 0)
    //          Bone structure
    //      }
    //      UINT - Animation clip count
    //      { [Animation clip count]
    //          UINT - Length of clip name
    //          wchar_t[] - Clip name (if length > 0)
    //          float - Start time
    //          float - End time
    //          UINT - Keyframe count
    //          { [Keyframe count]
    //              Keyframe structure
    //          }
    //      }
    // }

#pragma pack(push,1)

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

#pragma pack(pop)

}; // namespace

static_assert(sizeof(VSD3DStarter::Material) == 132, "CMO Mesh structure size incorrect");
static_assert(sizeof(VSD3DStarter::SubMesh) == 20, "CMO Mesh structure size incorrect");
static_assert(sizeof(VSD3DStarter::Vertex) == 52, "CMO Mesh structure size incorrect");
static_assert(sizeof(VSD3DStarter::SkinningVertex) == 32, "CMO Mesh structure size incorrect");
static_assert(sizeof(VSD3DStarter::MeshExtents) == 40, "CMO Mesh structure size incorrect");
static_assert(sizeof(VSD3DStarter::Bone) == 196, "CMO Mesh structure size incorrect");
static_assert(sizeof(VSD3DStarter::Clip) == 12, "CMO Mesh structure size incorrect");
static_assert(sizeof(VSD3DStarter::Keyframe) == 72, "CMO Mesh structure size incorrect");

namespace
{
    HRESULT LoadDataFromFile(_In_z_ const wchar_t* fileName,
        std::unique_ptr<uint8_t[]>& data,
        size_t& dataSize)
    {
        // open the file
#if (_WIN32_WINNT >= 0x0602 /*_WIN32_WINNT_WIN8*/)
        ScopedHandle hFile(safe_handle(CreateFile2(fileName,
            GENERIC_READ,
            FILE_SHARE_READ,
            OPEN_EXISTING,
            nullptr)));
#else
        ScopedHandle hFile(safe_handle(CreateFileW(fileName,
            GENERIC_READ,
            FILE_SHARE_READ,
            nullptr,
            OPEN_EXISTING,
            FILE_ATTRIBUTE_NORMAL,
            nullptr)));
#endif
        if (!hFile)
            return HRESULT_FROM_WIN32(GetLastError());

        // Get the file size
        FILE_STANDARD_INFO fileInfo;
        if (!GetFileInformationByHandleEx(hFile.get(), FileStandardInfo, &fileInfo, sizeof(fileInfo)))
        {
            return HRESULT_FROM_WIN32(GetLastError());
        }

        // File is too big for 32-bit allocation, so reject read
        if (fileInfo.EndOfFile.HighPart > 0)
        {
            return E_FAIL;
        }

        // Need at least enough data to contain the mesh count
        if (fileInfo.EndOfFile.LowPart < sizeof(UINT))
        {
            return E_FAIL;
        }

        // create enough space for the file data
        data.reset(new (std::nothrow) uint8_t[fileInfo.EndOfFile.LowPart]);
        if (!data)
        {
            return E_OUTOFMEMORY;
        }

        // read the data in
        DWORD BytesRead = 0;
        if (!ReadFile(hFile.get(), data.get(), fileInfo.EndOfFile.LowPart, &BytesRead, nullptr))
        {
            return HRESULT_FROM_WIN32(GetLastError());
        }

        if (BytesRead < fileInfo.EndOfFile.LowPart)
        {
            return E_FAIL;
        }

        dataSize = fileInfo.EndOfFile.LowPart;

        return S_OK;
    }


    //---------------------------------------------------------------------------------
    void DumpVB(UINT id, const VSD3DStarter::Vertex* vb, size_t count, bool full)
    {
        bool ellipsis = false;

        size_t lowbound = 5;
        size_t hibound = (count > 5) ? (count - 5) : 0;

        wprintf(L"\tVB #%u - %zu vertices\n", id, count);

        if (!full)
            return;

        auto vptr = vb;
        for (size_t j = 0; j < count; ++j, ++vptr)
        {
            if ((j >= lowbound) && (j < hibound))
            {
                if (!ellipsis)
                {
                    ellipsis = true;
                    wprintf(L"\t ...\n");
                }
                continue;
            }

            XMVECTOR v = PackedVector::XMLoadUByteN4(reinterpret_cast<const PackedVector::XMUBYTEN4*>(&vptr->color));

            XMFLOAT4 clr;
            XMStoreFloat4(&clr, v);

            wprintf(L"\t %6zu:\tP:(%f, %f, %f)\n\t\t\tN:(%f, %f, %f)\n\t\t\tT:(%f, %f, %f, %f)\n\t\t\tC:(%f, %f, %f, %f)\n\t\t\tTX:(%f, %f)\n", j + 1,
                double(vptr->Position.x), double(vptr->Position.y), double(vptr->Position.z),
                double(vptr->Normal.x), double(vptr->Normal.y), double(vptr->Normal.z),
                double(vptr->Tangent.x), double(vptr->Tangent.y), double(vptr->Tangent.z), double(vptr->Tangent.w),
                double(clr.x), double(clr.y), double(clr.z), double(clr.w),
                double(vptr->TextureCoordinates.x), double(vptr->TextureCoordinates.y));
        }
    }


    //---------------------------------------------------------------------------------
    void DumpSkinVB(UINT id, const VSD3DStarter::SkinningVertex* vb, size_t count, bool full)
    {
        bool ellipsis = false;

        size_t lowbound = 5;
        size_t hibound = (count > 5) ? (count - 5) : 0;

        wprintf(L"\tSkinningVB #%u - %zu vertices\n", id, count);

        if (!full)
            return;

        auto vptr = vb;
        for (size_t j = 0; j < count; ++j, ++vptr)
        {
            if ((j >= lowbound) && (j < hibound))
            {
                if (!ellipsis)
                {
                    ellipsis = true;
                    wprintf(L"\t ...\n");
                }
                continue;
            }

            wprintf(L"\t %6zu:\tB:(%u, %u, %u, %u)\n\t\t\tW:(%f, %f, %f, %f)\n", j + 1,
                vptr->boneIndex[0], vptr->boneIndex[1], vptr->boneIndex[2], vptr->boneIndex[3],
                double(vptr->boneWeight[0]), double(vptr->boneWeight[1]), double(vptr->boneWeight[2]), double(vptr->boneWeight[3]));
        }
    }

    //---------------------------------------------------------------------------------
    template<typename index_t>
    void DumpIB(UINT id, const index_t* ib, size_t count, bool full)
    {
        bool ellipsis = false;

        size_t nFaces = count / 3;

        size_t lowbound = 5;
        size_t hibound = (nFaces > 5) ? (nFaces - 5) : 0;

        wprintf(L"\tIB #%u - %zu indices (%zu faces)\n", id, count, nFaces);

        if (!full)
            return;

        const index_t* iptr = ib;
        for (size_t j = 0; j < nFaces; ++j, iptr += 3)
        {
            if ((j >= lowbound) && (j < hibound))
            {
                if (!ellipsis)
                {
                    ellipsis = true;
                    wprintf(L"\t ...\n");
                }
                continue;
            }

            wprintf(L"\t %6zu:\t%u, %u, %u\n", j + 1, *iptr, *(iptr + 1), *(iptr + 2));
        }
    }
}

//---------------------------------------------------------------------------------
#define EOFCHECK if ( dataSize < usedSize ) { wprintf( L"ERROR: End of file\n" ); return -1; }

int wmain(int argc, wchar_t* argv[])
{
    using namespace VSD3DStarter;

    if (argc != 2 && argc != 3)
    {
        wprintf(L"Usage: %s <filename.cmo> [-full]\n", argv[0]);
        return -1;
    }

    bool fulldump = false;

    if (argc == 3)
    {
        if (!_wcsicmp(argv[2], L"-full") || !_wcsicmp(argv[2], L"/full"))
        {
            fulldump = true;
        }
        else
        {
            wprintf(L"ERROR: Unknown switch '%s'\n", argv[2]);
            return -1;
        }
    }

    size_t dataSize = 0;
    std::unique_ptr<uint8_t[]> data;
    HRESULT hr = LoadDataFromFile(argv[1], data, dataSize);
    if (FAILED(hr))
    {
        wprintf(L"ERROR: failed to load %s\n", argv[1]);
        return -1;
    }

    const uint8_t* meshData = data.get();

    auto nMesh = reinterpret_cast<const UINT*>(meshData);
    size_t usedSize = sizeof(UINT);

    if (!*nMesh)
    {
        wprintf(L"ERROR: No meshes found\n");
        return -1;
    }

    if (*nMesh > 1)
    {
        wprintf(L"CMO %s\n%u meshes\n", argv[1], *nMesh);
    }
    else
    {
        wprintf(L"CMO %s\n", argv[1]);
    }

    for (size_t mesh = 0; mesh < *nMesh; ++mesh)
    {
        // Mesh name
        auto nName = reinterpret_cast<const UINT*>(meshData + usedSize);
        usedSize += sizeof(UINT);
        EOFCHECK

        auto meshName = reinterpret_cast<const wchar_t*>(meshData + usedSize);
        usedSize += sizeof(wchar_t) * (*nName);
        EOFCHECK

        // Materials
        auto nMats = reinterpret_cast<const UINT*>(meshData + usedSize);
        usedSize += sizeof(UINT);
        EOFCHECK

        wprintf(L" Mesh #%zu '%s'\n\tMaterials (%u)\n", mesh, meshName, *nMats);

        for (UINT j = 0; j < *nMats; ++j)
        {
            // Material name
            nName = reinterpret_cast<const UINT*>(meshData + usedSize);
            usedSize += sizeof(UINT);
            EOFCHECK

            auto matName = reinterpret_cast<const wchar_t*>(meshData + usedSize);
            usedSize += sizeof(wchar_t) * (*nName);
            EOFCHECK

            // Material settings
            auto matSetting = reinterpret_cast<const VSD3DStarter::Material*>(meshData + usedSize);
            usedSize += sizeof(VSD3DStarter::Material);
            EOFCHECK

            // Pixel shader name
            nName = reinterpret_cast<const UINT*>(meshData + usedSize);
            usedSize += sizeof(UINT);
            EOFCHECK

            auto psName = reinterpret_cast<const wchar_t*>(meshData + usedSize);
            usedSize += sizeof(wchar_t) * (*nName);
            EOFCHECK

            wprintf(L"\t\tMaterial #%u '%s'\n", j, matName);

            if (fulldump)
            {
                wprintf(L"\t\t A: (%f, %f, %f, %f)\n\t\t D: (%f, %f, %f, %f)\n\t\t S: (%f, %f, %f, %f) ^%f\n\t\t E: (%f, %f, %f, %f)\n",
                    double(matSetting->Ambient.x), double(matSetting->Ambient.y), double(matSetting->Ambient.z), double(matSetting->Ambient.w),
                    double(matSetting->Diffuse.x), double(matSetting->Diffuse.y), double(matSetting->Diffuse.z), double(matSetting->Diffuse.w),
                    double(matSetting->Specular.x), double(matSetting->Specular.y), double(matSetting->Specular.z), double(matSetting->Specular.w),
                    double(matSetting->SpecularPower),
                    double(matSetting->Emissive.x), double(matSetting->Emissive.y), double(matSetting->Emissive.z), double(matSetting->Emissive.w));

                wprintf(L"\t\t UV: [%f, %f, %f, %f]\n\t\t     [%f, %f, %f, %f]\n\t\t     [%f, %f, %f, %f]\n\t\t     [%f, %f, %f, %f]\n",
                    double(matSetting->UVTransform._11), double(matSetting->UVTransform._12), double(matSetting->UVTransform._13), double(matSetting->UVTransform._14),
                    double(matSetting->UVTransform._21), double(matSetting->UVTransform._22), double(matSetting->UVTransform._23), double(matSetting->UVTransform._24),
                    double(matSetting->UVTransform._31), double(matSetting->UVTransform._32), double(matSetting->UVTransform._33), double(matSetting->UVTransform._34),
                    double(matSetting->UVTransform._41), double(matSetting->UVTransform._42), double(matSetting->UVTransform._43), double(matSetting->UVTransform._44));
            }

            {
                wchar_t root[MAX_PATH] = {};
                auto last = wcsrchr(psName, '_');
                if (last)
                {
                    wcscpy_s(root, last + 1);
                }
                else
                {
                    wcscpy_s(root, psName);
                }

                auto first = wcschr(root, '.');
                if (first)
                    *first = 0;

                if (fulldump)
                {
                    wprintf(L"\t\t PS: '%s'\n('%s')\n", root, psName);
                }
                else
                {
                    wprintf(L"\t\t PS: '%s'\n", root);
                }
            }

            for (UINT t = 0; t < VSD3DStarter::MAX_TEXTURE; ++t)
            {
                nName = reinterpret_cast<const UINT*>(meshData + usedSize);
                usedSize += sizeof(UINT);
                EOFCHECK

                auto txtName = reinterpret_cast<const wchar_t*>(meshData + usedSize);
                usedSize += sizeof(wchar_t) * (*nName);
                EOFCHECK

                if (*nName && *txtName)
                    wprintf(L"\t   Texture%u: '%s'\n", t, txtName);
            }
        }

        // Skeletal data?
        auto bSkeleton = reinterpret_cast<const BYTE*>(meshData + usedSize);
        usedSize += sizeof(BYTE);
        EOFCHECK

        // Submeshes
        std::vector<VSD3DStarter::SubMesh> submeshes;

        auto nSubmesh = reinterpret_cast<const UINT*>(meshData + usedSize);
        usedSize += sizeof(UINT);
        EOFCHECK

        if (!*nSubmesh)
        {
            wprintf(L"WARNING: No sub-meshes found in %s\n", meshName);
        }
        else
        {
            auto subMesh = reinterpret_cast<const VSD3DStarter::SubMesh*>(meshData + usedSize);
            usedSize += sizeof(VSD3DStarter::SubMesh) * (*nSubmesh);
            EOFCHECK

                wprintf(L"\tSubMeshes (%u)\n", *nSubmesh);

            for (UINT j = 0; j < *nSubmesh; ++j)
            {
                wprintf(L"\t\tSubMesh #%u - material %u, IB %u, VB %u, start %u, count %u\n", j,
                    subMesh[j].MaterialIndex, subMesh[j].IndexBufferIndex, subMesh[j].VertexBufferIndex,
                    subMesh[j].StartIndex, subMesh[j].PrimCount);

                submeshes.push_back(*subMesh);
            }
        }

        // Index buffers
        auto nIBs = reinterpret_cast<const UINT*>(meshData + usedSize);
        usedSize += sizeof(UINT);
        EOFCHECK

        if (!*nIBs)
        {
            wprintf(L"WARNING: No IBs found in %s\n", meshName);
        }
        else
        {
            std::vector<size_t> ibs;
            ibs.reserve(*nIBs);
            for (UINT j = 0; j < *nIBs; ++j)
            {
                auto nIndexes = reinterpret_cast<const UINT*>(meshData + usedSize);
                usedSize += sizeof(UINT);
                EOFCHECK

                    if (!*nIndexes)
                    {
                        wprintf(L"WARNING: Empty IB found in %s (%u)\n", meshName, j);
                    }
                    else
                    {
                        size_t ibBytes = sizeof(uint16_t) * (*(nIndexes));
                        auto indexes = reinterpret_cast<const uint16_t*>(meshData + usedSize);
                        usedSize += ibBytes;
                        EOFCHECK

                            DumpIB(j, indexes, *nIndexes, fulldump);
                    }

                ibs.push_back(*nIndexes);
            }

            UINT j = 0;
            for (auto it = submeshes.cbegin(); it != submeshes.cend(); ++it, ++j)
            {
                if (it->IndexBufferIndex >= *nIBs)
                {
                    wprintf(L"ERROR: Submesh %u references invalid IB %u\n", j, it->IndexBufferIndex);
                }
                else if ((it->StartIndex + it->PrimCount * 3) > ibs[it->IndexBufferIndex])
                {
                    wprintf(L"ERROR: Submesh %u references invalid indices in IB %u with %zu indices (%u, %u)\n", j,
                        it->IndexBufferIndex, ibs[it->IndexBufferIndex], it->StartIndex, it->PrimCount);
                }
            }
        }

        // Vertex buffers
        auto nVBs = reinterpret_cast<const UINT*>(meshData + usedSize);
        usedSize += sizeof(UINT);
        EOFCHECK

        std::vector<size_t> vbData;
        if (!*nVBs)
        {
            wprintf(L"WARNING: No VBs found in %s\n", meshName);
        }
        else
        {
            vbData.reserve(*nVBs);

            UINT j = 0;
            for (auto it = submeshes.cbegin(); it != submeshes.cend(); ++it, ++j)
            {
                if (it->VertexBufferIndex >= *nVBs)
                {
                    wprintf(L"ERROR: Submesh %u references invalid VB %u\n", j, it->VertexBufferIndex);
                }
            }

            for (j = 0; j < *nVBs; ++j)
            {
                auto nVerts = reinterpret_cast<const UINT*>(meshData + usedSize);
                usedSize += sizeof(UINT);
                EOFCHECK

                if (!*nVerts)
                {
                    wprintf(L"WARNING: Empty VB found in %s (%u)\n", meshName, j);
                }
                else
                {
                    size_t vbBytes = sizeof(Vertex) * (*nVerts);

                    auto verts = reinterpret_cast<const Vertex*>(meshData + usedSize);
                    usedSize += vbBytes;
                    EOFCHECK

                        DumpVB(j, verts, *nVerts, fulldump);
                }

                vbData.push_back(*nVerts);
            }
        }

        // Skinning vertex buffers
        auto nSkinVBs = reinterpret_cast<const UINT*>(meshData + usedSize);
        usedSize += sizeof(UINT);
        EOFCHECK

        if (*nSkinVBs)
        {
            bool mismatch = false;
            if (*nSkinVBs != *nVBs)
            {
                wprintf(L"WARNING: Number of VBs not equal to number of skin VBs in %s (%u, %u)\n", meshName, *nVBs, *nSkinVBs);
                mismatch = true;
            }

            for (UINT j = 0; j < *nSkinVBs; ++j)
            {
                auto nVerts = reinterpret_cast<const UINT*>(meshData + usedSize);
                usedSize += sizeof(UINT);
                EOFCHECK

                    if (!*nVerts)
                    {
                        wprintf(L"WARNING: Empty SkinVB found in %s (%u)\n", meshName, j);
                    }
                    else
                    {
                        if (!mismatch)
                        {
                            if (vbData[j] != *nVerts)
                            {
                                wprintf(L"WARNING: Mismatch of verts for VB vs. SkinVB\n");
                            }
                        }

                        size_t vbBytes = sizeof(VSD3DStarter::SkinningVertex) * (*(nVerts));

                        auto verts = reinterpret_cast<const VSD3DStarter::SkinningVertex*>(meshData + usedSize);
                        usedSize += vbBytes;
                        EOFCHECK

                            DumpSkinVB(j, verts, *nVerts, fulldump);
                    }
            }
        }

        // Extents
        auto extents = reinterpret_cast<const VSD3DStarter::MeshExtents*>(meshData + usedSize);
        usedSize += sizeof(VSD3DStarter::MeshExtents);
        EOFCHECK

        wprintf(L"\tExtents:\n\t\t Sphere (%f, %f, %f) %f\n\t\t Box (%f, %f, %f)\n\t\t     (%f, %f, %f)\n",
            double(extents->CenterX), double(extents->CenterY), double(extents->CenterZ), double(extents->Radius),
            double(extents->MinX), double(extents->MinY), double(extents->MinZ), double(extents->MaxX), double(extents->MaxY), double(extents->MaxZ));

        // Animation data
        if (*bSkeleton)
        {
            // Bones
            auto nBones = reinterpret_cast<const UINT*>(meshData + usedSize);
            usedSize += sizeof(UINT);
            EOFCHECK

            if (!*nBones)
            {
                wprintf(L"WARNING: Animation bone data is missing\n");
            }
            else
            {
                wprintf(L"\tBones (%u)\n", *nBones);

                for (UINT j = 0; j < *nBones; ++j)
                {
                    // Bone name
                    nName = reinterpret_cast<const UINT*>(meshData + usedSize);
                    usedSize += sizeof(UINT);
                    EOFCHECK

                    auto boneName = reinterpret_cast<const wchar_t*>(meshData + usedSize);
                    usedSize += sizeof(wchar_t) * (*nName);
                    EOFCHECK

                    // Bone settings
                    auto bones = reinterpret_cast<const VSD3DStarter::Bone*>(meshData + usedSize);
                    usedSize += sizeof(VSD3DStarter::Bone);
                    EOFCHECK

                    if (bones->ParentIndex == -1)
                    {
                        wprintf(L"\t\t Bone #%u '%s'\n", j, boneName);
                    }
                    else
                    {
                        wprintf(L"\t\t Bone #%u '%s' (parent %d)\n", j, boneName, bones->ParentIndex);

                        if (bones->ParentIndex >= INT(*nBones))
                        {
                            wprintf(L"WARNING: Bone %u '%s' references bone not present in mesh '%s'\n",
                                j, boneName, meshName);
                        }
                    }
                }
            }

            // Animation Clips
            auto nClips = reinterpret_cast<const UINT*>(meshData + usedSize);
            usedSize += sizeof(UINT);
            EOFCHECK

            if (!*nClips)
            {
                wprintf(L"WARNING: No animation clips found\n");
            }
            else
            {
                wprintf(L"\tClips (%u)\n", *nBones);

                for (UINT j = 0; j < *nClips; ++j)
                {
                    // Clip name
                    nName = reinterpret_cast<const UINT*>(meshData + usedSize);
                    usedSize += sizeof(UINT);
                    EOFCHECK

                    auto clipName = reinterpret_cast<const wchar_t*>(meshData + usedSize);
                    usedSize += sizeof(wchar_t) * (*nName);
                    EOFCHECK

                    auto clip = reinterpret_cast<const VSD3DStarter::Clip*>(meshData + usedSize);
                    usedSize += sizeof(VSD3DStarter::Clip);
                    EOFCHECK

                    if (!clip->keys)
                    {
                        wprintf(L"WARNING: Animation clip %s for mesh %s missing keys", clipName, meshName);
                    }
                    else
                    {
                        auto keys = reinterpret_cast<const Keyframe*>(meshData + usedSize);
                        usedSize += sizeof(Keyframe) * clip->keys;
                        EOFCHECK

                        wprintf(L"\t\t Clip '%s' with %u keys (t: %f...%f)\n",
                            clipName, clip->keys, double(clip->StartTime), double(clip->EndTime));

                        for (UINT k = 0; k < clip->keys; ++k)
                        {
                            if (keys[k].BoneIndex >= *nBones)
                            {
                                wprintf(L"WARNING: Key %u in clip '%s' references bone not in mesh '%s' (%u, %u)\n",
                                    k, clipName, meshName, keys[k].BoneIndex, *nBones);
                            }
                        }
                    }
                }
            }
        }
        else if (*nSkinVBs)
        {
            wprintf(L"WARNING: Found skinning verts, but no skeleton data present\n");
        }
        else
        {
            wprintf(L"\tNo animation or skinning data present.\n");
        }
    }

    return 0;
}
