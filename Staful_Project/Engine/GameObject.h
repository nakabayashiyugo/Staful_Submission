#pragma once
#include <list>
#include <string>
#include "Transform.h"

class SphereCollider;

#define SAFE_DELETE(p) if(p != nullptr){ delete p; p = nullptr;}

class GameObject
{
protected:
	std::list<GameObject*>	childList_;
	Transform				transform_;
	GameObject*				pParent_;
	std::string				objectName_;
	SphereCollider*			pCollider_;

	bool isDead_;

public:
	//コンストラクタ・デストラクタ
	GameObject();
	GameObject(GameObject* parent, const std::string& name);
	~GameObject();

public:
	//基本的な奴
	virtual void Initialize() = 0;
	virtual void Update() = 0;
	void UpdateSub();
	virtual void Draw() = 0;
	void DrawSub();
	virtual void Release() = 0;
	void ReleaseSub();

	//当たり判定
	void AddCollider(SphereCollider* pCollider);
	void Collision(GameObject* pTarget);
	virtual void OnCollision(GameObject* pTarget) {};
	void RoundRobin(GameObject* pTarget);

	//子オブジェクトを追加（リストの最後へ）
	//引数：追加するオブジェクト
	void PushBackChild(GameObject* obj);

public:
	//テンプレートの定義
	template <class T>
	GameObject* Instantiate(GameObject* parent)
	{
		T* pNewObject = new T(parent);
		if (parent != nullptr)
		{
			parent->PushBackChild(pNewObject);
		}
		pNewObject->Initialize();
		return pNewObject;
	}

public:
	//transformのセッター
	void SetTransformPos(XMFLOAT3 pos);
	void SetTransformRot(XMFLOAT3 rot);
	void SetTransformSca(XMFLOAT3 sca);

	Transform GetTransform();

	std::string GetObjectName();

public:
	//オブジェクト探すためのやつ
	GameObject* FindChildObject(std::string _objName);
	GameObject* GetRootJob();
	GameObject* FindObject(std::string _objName);


	void KillMe();
};