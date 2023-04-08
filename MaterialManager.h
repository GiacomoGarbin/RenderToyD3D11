#pragma once

// std
#include <cassert>
#include <string>
#include <unordered_map>

// d3d
#include <directxmath.h>
using namespace DirectX;

//
#include "Utility.h"

struct Material
{
    XMFLOAT4 diffuse   = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
    XMFLOAT3 fresnel   = XMFLOAT3(0.01f, 0.01f, 0.01f);
    float    roughness = 0.25f;

    //int DiffuseSRVHeapIndex = -1;
    //int NormalSRVHeapIndex = -1;

    //XMFLOAT4X4 transform = MathHelper::Identity4x4();
};

class MaterialManager
{
public:

    void Init(const ComPtr<ID3D11Device>& pDevice,
              const ComPtr<ID3D11DeviceContext>& pContext)
    {
        mDevice = pDevice;
        mContext = pContext;
    }

    void UpdateBuffer()
    {
        if (bufferSize != materials.size())
        {
            if (bufferSize < materials.size())
            {
                mBuffer.Reset();
                mBufferSRV.Reset();

                bufferSize = std::size_t(1.5f * materials.size());

                D3D11_BUFFER_DESC desc;
                desc.ByteWidth = sizeof(Material) * UINT(bufferSize);
                desc.Usage = D3D11_USAGE_DEFAULT;
                desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
                desc.CPUAccessFlags = 0;
                desc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
                desc.StructureByteStride = sizeof(Material);

                ThrowIfFailed(mDevice->CreateBuffer(&desc, nullptr, &mBuffer));
                NameResource(mBuffer.Get(), "MaterialsSB");

                ThrowIfFailed(mDevice->CreateShaderResourceView(mBuffer.Get(), nullptr, &mBufferSRV));
                NameResource(mBufferSRV.Get(), "MaterialsBufferSRV");
            }

            D3D11_BOX box;
            box.left = 0;
            box.top = 0;
            box.front = 0;
            box.right = sizeof(Material) * UINT(materials.size());
            box.bottom = 1;
            box.back = 1;

            mContext->UpdateSubresource(mBuffer.Get(), 0, &box, materials.data(), 0, 0);
        }
    }

    static void AddMaterial(const std::string& name,
                            const Material& material)
    {
        assert(!lookup.contains(name));

        materials.push_back(material);
        lookup[name] = materials.size() - 1;
    }

    static const Material& GetMaterial(const std::size_t i)
    {
        assert(i < materials.size());

        return materials[i];
    }

    static const std::vector<Material>& GetMaterials()
    {
        return materials;
    }

    ID3D11ShaderResourceView* GetBufferSRV()
    {
        return mBufferSRV.Get();
    }

    ID3D11ShaderResourceView** GetAddressOfBufferSRV()
    {
        return mBufferSRV.GetAddressOf();
    }

private:

    static std::unordered_map<std::string, std::size_t> lookup;
    static std::vector<Material> materials;

    ComPtr<ID3D11Device> mDevice;
    ComPtr<ID3D11DeviceContext> mContext;
    
    ComPtr<ID3D11Buffer> mBuffer;
    ComPtr<ID3D11ShaderResourceView> mBufferSRV;

    static std::size_t bufferSize;
};