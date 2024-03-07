#include "Engine/GameObject.h"

class Togetoge
	:public GameObject
{
	int hModel_;

	XMFLOAT3 initPos_, destPos_;
public:
	Togetoge(GameObject* parent);

	//‰Šú‰»
	void Initialize() override;

	//XV
	void Update() override;

	//•`‰æ
	void Draw() override;

	//ŠJ•ú
	void Release() override;
};

