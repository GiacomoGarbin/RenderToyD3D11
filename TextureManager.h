#pragma once

// std
#include <cassert>
#include <unordered_map>
#include <vector>

// d3d
#include <d3d11.h>
#include "DDSTextureLoader11.h"
using namespace DirectX;

//
#include "Utility.h"

class TextureManager
{
public:

	void Init(const ComPtr<ID3D11Device>& pDevice,
			  const ComPtr<ID3D11DeviceContext>& pContext)
	{
		mDevice = pDevice;
		mContext = pContext;
	}

	std::size_t LoadTexture(const std::string& name)
	{
		assert(!mLookup.contains(name));

		ComPtr<ID3D11Resource> pTexture;
		ComPtr<ID3D11ShaderResourceView> pSRV;

		ThrowIfFailed(CreateDDSTextureFromFile(mDevice.Get(),
											   ToWideString(name).c_str(),
											   &pTexture,
											   &pSRV,
											   0,
											   nullptr));

		//NameResource(pTexture.Get(), name);

		mTextures.push_back(pTexture);
		mSRVs.push_back(pSRV);

		mLookup[name] = mTextures.size() - 1;

		return mTextures.size() - 1;
	}

	std::size_t LoadTexturesIntoTexture2DArray(const std::string& name,
											   const std::vector<std::string>& paths)
	{
		assert(!mLookup.contains(name));

		std::vector<ComPtr<ID3D11Resource>> resources;
		resources.resize(paths.size());

		D3D11_TEXTURE2D_DESC desc;

		for (std::size_t i = 0; i < paths.size(); ++i)
		{
			ThrowIfFailed(CreateDDSTextureFromFile(mDevice.Get(),
												   ToWideString(paths[i]).c_str(),
												   &resources[i],
												   nullptr,
												   0,
												   nullptr));

			D3D11_RESOURCE_DIMENSION dimension;
			resources[i]->GetType(&dimension);

			assert(dimension == D3D11_RESOURCE_DIMENSION_TEXTURE2D);

			ID3D11Texture2D* pTexture2D = static_cast<ID3D11Texture2D*>(resources[i].Get());

			D3D11_TEXTURE2D_DESC temp;
			pTexture2D->GetDesc((i == 0) ? &desc : &temp);

			if (i != 0)
			{
				assert(std::memcmp(&desc, &temp, sizeof(D3D11_TEXTURE2D_DESC)) == 0);
			}
		}

		ComPtr<ID3D11Texture2D> pTextureArray;
		ComPtr<ID3D11ShaderResourceView> pTextureArraySRV;

		desc.ArraySize = UINT(resources.size());
		desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;

		ThrowIfFailed(mDevice->CreateTexture2D(&desc, nullptr, &pTextureArray));
		NameResource(pTextureArray.Get(), name);

		for (UINT arraySlice = 0; arraySlice < desc.ArraySize; ++arraySlice)
		{
			const ComPtr<ID3D11Resource>& resource = resources[arraySlice];

			for (UINT mipSlice = 0; mipSlice < desc.MipLevels; ++mipSlice)
			{
				//D3D11_MAPPED_SUBRESOURCE mapped;
				//ThrowIfFailed(mContext->Map(resource.Get(), mipSlice, D3D11_MAP_READ, 0, &mapped));

				const UINT subresource = D3D11CalcSubresource(mipSlice,
															  arraySlice,
															  desc.MipLevels);

				//mContext->UpdateSubresource(pTextureArray.Get(),
				//							subresource,
				//							nullptr,
				//							mapped.pData,
				//							mapped.RowPitch,
				//							mapped.DepthPitch);

				//mContext->Unmap(resource.Get(), mipSlice);

				mContext->CopySubresourceRegion(pTextureArray.Get(),
												subresource,
												0,
												0,
												0,
												resource.Get(),
												mipSlice,
												nullptr);
			}

		}

		mDevice->CreateShaderResourceView(pTextureArray.Get(), nullptr, &pTextureArraySRV);
		NameResource(pTextureArraySRV.Get(), name + "SRV");

		mTextures.push_back(pTextureArray);
		mSRVs.push_back(pTextureArraySRV);

		mLookup[name] = mTextures.size() - 1;

		return mTextures.size() - 1;
	}

	ID3D11Resource* GetTexture(const std::size_t i)
	{
		assert(i < mTextures.size());
		return mTextures[i].Get();
	}

	ID3D11ShaderResourceView* GetSRV(const std::size_t i)
	{
		assert(i < mSRVs.size());
		return mSRVs[i].Get();
	}

private:

	ComPtr<ID3D11Device> mDevice;
	ComPtr<ID3D11DeviceContext> mContext;

	std::unordered_map<std::string, std::size_t> mLookup;

	std::vector<ComPtr<ID3D11Resource>> mTextures;
	std::vector<ComPtr<ID3D11ShaderResourceView>> mSRVs;
};