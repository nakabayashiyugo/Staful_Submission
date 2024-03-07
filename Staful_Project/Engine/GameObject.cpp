#include "GameObject.h"
#include "SphereCollider.h"

GameObject::GameObject()
	:pParent_(nullptr)
{
}

GameObject::GameObject(GameObject* parent, const std::string& name)
	:pParent_(parent), objectName_(name), isDead_(false), pCollider_(nullptr)
{
	if (parent != nullptr)
		this->transform_.pParent_ = &(parent->transform_);
}

GameObject::~GameObject()
{
}

void GameObject::UpdateSub()
{
	Update();

	RoundRobin(GetRootJob());
	for (auto itr = childList_.begin(); itr != childList_.end(); itr++)
	{
		(*itr)->UpdateSub();
	}

	for (auto itr = childList_.begin(); itr != childList_.end();)
	{
		if ((*itr)->isDead_)
		{
			(*itr)->ReleaseSub();
			SAFE_DELETE(*itr);
			itr = childList_.erase(itr);
		}
		else
		{
			itr++;
		}
	}
}

void GameObject::DrawSub()
{
	Draw();
	for (auto itr = childList_.begin(); itr != childList_.end(); itr++)
	{
		(*itr)->DrawSub();
	}
}

void GameObject::ReleaseSub()
{
	for (auto itr = childList_.begin(); itr != childList_.end(); itr++)
	{
		(*itr)->ReleaseSub();
		SAFE_DELETE(*itr);
	}
	Release();
}

void GameObject::AddCollider(SphereCollider* pCollider)
{
	this->pCollider_ = pCollider;
}

void GameObject::Collision(GameObject* pTarget)
{
	if (pTarget == this || pTarget->pCollider_ == nullptr)
		return;

	float Length = (this->transform_.position_.x - pTarget->transform_.position_.x) * (this->transform_.position_.x - pTarget->transform_.position_.x)
				 + (this->transform_.position_.y - pTarget->transform_.position_.y) * (this->transform_.position_.y - pTarget->transform_.position_.y)
				 + (this->transform_.position_.z - pTarget->transform_.position_.z) * (this->transform_.position_.z - pTarget->transform_.position_.z);

	float rLength = (this->pCollider_->GetRadius() + pTarget->pCollider_->GetRadius()) * (this->pCollider_->GetRadius() + pTarget->pCollider_->GetRadius());

	if (Length <= rLength)
	{
		OnCollision(pTarget);
	}
}

void GameObject::RoundRobin(GameObject* pTarget)
{
	if (pCollider_ == nullptr)
		return;
	if (pTarget->pCollider_ != nullptr)
		Collision(pTarget);
	for (auto itr : pTarget->childList_)
	{
		RoundRobin(itr);
	}
}

//子オブジェクトを追加（リストの最後へ）
void GameObject::PushBackChild(GameObject* obj)
{
	assert(obj != nullptr);

	obj->pParent_ = this;
	childList_.push_back(obj);
}
GameObject* GameObject::FindChildObject(std::string _objName)
{
	if (_objName == this->objectName_)
	{
		return (this); //自分が_objNameだった!!
	}
	else
	{
		for (auto itr:childList_)
		{
			GameObject* obj = itr->FindChildObject(_objName);
			if (obj != nullptr)
			{
				return obj;
			}
		}
	}

	return nullptr;
}

GameObject* GameObject::GetRootJob()
{
	if (pParent_ == nullptr)
	{
		return this;
	}
	return pParent_->GetRootJob();
}

GameObject* GameObject::FindObject(std::string _objName)
{
	return this->GetRootJob()->FindChildObject(_objName);
}

void GameObject::KillMe()
{
	isDead_ = true;
}

void GameObject::SetTransformPos(XMFLOAT3 pos)
{
	transform_.position_ = pos;
}

void GameObject::SetTransformRot(XMFLOAT3 rot)
{
	transform_.rotate_ = rot;
}

void GameObject::SetTransformSca(XMFLOAT3 sca)
{
	transform_.scale_ = sca;
}

Transform GameObject::GetTransform()
{
	return transform_;
}

std::string GameObject::GetObjectName()
{
	return objectName_;
}

