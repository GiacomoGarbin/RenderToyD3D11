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
    XMFLOAT4 diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
    XMFLOAT3 fresnel = XMFLOAT3(0.01f, 0.01f, 0.01f);
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
        if (mMaterialCount != mMaterials.size())
        {
            if (mMaterialCount < mMaterials.size())
            {
                mBuffer.Reset();
                mBufferSRV.Reset();

                mMaterialCount = std::size_t(1.5f * mMaterials.size());

                D3D11_BUFFER_DESC desc;
                desc.ByteWidth = sizeof(Material) * UINT(mMaterialCount);
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
            box.right = sizeof(Material) * UINT(mMaterials.size());
            box.bottom = 1;
            box.back = 1;

            mContext->UpdateSubresource(mBuffer.Get(), 0, &box, mMaterials.data(), 0, 0);
        }
    }

    std::size_t AddMaterial(const std::string& name,
                     const Material& material)
    {
        assert(!mLookup.contains(name));

        mMaterials.push_back(material);
        mLookup[name] = mMaterials.size() - 1;

        return mMaterials.size() - 1;
    }

    const Material& GetMaterial(const std::size_t i) const
    {
        assert(i < mMaterials.size());

        return mMaterials[i];
    }

    const std::vector<Material>& GetMaterials() const
    {
        return mMaterials;
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

    std::unordered_map<std::string, std::size_t> mLookup;
    std::vector<Material> mMaterials;

    ComPtr<ID3D11Device> mDevice;
    ComPtr<ID3D11DeviceContext> mContext;

    ComPtr<ID3D11Buffer> mBuffer;
    ComPtr<ID3D11ShaderResourceView> mBufferSRV;

    std::size_t mMaterialCount;
};