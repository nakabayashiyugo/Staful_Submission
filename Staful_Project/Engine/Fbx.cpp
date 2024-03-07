#include "Fbx.h"
#include "Texture.h"
#include "Camera.h"
#include "FbxParts.h"

Fbx::Fbx(): animSpeed_(0)
{
}

HRESULT Fbx::Load(std::string fileName)
{
	str = fileName;
	//マネージャを生成
	pFbxManager_ = FbxManager::Create();

	//インポーターを生成
	FbxImporter* fbxImporter = FbxImporter::Create(pFbxManager_, "imp");
	fbxImporter->Initialize(fileName.c_str(), -1, pFbxManager_->GetIOSettings());

	//シーンオブジェクトにFBXファイルの情報を流し込む
	pFbxScene_ = FbxScene::Create(pFbxManager_, "fbxscene");
	fbxImporter->Import(pFbxScene_);
	fbxImporter->Destroy();

	// アニメーションのタイムモードの取得
	frameRate_ = pFbxScene_->GetGlobalSettings().GetTimeMode();

	//現在のカレントディレクトリを覚えておく
	char defaultCurrentDir[MAX_PATH];
	GetCurrentDirectory(MAX_PATH, defaultCurrentDir);

	//引数のfileNameからディレクトリ部分を取得
	char dir[MAX_PATH];
	_splitpath_s(fileName.c_str(), nullptr, 0, dir, MAX_PATH, nullptr, 0, nullptr, 0);

	//カレントディレクトリ変更
	SetCurrentDirectory(dir);

	FbxNode* rootNode = pFbxScene_->GetRootNode();
	//そいつの子供の数を調べて
	int childCount = rootNode->GetChildCount();
	//1個ずつチェック
	for (int i = 0; childCount > i; i++)
	{
		CheckNode(rootNode->GetChild(i), &parts_);
	}

	//カレントディレクトリを元に戻す
	SetCurrentDirectory(defaultCurrentDir);

	//マネージャ解放
	//pFbxManager_->Destroy();
	return S_OK;
}

void Fbx::CheckNode(FbxNode* pNode, std::vector<FbxParts*>* pPartsList)
{
	//そのノードにはメッシュ情報が入っているだろうか？
	FbxNodeAttribute* attr = pNode->GetNodeAttribute();
	if (attr != nullptr && attr->GetAttributeType() == FbxNodeAttribute::eMesh)
	{
		//パーツを用意
		FbxParts* pParts = new FbxParts;
		if (FAILED(pParts->Init(pNode)))
		{
			//エラー処理
			MessageBox(nullptr, "スケルトン情報の初期化に失敗しました。", "エラー", MB_OK);
		}

		//パーツ情報を動的配列に追加
		pPartsList->push_back(pParts);
	}


	//子ノードにもデータがあるかも！！
	{
		//子供の数を調べて
		int childCount = pNode->GetChildCount();

		//一人ずつチェック
		for (int i = 0; i < childCount; i++)
		{
			CheckNode(pNode->GetChild(i), pPartsList);
		}
	}
}

void Fbx::Draw(Transform& transform, int frame)
{
	Direct3D::SetShader(SHADER_3D);

	//パーツを1個ずつ描画
	for (int k = 0; k < parts_.size(); k++)
	{
		// その瞬間の自分の姿勢行列を得る
		FbxTime     time;
		time.SetTime(0, 0, 0, frame, 0, 0, frameRate_);

		//スキンアニメーション（ボーン有り）の場合
		if (parts_[k]->GetSkinInfo() != nullptr)
		{
			parts_[k]->DrawSkinAnime(transform, time);
		}

		//メッシュアニメーションの場合
		else
		{
			parts_[k]->DrawMeshAnime(transform, time, pFbxScene_);
		}
	}
}



XMFLOAT3 Fbx::GetBonePosition(std::string boneName)
{
	XMFLOAT3 position = XMFLOAT3(0, 0, 0);
	for (int i = 0; i < parts_.size(); i++)
	{
		if (parts_[i]->GetBonePosition(boneName, &position))
			break;
	}


	return position;
}

void Fbx::Release()
{
	//SAFE_DELETE(pMaterialList_);
	//SAFE_RELEASE(pConstantBuffer_);
	//SAFE_RELEASE(pVertexBuffer_);
	////SAFE_RELEASE(pIndexBuffer_);

	for (int i = 0; i < parts_.size(); i++)
	{
		delete parts_[i];
	}
	parts_.clear();
}

void Fbx::RayCast(RayCastData& rayData)
{
	//すべてのパーツと判定
	for (int i = 0; i < parts_.size(); i++)
	{
		parts_[i]->RayCast(rayData);
	}
}
