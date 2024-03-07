#include "Model.h"
#include "Direct3D.h"

namespace Model
{
	struct ModelData
	{
		Fbx* pFbx_;
		Transform transform_;
		std::string fileName_;
		//アニメーションのフレーム
		float nowFrame, animSpeed;
		int startFrame, endFrame;
		//初期化
		ModelData() : pFbx_(nullptr), nowFrame(0), startFrame(0), endFrame(0), animSpeed(0),
			fileName_("")
		{
		}
		//アニメーションのフレーム数をセット
		//引数：startFrame	開始フレーム
		//引数：endFrame	終了フレーム
		//引数：animSpeed	アニメーション速度
		void SetAnimFrame(int start, int end, float speed)
		{
			nowFrame = (float)start;
			startFrame = start;
			endFrame = end;
			animSpeed = speed;
		}
	};

	//モデルのポインタを入れておくポインタ
	std::vector<ModelData*> modelList;
}

void Model::SetTransform(int hModel, Transform transform)
{
	modelList[hModel]->transform_ = transform;
}

int Model::Load(std::string filename)
{
	ModelData* pData = new ModelData();
	pData->fileName_ = filename;
	pData->pFbx_ = nullptr;
	for (auto itr = modelList.begin(); itr != modelList.end(); itr++)
	{
		if ((*itr)->fileName_ == filename)
		{
			pData->pFbx_ = (*itr)->pFbx_;
			break;
		}
	}
	if (pData->pFbx_ == nullptr)
	{
		pData->pFbx_ = new Fbx();
		if (FAILED(pData->pFbx_->Load(filename)))
		{
			//エラー処理
			MessageBox(nullptr, "モデルのロードに失敗しました。", "エラー", MB_OK);
		}
	}
	modelList.push_back(pData);
	return (modelList.size() - 1);
}

void Model::Draw(int hModel)
{
	if (hModel < 0 || hModel >= modelList.size() || modelList[hModel] == nullptr)
	{
		return;
	}

	//アニメーションを進める
	modelList[hModel]->nowFrame += modelList[hModel]->animSpeed;

	//最後までアニメーションしたら戻す
	if (modelList[hModel]->nowFrame > (float)modelList[hModel]->endFrame)
		modelList[hModel]->nowFrame = (float)modelList[hModel]->startFrame;



	if (modelList[hModel]->pFbx_)
	{
		modelList[hModel]->pFbx_->Draw(modelList[hModel]->transform_, (int)modelList[hModel]->nowFrame);
	}
}

void Model::Release()
{
	bool isReffered = false;
	for (int i = 0; i < modelList.size(); i++)
	{
		for (int j = i + 1; j < modelList.size(); j++)
		{
			if (modelList[i]->pFbx_ == modelList[j]->pFbx_)
			{
				isReffered = true;
				break;
			}
		}
		if (isReffered == false)
		{
			SAFE_DELETE(modelList[i]->pFbx_);
		}
		SAFE_DELETE(modelList[i]);
	}
	modelList.clear();

}

void Model::SetAnimFrame(int handle, int startFrame, int endFrame, float animSpeed)
{
	modelList[handle]->SetAnimFrame(startFrame, endFrame, animSpeed);
}

int Model::GetAnimFrame(int handle)
{
	return (int)modelList[handle]->nowFrame;
}

XMFLOAT3 Model::GetBonePosition(int handle, std::string boneName)
{
	XMFLOAT3 pos = modelList[handle]->pFbx_->GetBonePosition(boneName);
	XMVECTOR vec = XMVector3TransformCoord(XMLoadFloat3(&pos), modelList[handle]->transform_.GetWorldMatrix());
	XMStoreFloat3(&pos, vec);
	return pos;
}

void Model::RayCast(int hModel, RayCastData& rayData)
{
	modelList[hModel]->transform_.Calclation();
	//①ワールド行列の逆行列
	XMMATRIX wInv = XMMatrixInverse(nullptr, modelList[hModel]->transform_.GetWorldMatrix());
	//②レイの通過点を求める
	XMVECTOR vpass{ rayData.start.x + rayData.dir.x,
					rayData.start.y + rayData.dir.y,
					rayData.start.z + rayData.dir.z,
					rayData.start.w + rayData.dir.w };
	//③rayData.startをモデル空間に変換
	XMVECTOR vstart = XMLoadFloat4(&rayData.start);
	vstart = XMVector3TransformCoord(vstart, wInv);
	XMStoreFloat4(&rayData.start, vstart);
	//④(始点から方向ベクトルを伸ばした先)通過点(②)に①をかける
	vpass = XMVector3TransformCoord(vpass, wInv);
	//⑤rayData.dirを③から④に向かうベクトル
	vpass = vpass - vstart;
	XMStoreFloat4(&rayData.dir, vpass);


	//指定されたモデル番号のFBXにレイキャスト
	modelList[hModel]->pFbx_->RayCast(rayData);
}
