#pragma once
#include "Engine/GameObject.h"
#include "StageOrigin.h"
class PlayScene;

enum SCENESTATE
{
	SCENE_MAPEDIT1 = 0,
	SCENE_MAPEDIT1_DELAY,
	SCENE_MAPEDIT2,
	SCENE_MAPEDIT2_DELAY,
	SCENE_STAGE1,
	SCENE_STAGE1_DELAY,
	SCENE_STAGE2,
	SCENE_STAGE2_DELAY,
	SCENE_TURNEND,
};

const int PLAYERNUM = 2;

class SceneTransition
	:public GameObject, StageOrigin
{
	bool isClear_Player_[PLAYERNUM];
	bool isFinished_;
	int turnNum_;
	int player_Num_;
	
	PlayScene* pPS_[PLAYERNUM];

	int hPlayer1_, hPlayer2_;
	int hWin_, hLose_;

	SCENESTATE sceneState_;
public:
	SceneTransition(GameObject* parent);

	//初期化
	void Initialize() override;

	//更新
	void Update() override;

	//描画
	void Draw() override;

	//開放
	void Release() override;

	int GetMathSize_x() { return XSIZE; };
	int GetMathSize_z() { return ZSIZE; };

	int GetSceneState() { return (int)sceneState_; };
	void SetSceneState(int _sceneState) { sceneState_ = (SCENESTATE)_sceneState; };

	int GetPlayerNum() { return player_Num_; }

	int GetTurnNum() { return turnNum_; };

	void SetIsClear(int _player_num, bool _isClear)
	{
		if (_player_num >= 2 && _player_num < 0)
		{
			return;
		}
		isClear_Player_[_player_num] = _isClear;
	}

	void SetIsFinished(bool _isfinished) { isFinished_ = _isfinished; }

	void Write();
};
