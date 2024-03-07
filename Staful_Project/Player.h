#pragma once
#include "Engine/GameObject.h"
#include "StageOrigin.h"

class PlayScene;
class Timer;

enum PLAYER_STATE
{
	STATE_IDLE = 0,
	STATE_WALK,
	STATE_JAMP,
	STATE_FALL,
	STATE_DEAD,
};

enum STAGE_STATE
{
	STATE_START = 0,
	STATE_PLAY,
	STATE_GOAL,
	STATE_FAILURE,
};


class Player
	:public GameObject, StageOrigin
{
	int hModel_;
	int hFrame_, hGage_, hFrameOutline_;
	int hTime_;

	Transform tFrame_, tGage_, tFrameOutline_;
	Transform tTime_;

	bool isGoal_;

	PlayScene* pPlayScene_;

	XMVECTOR sub_velocity_, velocity_, jamp_start_velocity_;
	XMVECTOR eyeDirection_;

	XMFLOAT3 tableHitPoint_;
	bool isTableHit_;

	XMFLOAT3 startPos_, goalPos_;
	XMFLOAT3 centerPos_;
	XMFLOAT3 camRot_;


	PLAYER_STATE playerState_;
	PLAYER_STATE prevPlayerState_;
	STAGE_STATE stageState;
	//�A�j���[�V�����̃t���[��
	int nowFrame_, startFrame_, endFrame_;

	MATHDEDAIL standMath_;

	//�d��
	XMFLOAT3 gravity_;

	//�󒆂ł̃X�s�[�h����
	int air_dec_velocity_;

	//��Q���̒u���鐔
	int hurdle_Limit_;

	//�^�C�}�[
	Timer* pTimer_;
public:
	Player(GameObject* parent);

	//������
	void Initialize() override;

	//�X�V
	void Update() override;

	//�`��
	void Draw() override;

	//�J��
	void Release() override;

	void PlayUpdate();

	void IdleUpdate();
	void WalkUpdate();
	void JampUpdate();
	void FallUpdate();
	void DeadUpdate();

	void SetAnimFramerate();

	bool Is_InSide_Table(XMFLOAT3 _pos);

	void PlayerOperation();

	bool Is_Goal() { return isGoal_; }

	XMFLOAT3 GetPosition() { return centerPos_; }

	MATHDEDAIL SetStandMath(XMFLOAT3 _pos);
	MATHDEDAIL HollCheck(XMFLOAT3 _pos);
	void WallCheck(XMFLOAT3 _pos);

	bool GetFailed() {
		if (stageState == STATE_FAILURE)
		{
			return true;
		}
		return false;
	}

	void OnCollision(GameObject* pTarget) override;
};