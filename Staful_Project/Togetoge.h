#include "Engine/GameObject.h"

class Togetoge
	:public GameObject
{
	int hModel_;

	XMFLOAT3 initPos_, destPos_;
public:
	Togetoge(GameObject* parent);

	//������
	void Initialize() override;

	//�X�V
	void Update() override;

	//�`��
	void Draw() override;

	//�J��
	void Release() override;
};

