#include "PlayScene.h"

#include "Engine/Input.h"
#include "Engine/SceneManager.h"
#include "Engine/Image.h"

#include "SceneTransition.h"
#include "Timer.h"
#include "Stage.h"
#include "Player.h"

PlayScene::PlayScene(GameObject* parent)
	: GameObject(parent, "PlayScene"), table_Change_(false), save_Num_(2)
{
	pTrans_ = (SceneTransition*)FindObject("SceneTransition");
	XSIZE = (int)pTrans_->GetMathSize_x();
	ZSIZE = (int)pTrans_->GetMathSize_z();
	pTrans_->SetSceneState(pTrans_->GetSceneState() + 1);
	player_Num_ = pTrans_->GetPlayerNum();
	Math_Resize(XSIZE, ZSIZE, &math_);

	if (pTrans_->GetTurnNum() % 2 == 0 && (int)pTrans_->GetSceneState() == SCENESTATE::SCENE_STAGE1_DELAY)
	{
		save_Num_ -= 1;
	}
	else if (pTrans_->GetTurnNum() % 2 == 0 && (int)pTrans_->GetSceneState() == SCENESTATE::SCENE_STAGE2_DELAY)
	{
		save_Num_ += 1;
	}
	else if (pTrans_->GetTurnNum() % 2 == 1 && (int)pTrans_->GetSceneState() == SCENESTATE::SCENE_STAGE1_DELAY)
	{
		save_Num_ += 1;
	}
	else
	{
		save_Num_ -= 1;
	}

	Read();
}

void PlayScene::Initialize()
{
	pStage_ = (Stage*)FindObject("Stage");
	pPlayer_ = (Player*)FindObject("Player");
	pPlayer_->Instantiate<Player>(this);
	pStage_->Instantiate<Stage>(this);
}

void PlayScene::Update()
{
	pTrans_ = (SceneTransition*)FindObject("SceneTransition");
	pStage_ = (Stage*)FindObject("Stage");
	pPlayer_ = (Player*)FindObject("Player");
	if (pPlayer_->Is_Goal())
	{
		pTrans_->SetSceneState(pTrans_->GetSceneState() + 1);
		pTrans_->SetIsClear(player_Num_, true);
		KillMe();
	}
	if (pPlayer_->GetFailed())
	{
		pTrans_->SetSceneState(pTrans_->GetSceneState() + 1);
		KillMe();
	}
}

void PlayScene::Draw()
{
}

void PlayScene::Release()
{
}

void PlayScene::Read()
{
	std::ifstream read;
	std::string openfile = "StageSaveFile\\saveMath";

	openfile += std::to_string(save_Num_);
	read.open(openfile, std::ios::in);
	//  �t�@�C�����J��
	//  ios::in �͓ǂݍ��ݐ�p  ios::binary �̓o�C�i���`��

	if (!read) {
		std::cout << "�t�@�C�����J���܂���";
		return;
	}
	//  �t�@�C�����J���Ȃ������Ƃ��̑΍�

	//�t�@�C���̍Ō�܂ő�����
	for (int i = 0; i < XSIZE; i++)
	{
		for (int j = 0; j < ZSIZE; j++)
		{
			read.read((char*)&math_[i][j], sizeof(math_[i][j]));
			//������ł͂Ȃ��f�[�^��ǂ݂���

		}
	}
	read.close();  //�t�@�C�������

	//�Ƃ��Ƃ����[�g
	openfile = "StageSaveFile\\tgtgRoute";
	openfile += std::to_string(save_Num_);
	read.open(openfile, std::ios::in);
	if (!read) {
		std::cout << "�t�@�C�����J���܂���";
		return;
	}
	
	while (!read.eof())
	{
		TOGETOGEROUTE pTg;
		read.read((char*)&pTg, sizeof(pTg));
		if (pTg.route_.scale_.x < 1)
		{
			tTgtgRoute_.push_back(pTg);
		}
	}
	read.close();  //�t�@�C�������
}

XMFLOAT3 PlayScene::GetPlayerPos()
{
	pPlayer_ = (Player*)FindObject("Player");
	return pPlayer_->GetPosition();
}
