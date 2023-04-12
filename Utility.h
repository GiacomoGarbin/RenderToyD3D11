#pragma once

// windows
#include <wrl.h>
#include <comdef.h>
using Microsoft::WRL::ComPtr;

// d3d
#include <d3d11.h>

// std
#include <sstream>
#include <string>

#ifndef ThrowIfFailed
#if _DEBUG
struct Exception
{
    const HRESULT result;
    const std::wstring function;
    const std::wstring file;
    const int line;

    Exception(HRESULT result,
              const std::wstring& function,
              const std::wstring& file,
              int line)
        : result(result)
        , function(function)
        , file(file)
        , line(line)
    {}

    std::wstring ToString() const
    {
        const _com_error err(result);
        const std::wstring msg = err.ErrorMessage();

        std::wostringstream woss;
        woss << L"code: 0x" << std::hex << result << '\n';
        woss << L"message:\n" << msg << "\n\n";
        woss << L"function:\n" << function << "\n\n";
        woss << L"file: " << file << '\n';
        woss << L"line: " << std::dec << line << '\n';

        return woss.str();
    }
};

inline std::wstring AnsiToWString(const std::string& str)
{
    WCHAR buffer[512];
    MultiByteToWideChar(CP_ACP, 0, str.c_str(), -1, buffer, 512);
    return std::wstring(buffer);
}

#define ThrowIfFailed(x)                                                      \
{                                                                             \
    HRESULT hr__ = (x);                                                       \
    std::wstring wfn = AnsiToWString(__FILE__);                               \
    if (FAILED(hr__)) { throw Exception(hr__, L#x, wfn, __LINE__); } \
}
#else // _DEBUG
#define ThrowIfFailed(x) 
#endif // _DEBUG
#endif // ThrowIfFailed

void NameResource(ID3D11DeviceChild* pDeviceChild, const std::string& name);

enum class ShaderTarget
{
    VS,
    PS,
};

ComPtr<ID3DBlob> CompileShader(const std::wstring& fileName,
                               const D3D_SHADER_MACRO* defines,
                               const std::string& entryPoint,
                               const ShaderTarget target);

std::wstring ToWideString(const std::string& narrow);