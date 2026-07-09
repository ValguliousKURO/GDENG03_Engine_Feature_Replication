#pragma once
#include <d3dcompiler.h>
#include <fstream>

class ShaderInclude : public ID3DInclude
{
public:
	ShaderInclude() {}
	virtual HRESULT Open(D3D_INCLUDE_TYPE IncludeType, LPCSTR pFileName,
		LPCVOID pParentData, LPCVOID* ppData, UINT* pBytes)
	{
		std::ifstream shaderStream(pFileName);
		if (!shaderStream) return E_FAIL;
		std::string shaderCode{
			std::istreambuf_iterator<char>(shaderStream),
			std::istreambuf_iterator<char>()
		};
		char* shaderCodePtr = new char[shaderCode.size() + 1];
		memcpy(shaderCodePtr, shaderCode.c_str(), shaderCode.size() + 1);
		*ppData = shaderCodePtr;
		*pBytes = static_cast<UINT>(shaderCode.size());
		return S_OK;
	}
	virtual HRESULT Close(LPCVOID pData)
	{
		delete[] static_cast<const char*>(pData);
		return S_OK;
	}
};