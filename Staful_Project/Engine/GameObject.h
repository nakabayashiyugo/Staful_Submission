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
	//�R���X�g���N�^�E�f�X�g���N�^
	GameObject();
	GameObject(GameObject* parent, const std::string& name);
	~GameObject();

public:
	//��{�I�ȓz
	virtual void Initialize() = 0;
	virtual void Update() = 0;
	void UpdateSub();
	virtual void Draw() = 0;
	void DrawSub();
	virtual void Release() = 0;
	void ReleaseSub();

	//�����蔻��
	void AddCollider(SphereCollider* pCollider);
	void Collision(GameObject* pTarget);
	virtual void OnCollision(GameObject* pTarget) {};
	void RoundRobin(GameObject* pTarget);

	//�q�I�u�W�F�N�g��ǉ��i���X�g�̍Ō�ցj
	//�����F�ǉ�����I�u�W�F�N�g
	void PushBackChild(GameObject* obj);

public:
	//�e���v���[�g�̒�`
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
	//transform�̃Z�b�^�[
	void SetTransformPos(XMFLOAT3 pos);
	void SetTransformRot(XMFLOAT3 rot);
	void SetTransformSca(XMFLOAT3 sca);

	Transform GetTransform();

	std::string GetObjectName();

public:
	//�I�u�W�F�N�g�T�����߂̂��
	GameObject* FindChildObject(std::string _objName);
	GameObject* GetRootJob();
	GameObject* FindObject(std::string _objName);


	void KillMe();
};