#include "pch.h"
#include "platform.h"

#include <Model.h>

using namespace DirectX;

#define EOFCHECK if ( dataSize < usedSize ) { wprintf( L"ERROR: End of file\n" ); return -1; }

//---------------------------------------------------------------------------------
struct handle_closer { void operator()(HANDLE h) noexcept { if (h) CloseHandle(h); } };

using ScopedHandle = std::unique_ptr<void, handle_closer>;

inline HANDLE safe_handle(HANDLE h) noexcept { return (h == INVALID_HANDLE_VALUE) ? nullptr : h; }

//--------------------------------------------------------------------------------------

HRESULT Model::LoadDataFromFile(_In_z_ const wchar_t* fileName,
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

void Model::DumpVB(VertexBuffer& vbL, UINT id, const Model::Vertex* vb, size_t count, bool full)
{
    bool ellipsis = false;

    size_t lowbound = 5;
    size_t hibound = (count > 5) ? (count - 5) : 0;
#ifdef DEBUG_MODEL
    wprintf(L"\tVB #%u - %zu vertices\n", id, count);
#endif
    if (!full)
        return;

    auto vptr = vb;
    for (size_t j = 0; j < count; ++j, ++vptr)
    {
        vbL.Vertices.push_back(*vptr);

        if ((j >= lowbound) && (j < hibound))
        {
            if (!ellipsis)
            {
                ellipsis = true;
#ifdef DEBUG_MODEL
                wprintf(L"\t ...\n");
#endif
            }
            continue;
        }

        XMVECTOR v = PackedVector::XMLoadUByteN4(reinterpret_cast<const PackedVector::XMUBYTEN4*>(&vptr->color));

        XMFLOAT4 clr;
        XMStoreFloat4(&clr, v);
#ifdef DEBUG_MODEL
        wprintf(L"\t %6zu:\tP:(%f, %f, %f)\n\t\t\tN:(%f, %f, %f)\n\t\t\tT:(%f, %f, %f, %f)\n\t\t\tC:(%f, %f, %f, %f)\n\t\t\tTX:(%f, %f)\n", j + 1,
            double(vptr->Position.x), double(vptr->Position.y), double(vptr->Position.z),
            double(vptr->Normal.x), double(vptr->Normal.y), double(vptr->Normal.z),
            double(vptr->Tangent.x), double(vptr->Tangent.y), double(vptr->Tangent.z), double(vptr->Tangent.w),
            double(clr.x), double(clr.y), double(clr.z), double(clr.w),
            double(vptr->TextureCoordinates.x), double(vptr->TextureCoordinates.y));
#endif
    }
}

void Model::DumpSkinVB(SkinningVertexBuffer& svbL, UINT id, const Model::SkinningVertex* vb, size_t count, bool full)
{
    bool ellipsis = false;

    size_t lowbound = 5;
    size_t hibound = (count > 5) ? (count - 5) : 0;
#ifdef DEBUG_MODEL
    wprintf(L"\tSkinningVB #%u - %zu vertices\n", id, count);
#endif
    if (!full)
        return;

    auto vptr = vb;
    for (size_t j = 0; j < count; ++j, ++vptr)
    {
        svbL.SkinningVertices.push_back(*vptr);

        if ((j >= lowbound) && (j < hibound))
        {
            if (!ellipsis)
            {
                ellipsis = true;
#ifdef DEBUG_MODEL
                wprintf(L"\t ...\n");
#endif
            }
            continue;
        }
#ifdef DEBUG_MODEL
        wprintf(L"\t %6zu:\tB:(%u, %u, %u, %u)\n\t\t\tW:(%f, %f, %f, %f)\n", j + 1,
            vptr->boneIndex[0], vptr->boneIndex[1], vptr->boneIndex[2], vptr->boneIndex[3],
            double(vptr->boneWeight[0]), double(vptr->boneWeight[1]), double(vptr->boneWeight[2]), double(vptr->boneWeight[3]));
#endif
    }
}

template<typename index_t>
void Model::DumpIB(IndexBuffer& ibL, UINT id, const index_t* ib, size_t count, bool full)
{
    bool ellipsis = false;

    size_t nFaces = count / 3;

    size_t lowbound = 5;
    size_t hibound = (nFaces > 5) ? (nFaces - 5) : 0;
#ifdef DEBUG_MODEL
    wprintf(L"\tIB #%u - %zu indices (%zu faces)\n", id, count, nFaces);
#endif
    if (!full)
        return;

    const index_t* iptr = ib;
    for (size_t j = 0; j < nFaces; ++j, iptr += 3)
    {
        ibL.Indices16.push_back(*iptr);
        ibL.Indices16.push_back(*(iptr + 1));
        ibL.Indices16.push_back(*(iptr + 2));

        if ((j >= lowbound) && (j < hibound))
        {
            if (!ellipsis)
            {
                ellipsis = true;
#ifdef DEBUG_MODEL
                wprintf(L"\t ...\n");
#endif
            }
            continue;
        }
#ifdef DEBUG_MODEL
        wprintf(L"\t %6zu:\t%u, %u, %u\n", j + 1, *iptr, *(iptr + 1), *(iptr + 2));
#endif
    }
}

int Model::LoadCMOFile(const wchar_t* filename)
{
    bool fulldump = true;

    size_t dataSize = 0;
    std::unique_ptr<uint8_t[]> data;
    HRESULT hr = LoadDataFromFile(filename, data, dataSize);
    if (FAILED(hr))
    {
        wprintf(L"ERROR: failed to load %s\n", filename);
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
#ifdef DEBUG_MODEL
        wprintf(L"CMO %s\n%u meshes\n", filename, *nMesh);
#endif
    }
    else
    {
#ifdef DEBUG_MODEL
        wprintf(L"CMO %s\n", filename);
#endif
    }

    for (size_t mesh = 0; mesh < *nMesh; ++mesh)
    {
        Mesh meshL;

        // Mesh name
        auto nName = reinterpret_cast<const UINT*>(meshData + usedSize);
        usedSize += sizeof(UINT);
        EOFCHECK

            auto meshName = reinterpret_cast<const wchar_t*>(meshData + usedSize);
        usedSize += sizeof(wchar_t) * (*nName);
        EOFCHECK

            meshL.Name = meshName;

            // Materials
            auto nMats = reinterpret_cast<const UINT*>(meshData + usedSize);
        usedSize += sizeof(UINT);
        EOFCHECK

#ifdef DEBUG_MODEL
            wprintf(L" Mesh #%zu '%s'\n\tMaterials (%u)\n", mesh, meshName, *nMats);
#endif

        for (UINT j = 0; j < *nMats; ++j)
        {
            MaterialInfo miL;

            // Material name
            nName = reinterpret_cast<const UINT*>(meshData + usedSize);
            usedSize += sizeof(UINT);
            EOFCHECK

                auto matName = reinterpret_cast<const wchar_t*>(meshData + usedSize);
            usedSize += sizeof(wchar_t) * (*nName);
            EOFCHECK

                miL.Name = matName;

                // Material settings
                auto matSetting = reinterpret_cast<const Model::Material*>(meshData + usedSize);
            usedSize += sizeof(Model::Material);
            EOFCHECK

                miL.Mat = *matSetting;

                // Pixel shader name
                nName = reinterpret_cast<const UINT*>(meshData + usedSize);
            usedSize += sizeof(UINT);
            EOFCHECK

                auto psName = reinterpret_cast<const wchar_t*>(meshData + usedSize);
            usedSize += sizeof(wchar_t) * (*nName);
            EOFCHECK

                miL.PSName = psName;

#ifdef DEBUG_MODEL
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
#endif

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

#ifdef DEBUG_MODEL
                if (fulldump)
                {
                    wprintf(L"\t\t PS: '%s'\n('%s')\n", root, psName);
                }
                else
                {
                    wprintf(L"\t\t PS: '%s'\n", root);
                }
#endif

                miL.Root = root;
            }

            for (UINT t = 0; t < Model::MAX_TEXTURE; ++t)
            {
                Texture tx;

                nName = reinterpret_cast<const UINT*>(meshData + usedSize);
                usedSize += sizeof(UINT);
                EOFCHECK

                    auto txtName = reinterpret_cast<const wchar_t*>(meshData + usedSize);
                usedSize += sizeof(wchar_t) * (*nName);
                EOFCHECK

                    tx.Name = txtName;

#ifdef DEBUG_MODEL
                    if (*nName && *txtName)
                        wprintf(L"\t   Texture%u: '%s'\n", t, txtName);
#endif
                    miL.Textures.push_back(tx);
            }

            meshL.MaterialsInfo.push_back(miL);
        }

        // Skeletal data?
        auto bSkeleton = reinterpret_cast<const BYTE*>(meshData + usedSize);
        usedSize += sizeof(BYTE);
        EOFCHECK

            // Submeshes
            std::vector<Model::SubMesh> submeshes;

        auto nSubmesh = reinterpret_cast<const UINT*>(meshData + usedSize);
        usedSize += sizeof(UINT);
        EOFCHECK

            if (!*nSubmesh)
            {
#ifdef DEBUG_MODEL
                wprintf(L"WARNING: No sub-meshes found in %s\n", meshName);
#endif
            }
            else
            {
                auto subMesh = reinterpret_cast<const Model::SubMesh*>(meshData + usedSize);
                usedSize += sizeof(Model::SubMesh) * (*nSubmesh);
                EOFCHECK

#ifdef DEBUG_MODEL
                    wprintf(L"\tSubMeshes (%u)\n", *nSubmesh);
#endif

                for (UINT j = 0; j < *nSubmesh; ++j)
                {
#ifdef DEBUG_MODEL
                    wprintf(L"\t\tSubMesh #%u - material %u, IB %u, VB %u, start %u, count %u\n", j,
                        subMesh[j].MaterialIndex, subMesh[j].IndexBufferIndex, subMesh[j].VertexBufferIndex,
                        subMesh[j].StartIndex, subMesh[j].PrimCount);
#endif

                    submeshes.push_back(*subMesh);

                    meshL.SubMeshes.push_back(*subMesh);
                }
            }

        // Index buffers
        auto nIBs = reinterpret_cast<const UINT*>(meshData + usedSize);
        usedSize += sizeof(UINT);
        EOFCHECK

            if (!*nIBs)
            {
#ifdef DEBUG_MODEL
                wprintf(L"WARNING: No IBs found in %s\n", meshName);
#endif
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

                        IndexBuffer ibL; 
                        ibL.Count = *nIndexes;

                        if (!*nIndexes)
                        {
#ifdef DEBUG_MODEL
                            wprintf(L"WARNING: Empty IB found in %s (%u)\n", meshName, j);
#endif
                        }
                        else
                        {
                            size_t ibBytes = sizeof(uint16_t) * (*(nIndexes));
                            auto indexes = reinterpret_cast<const uint16_t*>(meshData + usedSize);
                            usedSize += ibBytes;
                            EOFCHECK

                                DumpIB(ibL, j, indexes, *nIndexes, fulldump);
                        }

                    ibs.push_back(*nIndexes);

                    meshL.Indices.push_back(ibL);
                }

                UINT j = 0;
                for (auto it = submeshes.cbegin(); it != submeshes.cend(); ++it, ++j)
                {
                    if (it->IndexBufferIndex >= *nIBs)
                    {
#ifdef DEBUG_MODEL
                        wprintf(L"ERROR: Submesh %u references invalid IB %u\n", j, it->IndexBufferIndex);
#endif
                    }
                    else if ((it->StartIndex + it->PrimCount * 3) > ibs[it->IndexBufferIndex])
                    {
#ifdef DEBUG_MODEL
                        wprintf(L"ERROR: Submesh %u references invalid indices in IB %u with %zu indices (%u, %u)\n", j,
                            it->IndexBufferIndex, ibs[it->IndexBufferIndex], it->StartIndex, it->PrimCount);
#endif
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
#ifdef DEBUG_MODEL
            wprintf(L"WARNING: No VBs found in %s\n", meshName);
#endif
        }
        else
        {
            vbData.reserve(*nVBs);

            UINT j = 0;
            for (auto it = submeshes.cbegin(); it != submeshes.cend(); ++it, ++j)
            {
                if (it->VertexBufferIndex >= *nVBs)
                {
#ifdef DEBUG_MODEL
                    wprintf(L"ERROR: Submesh %u references invalid VB %u\n", j, it->VertexBufferIndex);
#endif
                }
            }

            for (j = 0; j < *nVBs; ++j)
            {
                auto nVerts = reinterpret_cast<const UINT*>(meshData + usedSize);
                usedSize += sizeof(UINT);
                EOFCHECK

                    VertexBuffer vbL;
                    vbL.Count = *nVerts;

                    if (!*nVerts)
                    {
#ifdef DEBUG_MODEL
                        wprintf(L"WARNING: Empty VB found in %s (%u)\n", meshName, j);
#endif
                    }
                    else
                    {
                        size_t vbBytes = sizeof(Vertex) * (*nVerts);

                        auto verts = reinterpret_cast<const Vertex*>(meshData + usedSize);
                        usedSize += vbBytes;
                        EOFCHECK

                            DumpVB(vbL, j, verts, *nVerts, fulldump);
                    }

                vbData.push_back(*nVerts);

                meshL.Vertices.push_back(vbL);
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
#ifdef DEBUG_MODEL
                    wprintf(L"WARNING: Number of VBs not equal to number of skin VBs in %s (%u, %u)\n", meshName, *nVBs, *nSkinVBs);
#endif
                    mismatch = true;
                }

                for (UINT j = 0; j < *nSkinVBs; ++j)
                {
                    auto nVerts = reinterpret_cast<const UINT*>(meshData + usedSize);
                    usedSize += sizeof(UINT);
                    EOFCHECK

                        SkinningVertexBuffer svbL;
                        svbL.Count = *nVerts;

                        if (!*nVerts)
                        {
#ifdef DEBUG_MODEL
                            wprintf(L"WARNING: Empty SkinVB found in %s (%u)\n", meshName, j);
#endif
                        }
                        else
                        {
                            if (!mismatch)
                            {
                                if (vbData[j] != *nVerts)
                                {
#ifdef DEBUG_MODEL
                                    wprintf(L"WARNING: Mismatch of verts for VB vs. SkinVB\n");
#endif
                                }
                            }

                            size_t vbBytes = sizeof(Model::SkinningVertex) * (*(nVerts));

                            auto verts = reinterpret_cast<const Model::SkinningVertex*>(meshData + usedSize);
                            usedSize += vbBytes;
                            EOFCHECK

                                DumpSkinVB(svbL, j, verts, *nVerts, fulldump);
                        }

                        meshL.SkinningVertices.push_back(svbL);
                }
            }

        // Extents
        auto extents = reinterpret_cast<const Model::MeshExtents*>(meshData + usedSize);
        usedSize += sizeof(Model::MeshExtents);
        EOFCHECK

            meshL.extents = *extents;
#ifdef DEBUG_MODEL
            wprintf(L"\tExtents:\n\t\t Sphere (%f, %f, %f) %f\n\t\t Box (%f, %f, %f)\n\t\t     (%f, %f, %f)\n",
                double(extents->CenterX), double(extents->CenterY), double(extents->CenterZ), double(extents->Radius),
                double(extents->MinX), double(extents->MinY), double(extents->MinZ), double(extents->MaxX), double(extents->MaxY), double(extents->MaxZ));
#endif
        // Animation data
        if (*bSkeleton)
        {
            // Bones
            auto nBones = reinterpret_cast<const UINT*>(meshData + usedSize);
            usedSize += sizeof(UINT);
            EOFCHECK

                if (!*nBones)
                {
#ifdef DEBUG_MODEL
                    wprintf(L"WARNING: Animation bone data is missing\n");
#endif
                }
                else
                {
#ifdef DEBUG_MODEL
                    wprintf(L"\tBones (%u)\n", *nBones);
#endif

                    for (UINT j = 0; j < *nBones; ++j)
                    {
                        BoneInfo biL;

                        // Bone name
                        nName = reinterpret_cast<const UINT*>(meshData + usedSize);
                        usedSize += sizeof(UINT);
                        EOFCHECK

                            auto boneName = reinterpret_cast<const wchar_t*>(meshData + usedSize);
                        usedSize += sizeof(wchar_t) * (*nName);
                        EOFCHECK

                            biL.Name = boneName;

                            // Bone settings
                            auto bones = reinterpret_cast<const Model::Bone*>(meshData + usedSize);
                        usedSize += sizeof(Model::Bone);
                        EOFCHECK

                            biL.bone = *bones;

                            if (bones->ParentIndex == -1)
                            {
#ifdef DEBUG_MODEL
                                wprintf(L"\t\t Bone #%u '%s'\n", j, boneName);
#endif
                            }
                            else
                            {
#ifdef DEBUG_MODEL
                                wprintf(L"\t\t Bone #%u '%s' (parent %d)\n", j, boneName, bones->ParentIndex);

                                if (bones->ParentIndex >= INT(*nBones))
                                {
                                    wprintf(L"WARNING: Bone %u '%s' references bone not present in mesh '%s'\n",
                                        j, boneName, meshName);
                                }
#endif
                            }

                            meshL.BonesInfo.push_back(biL);
                    }
                }

            // Animation Clips
            auto nClips = reinterpret_cast<const UINT*>(meshData + usedSize);
            usedSize += sizeof(UINT);
            EOFCHECK

                if (!*nClips)
                {
#ifdef DEBUG_MODEL
                    wprintf(L"WARNING: No animation clips found\n");
#endif
                }
                else
                {
#ifdef DEBUG_MODEL
                    wprintf(L"\tClips (%u)\n", *nBones);
#endif

                    for (UINT j = 0; j < *nClips; ++j)
                    {
                        ClipInfo ciL;

                        // Clip name
                        nName = reinterpret_cast<const UINT*>(meshData + usedSize);
                        usedSize += sizeof(UINT);
                        EOFCHECK

                            auto clipName = reinterpret_cast<const wchar_t*>(meshData + usedSize);
                        usedSize += sizeof(wchar_t) * (*nName);
                        EOFCHECK

                            ciL.Name = clipName;

                            auto clip = reinterpret_cast<const Model::Clip*>(meshData + usedSize);
                        usedSize += sizeof(Model::Clip);
                        EOFCHECK

                            ciL.clip = *clip;

                            if (!clip->keys)
                            {
#ifdef DEBUG_MODEL
                                wprintf(L"WARNING: Animation clip %s for mesh %s missing keys", clipName, meshName);
#endif
                            }
                            else
                            {
                                auto keys = reinterpret_cast<const Keyframe*>(meshData + usedSize);
                                usedSize += sizeof(Keyframe) * clip->keys;
                                EOFCHECK
                              
#ifdef DEBUG_MODEL
                                    wprintf(L"\t\t Clip '%s' with %u keys (t: %f...%f)\n",
                                        clipName, clip->keys, double(clip->StartTime), double(clip->EndTime));
#endif

                                for (UINT k = 0; k < clip->keys; ++k)
                                {
                                    if (keys[k].BoneIndex >= *nBones)
                                    {
#ifdef DEBUG_MODEL
                                        wprintf(L"WARNING: Key %u in clip '%s' references bone not in mesh '%s' (%u, %u)\n",
                                            k, clipName, meshName, keys[k].BoneIndex, *nBones);
#endif
                                    }

                                    ciL.Keyframes.push_back(keys[k]);
                                }
                            }

                            meshL.ClipsInfo.push_back(ciL);
                    }
                }
        }
        else if (*nSkinVBs)
        {
#ifdef DEBUG_MODEL
            wprintf(L"WARNING: Found skinning verts, but no skeleton data present\n");
#endif
        }
        else
        {
#ifdef DEBUG_MODEL
            wprintf(L"\tNo animation or skinning data present.\n");
#endif
        }
    
        Meshes.push_back(meshL);
    }

    return 0;
}
