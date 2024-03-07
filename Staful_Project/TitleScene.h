#pragma once
#include "Engine/GameObject.h"

class TitleScene
	:public GameObject
{
	int hStartButton_, hStaful_;

	Transform tStartButton_, tStaful_;
	XMFLOAT3 mousePos_;
public:
	TitleScene(GameObject* parent);

	//������
	void Initialize() override;

	//�X�V
	void Update() override;

	//�`��
	void Draw() override;

	//�J��
	void Release() override;
};