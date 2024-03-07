#include "Player.h"

#include "Engine/Input.h"
#include "Engine/Camera.h"
#include "Engine/Model.h"
#include "Engine/Image.h"
#include "Engine/Fbx.h"
#include "Engine/SphereCollider.h"

#include "PlayScene.h"
#include "SceneTransition.h"
#include "Stage.h"
#include "Timer.h"
#include "StageOrigin.h"

const float MODELSIZE = 0.8f;
const float AIR_DEC_VELOCITY_INIT = 1.0f;
const float AIR_DEC_VELOCITY = 2.0f;

Player::Player(GameObject* parent)
	: GameObject(parent, "Player"), 
	hModel_(-1),
	velocity_(XMVectorSet(0, 0, 0, 0)), sub_velocity_(XMVectorSet(0, 0, 0, 0)), 
	jamp_start_velocity_(XMVectorSet(0, 0, 0, 0)), eyeDirection_(XMVectorSet(0, 0, 1, 0)),
	camRot_(0, 0, 0), gravity_(0, 0, 0), 
	playerState_(STATE_IDLE), prevPlayerState_(STATE_DEAD), stageState(STATE_START),
	air_dec_velocity_(1),
	hurdle_Limit_(0),
	tableHitPoint_(XMFLOAT3(0, 0, 0)), isTableHit_(false),
	hFrame_(-1), hFrameOutline_(-1), hGage_(-1), hTime_(-1),
	isGoal_(false)
{
	pTrans_ = (SceneTransition*)FindObject("SceneTransition");
	XSIZE = (int)pTrans_->GetMathSize_x();
	ZSIZE = (int)pTrans_->GetMathSize_z();

	Math_Resize(XSIZE, ZSIZE, &math_);

	pPlayScene_ = (PlayScene*)FindObject("PlayScene");

	SetTableMath(pPlayScene_->GetTableMath());

	for (int x = 0; x < XSIZE; x++)
	{
		for (int z = 0; z < ZSIZE; z++)
		{
			if (math_[x][z].mathType_ == MATH_START)
			{
				startPos_ = XMFLOAT3((float)x, 1.0f, (float)z);
			}
			if (math_[x][z].mathType_ == MATH_GOAL)
			{
				goalPos_ = XMFLOAT3((float)x, 1.0f, (float)z);
			}
		}
	}
}

void Player::Initialize()
{
	std::string fileName = "Assets\\Player";
	fileName += std::to_string(pPlayScene_->GetPlayerNum() + 1) + ".fbx";
	hModel_ = Model::Load(fileName);
	assert(hModel_ >= 0);

	hFrame_ = Image::Load("Assets\\Timer_Frame.png");
	assert(hFrame_ >= 0);
	hFrameOutline_ = Image::Load("Assets\\Timer_FrameOutline.png");                                                                          
	assert(hFrameOutline_ >= 0);
	hGage_ = Image::Load("Assets\\Timer_Gage.png");
	assert(hGage_ >= 0);
	hTime_ = Image::Load("Assets\\Logo_TIME.png");
	assert(hTime_ >= 0);
	
	pTimer_ = new Timer(30);
}

void Player::Update()
{
	Stage* pStage = (Stage*)FindObject("Stage");
	
	switch (stageState)
	{
	case STATE_START:
		transform_.position_ = startPos_;
		velocity_ = sub_velocity_ = XMVectorSet(0, 0, 0, 0);
		stageState = STATE_PLAY;
		playerState_ = STATE_IDLE;
		break;
	case STATE_PLAY:
		PlayUpdate();
		break;
	case STATE_GOAL:
		isGoal_ = true;
		break;
	}

	if (pPlayScene_->GetTableChange())
	{
		SetTableMath(pPlayScene_->GetTableMath());
		pPlayScene_->SetTableChange(false);
	}
}

void Player::Draw()
{ 
	//transform_.scale_ = XMFLOAT3(0.5f, 0.5f, 0.5f);
	
	Model::SetTransform(hModel_, transform_);
	Model::Draw(hModel_);

	//時間ゲージ
	XMFLOAT3 timerPos = XMFLOAT3(-0.6f, 0.8f, 0);
	tFrame_.position_ = XMFLOAT3(timerPos.x, timerPos.y, 0);
	tFrameOutline_.position_ = XMFLOAT3(timerPos.x, timerPos.y, 0);
	tGage_.position_ =
		XMFLOAT3(-((0.3f / pTimer_->GetLimitTime()) * pTimer_->GetCurTime()) + timerPos.x,
			timerPos.y, 0);
	tFrame_.scale_ = XMFLOAT3(0.3f, 0.1f, 1);
	tGage_.scale_ = XMFLOAT3(float(pTimer_->GetLimitTime() - pTimer_->GetCurTime()) / 100 * (0.3f / (float(pTimer_->GetLimitTime()) / 100))
		, 0.1f, 1);
	tFrameOutline_.scale_ = XMFLOAT3(0.31f, 0.11f, 1);

	Image::SetTransform(hGage_, tGage_);
	Image::SetTransform(hFrame_, tFrame_);
	Image::SetTransform(hFrameOutline_, tFrameOutline_);
	Image::Draw(hGage_);
	Image::Draw(hFrame_);
	Image::Draw(hFrameOutline_);


	tTime_.position_ = XMFLOAT3(-0.1f, 0.8f, 0);
	tTime_.scale_ = XMFLOAT3(0.2f, 0.1f, 1);
	Image::SetTransform(hTime_, tTime_);
	Image::Draw(hTime_);
}

void Player::Release()
{
}

void Player::PlayUpdate()
{
	SphereCollider* pSC = new SphereCollider(MODELSIZE / 2);
	this->AddCollider(pSC);

	//時間切れ
	pTimer_->Update();
	if (pTimer_->isTimeUpped())
	{
		stageState = STATE_FAILURE;
	}

	SetAnimFramerate();
	PlayerOperation();

	switch (playerState_)
	{
	case STATE_IDLE:
		IdleUpdate();
		break;
	case STATE_WALK:
		WalkUpdate();
		break;
	case STATE_JAMP:
		JampUpdate();
		break;
	case STATE_FALL:
		FallUpdate();
		break;
	case STATE_DEAD:
		DeadUpdate();
		break;
	}

	if (playerState_ != STATE_DEAD)
	{
		//コンベアによって移動する方向
		XMVECTOR converyor_velocity = XMVectorSet(-1.0f, 0, 0, 0);
		standMath_ = SetStandMath(transform_.position_);
		switch (standMath_.mathType_)
		{
		case MATH_CONVEYOR:
			XMMATRIX yrot = XMMatrixRotationY(XMConvertToRadians(-standMath_.mathPos_.rotate_.z));
			converyor_velocity = XMVector3Transform(converyor_velocity, yrot);	//その回転でベクトルの向きを変える
			converyor_velocity = converyor_velocity * 0.04f;
			if (playerState_ == STATE_WALK || playerState_ == STATE_IDLE)		velocity_ += converyor_velocity;
			break;
		case MATH_GOAL:
			stageState = STATE_GOAL;
			break;
		case MATH_HOLE:
			playerState_ = STATE_FALL;
			break;
		default:break;
		}

		velocity_ += XMLoadFloat3(&gravity_);
		velocity_ += jamp_start_velocity_;
		transform_.position_ += velocity_;
	}
}

bool Player::Is_InSide_Table(XMFLOAT3 _pos)
{
	return (_pos.x + MODELSIZE) >= 0 && (_pos.x) < XSIZE - (1.0f - MODELSIZE) &&
		(_pos.z + MODELSIZE) >= 0 && (_pos.z) < ZSIZE - (1.0f - MODELSIZE);
}

void Player::PlayerOperation()
{
	const int DEC_VELOCITY_INIT = 20;
	const int DEC_VELOCITY_UPDATE = 5;
	const int DEC_VELOCITY_LIMIT = 100;
	static int dec_velocity_ = DEC_VELOCITY_INIT;

	if (playerState_ != STATE_DEAD)
	{
		//前後左右移動
		if (Input::IsKey(DIK_W))
		{
			sub_velocity_ += XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f);
			sub_velocity_ = XMVector4Normalize(sub_velocity_); //正規化して全部1になる
			dec_velocity_ = DEC_VELOCITY_INIT;
		}
		if (Input::IsKey(DIK_S))
		{
			sub_velocity_ += XMVectorSet(0.0f, 0.0f, -1.0f, 0.0f);
			sub_velocity_ = XMVector4Normalize(sub_velocity_);
			dec_velocity_ = DEC_VELOCITY_INIT;
		}
		if (Input::IsKey(DIK_A))
		{
			sub_velocity_ += XMVectorSet(-1.0f, 0.0f, 0.0f, 0.0f);
			sub_velocity_ = XMVector4Normalize(sub_velocity_);
			dec_velocity_ = DEC_VELOCITY_INIT;
		}
		if (Input::IsKey(DIK_D))
		{
			sub_velocity_ += XMVectorSet(1.0f, 0.0f, 0.0f, 0.0f);
			sub_velocity_ = XMVector4Normalize(sub_velocity_);
			dec_velocity_ = DEC_VELOCITY_INIT;
		}


		velocity_ = sub_velocity_ / dec_velocity_ / air_dec_velocity_;

		dec_velocity_ += DEC_VELOCITY_UPDATE;

		if (dec_velocity_ >= DEC_VELOCITY_LIMIT)
		{
			velocity_ = sub_velocity_ = XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);
			dec_velocity_ = DEC_VELOCITY_INIT;
		}

		//カメラ回転
		//回転
		if (Input::IsKey(DIK_RIGHT))
		{
			camRot_.y++;
		}
		if (Input::IsKey(DIK_LEFT))
		{
			camRot_.y--;
		}
	}

	//カメラ回転
	XMVECTOR cameraBase = XMVectorSet(0, 7, -5, 0);

	XMMATRIX yrot = XMMatrixRotationY(XMConvertToRadians(camRot_.y));

	XMVECTOR cameraRotVec = XMVector3Transform(cameraBase, yrot);

	XMVECTOR vPos = XMVectorSet(transform_.position_.x, 1, transform_.position_.z, 0);

	Camera::SetPosition(vPos + cameraRotVec);
	Camera::SetTarget(XMFLOAT3(transform_.position_.x, 1, transform_.position_.z));


	//移動方向回転
	velocity_ = XMVector3Transform(velocity_, yrot);

	if (XMVectorGetX(XMVector3Length(velocity_)) != 0)
	{
		XMVECTOR v = XMVector3Dot(eyeDirection_, XMVector3Normalize(velocity_));

		float angle = XMConvertToDegrees(acos(XMVectorGetX(v)));

		// XMVector3Cross : vFront と vMove の外積求める
		XMVECTOR vCross = XMVector3Cross(eyeDirection_, XMVector3Normalize(velocity_));

		// vFront と vMove の外積が下向きだったら angle に-1をかけて、向きを反対にする
		if (XMVectorGetY(vCross) < 0)
		{
			angle *= -1;
		}
		transform_.rotate_.y = angle;
	}

	//ジャンプ
	if (Input::IsKeyDown(DIK_SPACE) && 
	(playerState_ == STATE_WALK || playerState_ == STATE_IDLE))
	{
		playerState_ = STATE_JAMP;
		jamp_start_velocity_ = velocity_ / (air_dec_velocity_ + 1);
	}
	
}

MATHDEDAIL Player::SetStandMath(XMFLOAT3 _pos)
{
	if (!Is_InSide_Table(_pos))
	{
		return MATHDEDAIL{ MATH_HOLE, transform_};
	}
	MATHDEDAIL ret;

	float plusX = MODELSIZE / 2, plusZ = MODELSIZE / 2;
	if (XSIZE - transform_.position_.x < plusX)
	{
		plusX = (float)(XSIZE - transform_.position_.x) - 0.00001f;
	}
	if (ZSIZE - transform_.position_.z < plusZ)
	{
		plusZ = (float)(ZSIZE - transform_.position_.z) - 0.00001f;
	}
	centerPos_ =
		XMFLOAT3(transform_.position_.x + plusX, transform_.position_.y, transform_.position_.z + plusZ);

	ret = math_[centerPos_.x][centerPos_.z];
	//HOLLチェック
	ret = HollCheck(_pos);

	ret = math_[centerPos_.x][centerPos_.z];

	//WALLチェック
	WallCheck(_pos);

	return ret;
}

void Player::IdleUpdate()
{
	tableHitPoint_ = XMFLOAT3(0, 0, 0);
	isTableHit_ = false;
	gravity_ = XMFLOAT3(0, 0, 0);
	transform_.position_.y = startPos_.y;
	air_dec_velocity_ = AIR_DEC_VELOCITY_INIT;
	jamp_start_velocity_ = XMVectorSet(0, 0, 0, 0);

	if (XMVectorGetX(XMVector3Length(velocity_)))
	{
		playerState_ = STATE_WALK;
	}
}

void Player::WalkUpdate()
{
	tableHitPoint_ = XMFLOAT3(0, 0, 0);
	isTableHit_ = false;
	gravity_ = XMFLOAT3(0, 0, 0);
	transform_.position_.y = startPos_.y;
	air_dec_velocity_ = AIR_DEC_VELOCITY_INIT;
	jamp_start_velocity_ = XMVectorSet(0, 0, 0, 0);

	if (!XMVectorGetX(XMVector3Length(velocity_)))
	{
		playerState_ = STATE_IDLE;
	}
}

void Player::JampUpdate()
{
	gravity_.y = 0.2f;
	air_dec_velocity_ = AIR_DEC_VELOCITY;
	if (transform_.position_.y >= 1.5f)
	{
		playerState_ = STATE_FALL;
	}
}

void Player::FallUpdate()
{
	gravity_.y += -0.01f;
	air_dec_velocity_ = AIR_DEC_VELOCITY;
	if (transform_.position_.y < 1.0f && !isTableHit_)
	{
		isTableHit_ = true;
		tableHitPoint_ = transform_.position_;
	}
	if (isTableHit_)
	{
		if (SetStandMath(tableHitPoint_).mathType_ != (int)MATH_HOLE)
		{
			playerState_ = STATE_WALK;
			return;
		}
	}
	if (transform_.position_.y < -1.0f)
	{
		playerState_ = STATE_DEAD;
		return;
	}
}

void Player::DeadUpdate()
{
	if (abs(startPos_.x - transform_.position_.x) <= 0.01f &&
		abs(startPos_.z - transform_.position_.z) <= 0.01f)
	{
		stageState = STATE_START;
	}
	transform_.position_.x = transform_.position_.x + (startPos_.x - transform_.position_.x) / 10;
	transform_.position_.z = transform_.position_.z + (startPos_.z - transform_.position_.z) / 10;
	transform_.position_.y = 10;
}

void Player::SetAnimFramerate()
{

	switch (playerState_)
	{
	case STATE_IDLE:
		startFrame_ = 1;
		endFrame_ = 60;
		break;
	case STATE_WALK:
		startFrame_ = 61;
		endFrame_ = 120;
		break;
	case STATE_JAMP:
		startFrame_ = 121;
		endFrame_ = 150;
		break;
	case STATE_FALL:
		startFrame_ = 150;
		endFrame_ = 150;
		break;
	}
	if (prevPlayerState_ != playerState_)
	{
		Model::SetAnimFrame(hModel_, startFrame_, endFrame_, 1);
	}
	prevPlayerState_ = playerState_;
}

MATHDEDAIL Player::HollCheck(XMFLOAT3 _pos)
{
	XMFLOAT3 rightFront = XMFLOAT3(_pos.x + MODELSIZE, _pos.y, _pos.z + MODELSIZE);
	XMFLOAT3 rightBack = XMFLOAT3(_pos.x + MODELSIZE, _pos.y, _pos.z - (1.0f - MODELSIZE));
	XMFLOAT3 leftFront = XMFLOAT3(_pos.x - (1.0f - MODELSIZE), _pos.y, _pos.z + MODELSIZE);
	XMFLOAT3 leftBack = XMFLOAT3(_pos.x - (1.0f - MODELSIZE), _pos.y, _pos.z - (1.0f - MODELSIZE));

	if (Is_InSide_Table(rightFront) &&
		math_[rightFront.x][rightFront.z].mathType_ != MATH_WALL &&
		math_[rightFront.x][rightFront.z].mathType_ != MATH_HOLE)
	{
		return math_[rightFront.x][rightFront.z];
	}
	else if (Is_InSide_Table(rightBack) &&
		math_[rightBack.x][rightBack.z].mathType_ != MATH_WALL &&
		math_[rightBack.x][rightBack.z].mathType_ != MATH_HOLE)
	{
		return math_[rightBack.x][rightBack.z];
	}
	else if (Is_InSide_Table(leftFront) &&
		math_[leftFront.x][leftFront.z].mathType_ != MATH_WALL &&
		math_[leftFront.x][leftFront.z].mathType_ != MATH_HOLE)
	{
		return math_[leftFront.x][leftFront.z];
	}
	else if (Is_InSide_Table(leftBack) &&
		math_[leftBack.x][leftBack.z].mathType_ != MATH_WALL &&
		math_[leftBack.x][leftBack.z].mathType_ != MATH_HOLE)
	{
		return math_[leftBack.x][leftBack.z];
	}
	return math_[_pos.x][_pos.z];
}

void Player::WallCheck(XMFLOAT3 _pos)
{
	bool check = false;
	XMFLOAT3 rightFront = XMFLOAT3(_pos.x + MODELSIZE, _pos.y, _pos.z + MODELSIZE);
	XMFLOAT3 rightBack = XMFLOAT3(_pos.x + MODELSIZE, _pos.y, _pos.z + (1.0f - MODELSIZE));
	XMFLOAT3 leftFront = XMFLOAT3(_pos.x + (1.0f - MODELSIZE), _pos.y, _pos.z + MODELSIZE);
	XMFLOAT3 leftBack = XMFLOAT3(_pos.x + (1.0f - MODELSIZE), _pos.y, _pos.z + (1.0f - MODELSIZE));

	if (Is_InSide_Table(rightFront) &&
		math_[rightFront.x][rightFront.z].mathType_ == MATH_WALL)
	{
		check = true;
		//前
		if (abs((float)((int)rightFront.x - rightFront.x)) > abs((float)((int)rightFront.z - rightFront.z)) ||
			math_[leftFront.x][leftFront.z].mathType_ == MATH_WALL)
		{
			transform_.position_.z = (float)(int)rightFront.z - (rightFront.z - _pos.z);
		}
		//右
		if (abs((float)((int)rightFront.x - rightFront.x)) <= abs((float)((int)rightFront.z - rightFront.z)) ||
			math_[rightBack.x][rightBack.z].mathType_ == MATH_WALL)
		{
			transform_.position_.x = (float)((int)rightFront.x) - (rightFront.x - _pos.x);
		}
	}
	if (Is_InSide_Table(rightBack) &&
		math_[rightBack.x][rightBack.z].mathType_ == MATH_WALL)
	{
		check = true;
		//後ろ
		if (abs((float)((int)rightBack.x - rightBack.x)) > abs((float)((int)(rightBack.z + 1) - rightBack.z)) ||
			math_[leftBack.x][leftBack.z].mathType_ == MATH_WALL)
		{
			transform_.position_.z = (float)(int)(rightBack.z + 1) - (rightBack.z - _pos.z);
		}
		//右
		if (abs((float)((int)rightBack.x - rightBack.x)) <= abs((float)((int)(rightBack.z + 1) - rightBack.z)) ||
			math_[rightFront.x][rightFront.z].mathType_ == MATH_WALL)
		{
			transform_.position_.x = (float)(int)rightBack.x - (rightBack.x - _pos.x);
		}
	}
	if (Is_InSide_Table(leftFront) &&
		math_[leftFront.x][leftFront.z].mathType_ == MATH_WALL)
	{
		check = true;
		//前
		if (abs((float)((int)(leftFront.x + 1) - leftFront.x)) > abs((float)((int)leftFront.z - leftFront.z)) ||
			math_[rightFront.x][rightFront.z].mathType_ == MATH_WALL)
		{
			transform_.position_.z = (float)(int)leftFront.z - (leftFront.z - _pos.z);
		}
		//左
		if (abs((float)((int)(leftFront.x + 1) - leftFront.x)) <= abs((float)((int)leftFront.z - leftFront.z)) ||
			math_[leftBack.x][leftBack.z].mathType_ == MATH_WALL)
		{
			transform_.position_.x = (float)(int)(leftFront.x + 1) - (leftFront.x - _pos.x);
		}
	}
	if (Is_InSide_Table(leftBack) &&
		math_[leftBack.x][leftBack.z].mathType_ == MATH_WALL)
	{
		check = true;
		//後ろ
		if (abs((float)((int)(leftBack.x + 1) - leftBack.x)) > abs((float)((int)(leftBack.z + 1) - leftBack.z)) ||
			math_[rightBack.x][rightBack.z].mathType_ == MATH_WALL)
		{
			transform_.position_.z = (float)(int)(leftBack.z + 1) - (leftBack.z - _pos.z);
		}
		//左
		if (abs((float)((int)(leftBack.x + 1) - leftBack.x)) <= abs((float)((int)(leftBack.z + 1) - leftBack.z)) ||
			math_[leftFront.x][leftFront.z].mathType_ == MATH_WALL)
		{
			transform_.position_.x = (float)(int)(leftBack.x + 1) - (leftBack.x - _pos.x);
		}
	}
}

void Player::OnCollision(GameObject* pTarget)
{
	if (pTarget->GetObjectName() == "Togetoge")
	{
		playerState_ = STATE_DEAD;
	}
}
