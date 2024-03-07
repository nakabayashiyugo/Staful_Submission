#pragma once
#include <DirectXMath.h>
#include <vector>
#include "Direct3D.h"
#include "Camera.h"
#include "Texture.h"
#include "Transform.h"

using namespace DirectX;

class Sprite
{
	//�R���X�^���g�o�b�t�@�[
	struct CONSTANT_BUFFER
	{
		XMMATRIX	matW;
		XMMATRIX	uvTrans;
		XMMATRIX	color;
	};

	struct VERTEX
	{
		XMVECTOR position;
		XMVECTOR uv;
	};
protected:
	UINT64 vertexNum_;
	std::vector<VERTEX> vertices_;
	ID3D11Buffer* pVertexBuffer_;	//���_�o�b�t�@

	UINT64 indexNum_;
	std::vector<int> index_;

	ID3D11Buffer* pIndexBuffer_;
	ID3D11Buffer* pConstantBuffer_;	//�R���X�^���g�o�b�t�@

	Texture* pTexture_;

	XMFLOAT3 texture_size_;
public:
	/// <summary>
	///  �l�p�`��\���N���X�̃R���X�g���N�^
	/// </summary>
	Sprite();
	~Sprite();
	HRESULT Load(std::string filename);
	void Draw(Transform& transform, RECT rect);
	void Release();
	XMFLOAT3 GetTextureSize() { return texture_size_; }

private:
	virtual void InitVertexData();

	HRESULT CreateVertexBuffer();

	virtual void InitIndexData();
	HRESULT CreateIndexBuffer();

	HRESULT CreateConstantBuffer();

	void PassDataToCB(Transform& transform, RECT rect);
	void SetBufferToPipeline();
};

