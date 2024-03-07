#include "SceneManager.h"
#include "../TitleScene.h"
#include "../PlayScene.h"
#include "../MapEditScene.h"
#include "../SceneTransition.h"
#include "Model.h"

SceneManager::SceneManager(GameObject* parent)
	: GameObject(parent, "SceneManager")
{
}

SceneManager::~SceneManager()
{
}

void SceneManager::Initialize()
{
	currentSceneID_ = SCENE_ID_TITLE;
	nextSceneID_ = currentSceneID_;
	Instantiate<TitleScene>(this);
}

void SceneManager::Update()
{
	if (currentSceneID_ != nextSceneID_)
	{
		//���̃V�[���̃I�u�W�F�N�g��S�폜
		auto scene = childList_.begin();
		(*scene)->ReleaseSub();
		SAFE_DELETE(*scene);
		childList_.clear();

		Model::Release();
		//��Еt���I��

		switch (nextSceneID_)
		{
		case SCENE_ID_PLAY:
			Instantiate<PlayScene>(this);
				break;
		case SCENE_ID_EDIT:
			Instantiate<MapEditScene>(this);
			break;
		case SCENE_ID_TRANSITION:
			Instantiate<SceneTransition>(this);
			break;
		case SCENE_ID_TITLE:
			Instantiate<TitleScene>(this);
			break;
		}
		currentSceneID_ = nextSceneID_;
	}
}

void SceneManager::Draw()
{
}

void SceneManager::Release()
{
}

void SceneManager::ChangeScene(SCENE_ID _next)
{
	nextSceneID_ = _next;
}
