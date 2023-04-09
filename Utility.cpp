#include "Utility.h"

// d3d
#include <d3dcompiler.h>

void NameResource(ID3D11DeviceChild* pDeviceChild, const std::string& name)
{
#if _DEBUG
	ThrowIfFailed(pDeviceChild->SetPrivateData(WKPDID_D3DDebugObjectName,
											   UINT(name.length()),
											   name.data()));
#endif // _DEBUG
}

ComPtr<ID3DBlob> CompileShader(const std::wstring& fileName,
							   const D3D_SHADER_MACRO* defines,
							   const std::string& entryPoint,
							   const ShaderTarget target)
{
	UINT flags = 0;
#ifdef _DEBUG
	flags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif // _DEBUG

	HRESULT result = S_OK;

	ComPtr<ID3DBlob> code = nullptr;
	ComPtr<ID3DBlob> errors = nullptr;

	std::string targetName;

	switch (target)
	{
		case ShaderTarget::VS:
			targetName = "vs_5_0";
			break;
		case ShaderTarget::PS:
			targetName = "ps_5_0";
			break;
	}

	result = D3DCompileFromFile(fileName.c_str(),
								defines,
								D3D_COMPILE_STANDARD_FILE_INCLUDE,
								entryPoint.c_str(),
								targetName.c_str(),
								flags,
								0,
								&code,
								&errors);

#ifdef _DEBUG
	if (errors != nullptr)
	{
		OutputDebugStringA(static_cast<const char*>(errors->GetBufferPointer()));
	}
#endif // _DEBUG

	ThrowIfFailed(result);

	return code;
}