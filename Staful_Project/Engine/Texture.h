#pragma once
#include <d3d11.h>
#include <DirectXMath.h>
#include <d3dcompiler.h>
#include <wincodec.h>
#include <wrl.h>

#include "string"

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "LibFbxSDK-MD.lib")
#pragma comment(lib, "LibXml2-MD.lib")
#pragma comment(lib, "zlib-MD.lib")
#pragma comment( lib, "WindowsCodecs.lib" )

using namespace DirectX;
using std::string;

class Texture
{
	ID3D11SamplerState* pSampler_;
	ID3D11ShaderResourceView* pSRV_;
	XMFLOAT3					size_;				//画像ファイルのサイズ
public:
	Texture();
	~Texture();
	HRESULT Load(std::string fileName);
	void Release();
	ID3D11SamplerState* GetSampler();
	ID3D11ShaderResourceView* GetSRV();
	XMFLOAT3 GetSize() { return size_; }	//画像サイズの取得
};