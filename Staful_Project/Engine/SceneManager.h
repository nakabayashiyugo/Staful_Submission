#pragma once
#include "GameObject.h"

enum SCENE_ID
{
	SCENE_ID_PLAY = 0,
	SCENE_ID_EDIT,
	SCENE_ID_TRANSITION,
	SCENE_ID_TITLE,
};

class SceneManager
	:public GameObject
{
	SCENE_ID currentSceneID_;
	SCENE_ID nextSceneID_;
public:
	SceneManager(GameObject* parent);

	~SceneManager();

	//‰Šú‰»
	void Initialize() override;

	//XV
	void Update() override;

	//•`‰æ
	void Draw() override;

	//ŠJ•ú
	void Release() override;

	void ChangeScene(SCENE_ID _next);

	SCENE_ID GetCurrentSceneID(){ return currentSceneID_; }
};

