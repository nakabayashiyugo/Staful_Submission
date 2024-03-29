#pragma once
#include <DirectXMath.h>

using namespace DirectX;

//位置、向き、拡大率などを管理するクラス
class Transform
{
	XMMATRIX matTranslate_;	//移動行列
	XMMATRIX matRotate_;	//回転行列	
	XMMATRIX matScale_;	//拡大行列

public:
	XMFLOAT3 position_;	//位置
	XMFLOAT3 rotate_;	//向き
	XMFLOAT3 scale_;	//拡大率

	Transform* pParent_; //親のオブジェクトのTransform

	//コンストラクタ
	Transform();

	//デストラクタ
	~Transform();

	//各行列の計算
	void Calclation();

	//ワールド行列を取得
	XMMATRIX GetWorldMatrix();

	XMMATRIX GetNormalMatrix();
};

inline XMFLOAT3 operator += (XMFLOAT3& p, const XMVECTOR& v)
{
	XMVECTOR pv = XMLoadFloat3(&p);
	pv += v;
	XMStoreFloat3(&p, pv);
	return p;
}

