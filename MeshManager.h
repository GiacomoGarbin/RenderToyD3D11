#pragma once

// std
#include <unordered_map>
#include <vector>

// d3d
#include <directxmath.h>
using namespace DirectX;

//
#include "Utility.h"

struct VertexData
{
	XMFLOAT3 position;
	XMFLOAT3 normal;
	XMFLOAT3 tangent;
	XMFLOAT2 uv;

	VertexData();
	VertexData(float px, float py, float pz,
			   float nx, float ny, float nz,
			   float u,  float v,
			   float tx, float ty, float tz);
};

struct MeshData
{
	using IndexType = uint16_t;

	std::vector<VertexData> vertices;
	std::vector<IndexType> indices;

	UINT indexStart;
	UINT indexCount;
	UINT vertexBase;
};

class MeshManager
{
public:

	void Init(const ComPtr<ID3D11Device>& pDevice,
			  const ComPtr<ID3D11DeviceContext>& pContext)
	{
		mDevice = pDevice;
		mContext = pContext;
	}

	void UpdateBuffers()
	{
		if (mMeshCount != mMeshes.size())
		{
			mVertexBuffer.Reset();
			mIndexBuffer.Reset();

			std::vector<VertexData> vertices;
			std::vector<MeshData::IndexType> indices;

			for (const MeshData& mesh : mMeshes)
			{
				vertices.insert(vertices.end(), mesh.vertices.begin(), mesh.vertices.end());
				indices.insert(indices.end(), mesh.indices.begin(), mesh.indices.end());
			}

			assert(vertices.size() == mVertexCount);
			assert(indices.size() == mIndexCount);

			// vertex buffer
			{
				D3D11_BUFFER_DESC desc;
				desc.ByteWidth = sizeof(VertexData) * mVertexCount;
				desc.Usage = D3D11_USAGE_IMMUTABLE;
				desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
				desc.CPUAccessFlags = 0;
				desc.MiscFlags = 0;
				desc.StructureByteStride = 0;

				D3D11_SUBRESOURCE_DATA data;
				data.pSysMem = vertices.data();
				data.SysMemPitch = 0;
				data.SysMemSlicePitch = 0;

				ThrowIfFailed(mDevice->CreateBuffer(&desc, &data, &mVertexBuffer));

				NameResource(mVertexBuffer.Get(), "VertexBuffer");
			}

			// index buffer
			{
				D3D11_BUFFER_DESC desc;
				desc.ByteWidth = sizeof(MeshData::IndexType) * mIndexCount;
				desc.Usage = D3D11_USAGE_IMMUTABLE;
				desc.BindFlags = D3D11_BIND_INDEX_BUFFER;
				desc.CPUAccessFlags = 0;
				desc.MiscFlags = 0;
				desc.StructureByteStride = 0;

				D3D11_SUBRESOURCE_DATA data;
				data.pSysMem = indices.data();
				data.SysMemPitch = 0;
				data.SysMemSlicePitch = 0;

				ThrowIfFailed(mDevice->CreateBuffer(&desc, &data, &mIndexBuffer));

				NameResource(mIndexBuffer.Get(), "IndexBuffer");
			}
		}
	}

	std::size_t AddMesh(const std::string& name, MeshData& mesh)
	{
		assert(!mLookup.contains(name));

		mesh.indexStart = mIndexCount;
		mesh.indexCount = UINT(mesh.indices.size());
		mesh.vertexBase = mVertexCount;

		mMeshes.push_back(mesh);
		mLookup[name] = mMeshes.size() - 1;

		mVertexCount += UINT(mesh.vertices.size());
		mIndexCount += UINT(mesh.indices.size());

		return mMeshes.size() - 1;
	}

	const MeshData& GetMesh(const std::size_t i) const
	{
		assert(i < mMeshes.size());

		return mMeshes[i];
	}

	ID3D11Buffer** GetAddressOfVertexBuffer()
	{
		return mVertexBuffer.GetAddressOf();
	}

	ID3D11Buffer* GetIndexBuffer()
	{
		return mIndexBuffer.Get();
	}

	DXGI_FORMAT GetIndexBufferFormat()
	{
		return mIndexBufferFormat;
	}

	static MeshData CreateBox(const float width, const float height, const float depth);

private:

	std::unordered_map<std::string, std::size_t> mLookup;
	std::vector<MeshData> mMeshes;

	std::size_t mMeshCount = 0;

	ComPtr<ID3D11Device> mDevice;
	ComPtr<ID3D11DeviceContext> mContext;

	ComPtr<ID3D11Buffer> mVertexBuffer;
	ComPtr<ID3D11Buffer> mIndexBuffer;
	DXGI_FORMAT mIndexBufferFormat = DXGI_FORMAT_R16_UINT;

	UINT mVertexCount = 0;
	UINT mIndexCount = 0;
};