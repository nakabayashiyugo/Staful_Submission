#include <string>
#include <vector>
#include <array>

#include "Engine/Input.h"
#include "Engine/Model.h"
#include "Engine/Direct3D.h"
#include "Engine/RootJob.h"
#include "Engine/Camera.h"

#include "SceneTransition.h"
#include "PlayScene.h"
#include "Stage.h"
#include "Togetoge.h"

#include "resource.h"

const int MAKEHOLETIME = 2;
const int FPS = 60;

Stage::Stage(GameObject* parent)
	: GameObject(parent, "Stage"), isStandPitfall_(false),
	tgtgGivePos_(0, 0, 0)
{
	for (int i = 0; i < 5; i++)
	{
		hModel_[i] = -1;
	}
	pTrans_ = (SceneTransition*)FindObject("SceneTransition");
	XSIZE = (int)pTrans_->GetMathSize_x();
	ZSIZE = (int)pTrans_->GetMathSize_z();
	Math_Resize(XSIZE, ZSIZE, &math_);
	Math_Resize(XSIZE, ZSIZE, &makeHoleTimes_);
	pPlayScene_ = (PlayScene*)FindObject("PlayScene");
	SetTableMath(pPlayScene_->GetTableMath());
	SetTogetogeRoute(pPlayScene_->GetTogetogeRoute());
	
	for (int x = 0; x < makeHoleTimes_.size(); x++)
	{
		for (int z = 0; z < makeHoleTimes_[x].size(); z++)
		{
			makeHoleTimes_[x][z] = FPS * MAKEHOLETIME;
		}
	}
}

void Stage::Initialize()
{
	std::string modelName[MATH_MAX] =
	{
		"Block_Floor.fbx",
		"Block_Wall.fbx",
		"Block_Holl.fbx",
		"Block_Converyor.fbx",
		"Block_Floor.fbx",
		"Block_Crack.fbx",
		"Block_Start.fbx",
		"Block_Goal.fbx"
	};
	std::string fname_base = "Assets\\";
	//モデルデータのロード
	for (int i = 0; i < MATH_MAX; i++)
	{
		hModel_[i] = Model::Load(fname_base + modelName[i]);
		assert(hModel_[i] >= 0);
	}

	//とげとげ出す
	pTgtg_.resize(tTgtgRoute_.size());
	for (int i = 0; i < pTgtg_.size(); i++)
	{
		tgtgGivePos_ = tTgtgRoute_[i].initPos_;
		pTgtg_[i]->Instantiate<Togetoge>(this);
	}
}


void Stage::Update()
{
	XMFLOAT3 playerPos = pPlayScene_->GetPlayerPos();
	if (playerPos.x >= 0 && playerPos.x < XSIZE &&
		playerPos.z >= 0 && playerPos.z < ZSIZE)
	{
		if (math_[(int)playerPos.x][(int)playerPos.z].mathType_ == MATH_PITFALL)
		{
			makeHoleTimes_[(int)playerPos.x][(int)playerPos.z]--;
			if (makeHoleTimes_[(int)playerPos.x][(int)playerPos.z] <= 0)
			{
				math_[(int)playerPos.x][(int)playerPos.z].mathType_ = MATH_HOLE;
				Write();
				pPlayScene_->Read();
				pPlayScene_->SetTableChange(true);
			}
		}
	}
}

void Stage::Draw()
{
	for (int x = 0; x < XSIZE; x++)
	{
		for (int z = 0; z < ZSIZE; z++)
		{
			Transform mathTrans;
			mathTrans.position_ = XMFLOAT3(x, 0, z);

			switch (math_[x][z].mathType_)
			{
			case MATH_FLOOR:
				Model::SetTransform(hModel_[math_[x][z].mathType_], mathTrans);
				Model::Draw(hModel_[math_[x][z].mathType_]);
				break;
			case MATH_WALL :
				Model::SetTransform(hModel_[MATH_FLOOR], mathTrans);
				Model::Draw(hModel_[MATH_FLOOR]);
				mathTrans.position_.y = 1;
				Model::SetTransform(hModel_[math_[x][z].mathType_], mathTrans);
				Model::Draw(hModel_[math_[x][z].mathType_]);
				break;
			case MATH_CONVEYOR:
				mathTrans.rotate_.y = (math_[x][z].mathPos_.rotate_.z);
				Model::SetTransform(hModel_[math_[x][z].mathType_], mathTrans);
				Model::Draw(hModel_[math_[x][z].mathType_]);
				break;
			case MATH_TOGETOGE:
				Model::SetTransform(hModel_[MATH_FLOOR], mathTrans);
				Model::Draw(hModel_[MATH_FLOOR]);
				mathTrans.position_.y = 1;
				break;
			case MATH_PITFALL:
				if (makeHoleTimes_[x][z] >= FPS * MAKEHOLETIME / 2)
				{
					Model::SetTransform(hModel_[MATH_FLOOR], mathTrans);
					Model::Draw(hModel_[MATH_FLOOR]);
				}
				else
				{
					Model::SetTransform(hModel_[math_[x][z].mathType_], mathTrans);
					Model::Draw(hModel_[math_[x][z].mathType_]);
				}

				break;
			case MATH_START:
				Model::SetTransform(hModel_[MATH_FLOOR], mathTrans);
				Model::Draw(hModel_[MATH_FLOOR]);
				mathTrans.position_.y = 1;
				Model::SetTransform(hModel_[math_[x][z].mathType_], mathTrans);
				Model::Draw(hModel_[math_[x][z].mathType_]);
				break;
			case MATH_GOAL:
				Model::SetTransform(hModel_[MATH_FLOOR], mathTrans);
				Model::Draw(hModel_[MATH_FLOOR]);
				mathTrans.position_.y = 1;
				Model::SetTransform(hModel_[math_[x][z].mathType_], mathTrans);
				Model::Draw(hModel_[math_[x][z].mathType_]);
				break;
			}
		}
	}
	
}

void Stage::Release()
{
}

void Stage::SetBlock(int x, int z, MATHTYPE _type)
{
	math_.at(x).at(z).mathType_ = _type;
	assert(_type <= MATHTYPE::MATH_MAX);
}

void Stage::Write()
{
	std::ofstream write;
	std::string savefile = "StageSaveFile\\saveMath";
	savefile += std::to_string(pPlayScene_->GetSaveNum());
	write.open(savefile, std::ios::out);

	//  ファイルが開けなかったときのエラー表示
	if (!write) {
		std::cout << "ファイル " << savefile << " が開けません";
		return;
	}

	for (int i = 0; i < XSIZE; i++) {
		for (int j = 0; j < ZSIZE; j++)
		{
			write.write((char*)&math_[i][j], sizeof(math_[i][j]));
			//文字列ではないデータをかきこむ
		}
	}

	write.close();  //ファイルを閉じる

	//とげとげルート
	savefile = "StageSaveFile\\tgtgRoute";
	savefile += std::to_string(pPlayScene_->GetSaveNum());
	write.open(savefile, std::ios::out);
	//  ファイルが開けなかったときのエラー表示
	if (!write) {
		std::cout << "ファイル " << savefile << " が開けません";
		return;
	}
	for (auto itr = tTgtgRoute_.begin(); itr != tTgtgRoute_.end(); itr++)
	{
		write.write((char*)&itr, sizeof(itr));
	}
	write.close();  //ファイルを閉じる
}

void Stage::SetTableMath(std::vector<std::vector<MATHDEDAIL>> _math)
{
	for (int x = 0; x < XSIZE; x++)
	{
		for (int z = 0; z < ZSIZE; z++)
		{
			math_.at(x).at(z) = _math.at(x).at(z);
		}
	}
}

XMFLOAT3 Stage::GetTogetogeInitPos()
{
	return tgtgGivePos_;
}


