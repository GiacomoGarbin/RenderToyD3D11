#pragma once

// windows
#include <wrl.h>
#include <comdef.h>
using Microsoft::WRL::ComPtr;

// d3d
#include <d3d11.h>
#include <directxmath.h>
using namespace DirectX;

// std
#include <cassert>
#include <string>
#include <unordered_map>

//
#include "Utility.h"

struct Object
{
    Object()
        : mesh(-1)
        , material(-1)
    {
        XMStoreFloat4x4(&world, XMMatrixIdentity());
        XMStoreFloat4x4(&uvTransform, XMMatrixIdentity());
    }

    std::size_t mesh;
    std::size_t material;
    XMFLOAT4X4  world;
    XMFLOAT4X4  uvTransform;

    // raster state
    // depth stencil state
    // blend state
    // stencil ref
};

class ObjectManager
{
public:

    void Init(const ComPtr<ID3D11Device>& pDevice,
              const ComPtr<ID3D11DeviceContext>& pContext)
    {
        mDevice = pDevice;
        mContext = pContext;

        D3D11_BUFFER_DESC desc;
        desc.ByteWidth = sizeof(ObjectCB);
        desc.Usage = D3D11_USAGE_DEFAULT;
        desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
        desc.CPUAccessFlags = 0;
        desc.MiscFlags = 0;
        desc.StructureByteStride = 0;

        ThrowIfFailed(mDevice->CreateBuffer(&desc, nullptr, &mBuffer));

        NameResource(mBuffer.Get(), "ObjectCB");
    }

    void UpdateBuffer(const std::size_t i)
    {
        const Object& object = GetObject(i);

        ObjectCB buffer;
        buffer.world    = object.world;
        buffer.uvTransform = object.uvTransform;
        buffer.material = UINT(object.material);

        mContext->UpdateSubresource(mBuffer.Get(), 0, nullptr, &buffer, 0, 0);
    }

    ID3D11Buffer** GetAddressOfBuffer()
    {
        return mBuffer.GetAddressOf();
    }

    std::size_t AddObject(const Object& object)
    {
        mObjects.push_back(object);
        return mObjects.size() - 1;
    }

    const Object& GetObject(const std::size_t i) const
    {
        assert(i < mObjects.size());

        return mObjects[i];
    }

    const std::vector<Object>& GetObjects() const
    {
        return mObjects;
    }

private:

    std::vector<Object> mObjects;

    struct ObjectCB
    {
        ObjectCB()
        {
            XMStoreFloat4x4(&world, XMMatrixIdentity());
            XMStoreFloat4x4(&uvTransform, XMMatrixIdentity());
        }

        XMFLOAT4X4 world;
        XMFLOAT4X4 uvTransform;
        UINT material = -1;
        XMFLOAT3 padding;
    };

    static_assert((sizeof(ObjectCB) % 16) == 0, "constant buffer size must be 16-byte aligned");

    ComPtr<ID3D11Buffer> mBuffer;

    ComPtr<ID3D11Device> mDevice;
    ComPtr<ID3D11DeviceContext> mContext;
};