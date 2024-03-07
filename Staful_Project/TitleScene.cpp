#include "TitleScene.h"
#include "SceneTransition.h"
#include "Engine/Image.h"
#include "Engine/Input.h"
#include "Engine/Direct3D.h"
#include "Engine/SceneManager.h"

TitleScene::TitleScene(GameObject* parent)
	: GameObject(parent, "TitleScene"), hStartButton_(-1), hStaful_(-1),
	mousePos_(0, 0, 0)
{
}

void TitleScene::Initialize()
{
	hStartButton_ = Image::Load("Assets\\Logo_StartButton.png");
	assert(hStartButton_ >= 0);
	hStaful_ = Image::Load("Assets\\Logo_Staful.png");
	assert(hStaful_ >= 0);
}

void TitleScene::Update()
{
	mousePos_ = Input::GetMousePosition();
	
	mousePos_.x = mousePos_.x - (Direct3D::scrWidth / 2);
	mousePos_.y = mousePos_.y - (Direct3D::scrHeight / 2);

	float SBRight = tStartButton_.position_.x * (Direct3D::scrWidth / 2) + (Direct3D::scrWidth * tStartButton_.scale_.x / 2);

	float SBLeft = tStartButton_.position_.x * (Direct3D::scrWidth / 2) - (Direct3D::scrWidth * tStartButton_.scale_.x / 2);

	float SBUp = (tStartButton_.position_.y * (Direct3D::scrHeight / 2) + (Direct3D::scrHeight * tStartButton_.scale_.y / 2)) * -1;

	float SBDown = (tStartButton_.position_.y * (Direct3D::scrHeight / 2) - (Direct3D::scrHeight * tStartButton_.scale_.y / 2)) * -1;

	if (mousePos_.x >= SBLeft && mousePos_.x <= SBRight &&
		mousePos_.y >= SBUp && mousePos_.y <= SBDown)
	{
		if (Input::IsMuoseButtonUp(0))
		{
			SceneManager* pSceneManager = (SceneManager*)FindObject("SceneManager");
			pSceneManager->ChangeScene(SCENE_ID_TRANSITION);
		}
	}
	else
	{
		std::string resStr = std::to_string((float)mousePos_.x) + ", " + std::to_string(mousePos_.y) + "\n";
		OutputDebugString(resStr.c_str());
	}
}

void TitleScene::Draw()
{
	tStartButton_.position_.y = -0.1f;
	tStartButton_.scale_ = XMFLOAT3(0.3f, 0.1f, 1);
	tStaful_.position_.y = 0.5f;
	tStaful_.scale_ = XMFLOAT3(0.6f, 0.4f, 1);

	Image::SetTransform(hStartButton_, tStartButton_);
	Image::SetTransform(hStaful_, tStaful_);
	Image::Draw(hStartButton_);
	Image::Draw(hStaful_);
}

void TitleScene::Release()
{
}