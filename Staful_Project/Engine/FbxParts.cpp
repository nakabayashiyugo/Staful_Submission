#include "FbxParts.h"
#include "Fbx.h"
#include "Direct3D.h"
#include "Camera.h"
#include "Texture.h"
#include "DirectXCollision.h"

FbxParts::FbxParts() : 
    pVertexBuffer_(nullptr), pIndexBuffer_(nullptr), pConstantBuffer_(nullptr), pMaterialList_(nullptr)
{
}

HRESULT FbxParts::Init(FbxNode* pNode)
{
    FbxMesh* mesh = pNode->GetMesh();

    //各情報の個数を取得
    vertexCount_ = mesh->GetControlPointsCount();	//頂点の数
    polygonCount_ = mesh->GetPolygonCount();	//ポリゴンの数
    materialCount_ = pNode->GetMaterialCount();

	HRESULT hr;

	hr = InitVertex(mesh);		//頂点バッファ準備
	if (FAILED(hr))
	{
		//エラー処理
		MessageBox(nullptr, "頂点データ用バッファの作成に失敗しました", "エラー", MB_OK);
		return hr;
	}
	hr = InitIndex(mesh);		//インデックスバッファ準備
	if (FAILED(hr))
	{
		//エラー処理
		MessageBox(nullptr, "インデックスバッファの作成に失敗しました", "エラー", MB_OK);
		return hr;
	}
	hr = IntConstantBuffer();	//コンスタントバッファ準備
	if (FAILED(hr))
	{
		//エラー処理
		MessageBox(nullptr, "コンスタントバッファの作成に失敗しました", "エラー", MB_OK);
		return hr;
	}
	hr = InitMaterial(pNode);
	if (FAILED(hr))
	{
		//エラー処理
		MessageBox(nullptr, "コンスタントバッファの作成に失敗しました", "エラー", MB_OK);
		return hr;
	}
	hr = InitSkelton(mesh);		//骨の情報を準備
	if (FAILED(hr))
	{
		//エラー処理
		MessageBox(nullptr, "スケルトン情報の初期化に失敗しました。", "エラー", MB_OK);
	}

	return hr;
}

HRESULT FbxParts::InitVertex(fbxsdk::FbxMesh* mesh)
{
	//頂点情報を入れる配列
	pVertices_ = new VERTEX[vertexCount_];

	//全ポリゴン
	for (DWORD poly = 0; poly < polygonCount_; poly++)
	{
		//3頂点分
		for (int vertex = 0; vertex < 3; vertex++)
		{
			int index = mesh->GetPolygonVertex(poly, vertex);

			/////////////////////////頂点の位置/////////////////////////////////////
			FbxVector4 pos = mesh->GetControlPointAt(index);
			pVertices_[index].position = XMVectorSet((float)-pos[0], (float)pos[1], (float)pos[2], 0);

			/////////////////////////頂点の法線/////////////////////////////////////
			FbxVector4 Normal;
			mesh->GetPolygonVertexNormal(poly, vertex, Normal);	//ｉ番目のポリゴンの、ｊ番目の頂点の法線をゲット
			pVertices_[index].normal = XMVectorSet((float)-Normal[0], (float)Normal[1], (float)Normal[2], 0);

			///////////////////////////頂点のＵＶ/////////////////////////////////////
			FbxLayerElementUV* pUV = mesh->GetLayer(0)->GetUVs();
			int uvIndex = mesh->GetTextureUVIndex(poly, vertex, FbxLayerElement::eTextureDiffuse);
			FbxVector2  uv = pUV->GetDirectArray().GetAt(uvIndex);
			pVertices_[index].uv = XMVectorSet((float)uv.mData[0], (float)(1.0f - uv.mData[1]), 0.0f, 0);
		}
	}

	int m_dwNumUV = mesh->GetTextureUVCount();
	FbxLayerElementUV* pUV = mesh->GetLayer(0)->GetUVs();
	if (m_dwNumUV > 0 && pUV->GetMappingMode() == FbxLayerElement::eByControlPoint)
	{
		for (int k = 0; k < m_dwNumUV; k++)
		{
			FbxVector2 uv = pUV->GetDirectArray().GetAt(k);
			pVertices_[k].uv = XMVectorSet((float)uv.mData[0], (float)(1.0f - uv.mData[1]), 0.0f, 0.0f);
		}
	}

	HRESULT hr;

	// 頂点データ用バッファの設定
	D3D11_BUFFER_DESC bd_vertex;
	bd_vertex.ByteWidth = sizeof(VERTEX) * vertexCount_;
	bd_vertex.Usage = D3D11_USAGE_DYNAMIC;
	bd_vertex.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd_vertex.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	bd_vertex.MiscFlags = 0;
	bd_vertex.StructureByteStride = 0;
	D3D11_SUBRESOURCE_DATA data_vertex;
	data_vertex.pSysMem = pVertices_;
	hr = Direct3D::pDevice_->CreateBuffer(&bd_vertex, &data_vertex, &pVertexBuffer_);
	if (FAILED(hr))
	{
		//エラー処理
		MessageBox(nullptr, "頂点データ用バッファの作成に失敗しました", "エラー", MB_OK);
		return hr;
	}

	return S_OK;
}

HRESULT FbxParts::InitIndex(fbxsdk::FbxMesh* mesh)
{
	pIndexBuffer_ = new ID3D11Buffer * [materialCount_];
	indexCount_ = std::vector<int>(materialCount_);
	ppIndex_ = new int* [materialCount_];


	//std::vector<int> index(polygonCount_ * 3);//ポリゴン数*3＝全頂点分用意
	for (int i = 0; i < materialCount_; i++)
	{
		ppIndex_[i] = new int[polygonCount_ * 3];

		int count = 0;

		//全ポリゴン
		for (DWORD poly = 0; poly < polygonCount_; poly++)
		{
			FbxLayerElementMaterial* mtl = mesh->GetLayer(0)->GetMaterials();
			int mtlId = mtl->GetIndexArray().GetAt(poly);

			if (mtlId == i)
			{
				//3頂点分
				for (DWORD vertex = 0; vertex < 3; vertex++)
				{
					ppIndex_[i][count] = mesh->GetPolygonVertex(poly, vertex);
					count++;
				}
			}
		}
		indexCount_[i] = count;

		HRESULT hr;

		// インデックスバッファを生成する
		D3D11_BUFFER_DESC   bd;
		bd.Usage = D3D11_USAGE_DEFAULT;
		bd.ByteWidth = sizeof(int) * polygonCount_ * 3;
		bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
		bd.CPUAccessFlags = 0;
		bd.MiscFlags = 0;

		D3D11_SUBRESOURCE_DATA InitData;
		InitData.pSysMem = ppIndex_[i];
		InitData.SysMemPitch = 0;
		InitData.SysMemSlicePitch = 0;
		hr = Direct3D::pDevice_->CreateBuffer(&bd, &InitData, &pIndexBuffer_[i]);
		if (FAILED(hr))
		{
			//エラー処理
			MessageBox(nullptr, "インデックスバッファの作成に失敗しました", "エラー", MB_OK);
			return hr;
		}
	}

	return S_OK;
}

HRESULT FbxParts::IntConstantBuffer()
{
	HRESULT hr;

	//コンスタントバッファ作成
	D3D11_BUFFER_DESC cb;
	cb.ByteWidth = sizeof(CONSTANT_BUFFER);
	cb.Usage = D3D11_USAGE_DYNAMIC;
	cb.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cb.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	cb.MiscFlags = 0;
	cb.StructureByteStride = 0;

	// コンスタントバッファの作成
	hr = Direct3D::pDevice_->CreateBuffer(&cb, nullptr, &pConstantBuffer_);
	if (FAILED(hr))
	{
		//エラー処理
		MessageBox(nullptr, "コンスタントバッファの作成に失敗しました", "エラー", MB_OK);
		return hr;
	}

	return S_OK;
}

HRESULT FbxParts::InitMaterial(fbxsdk::FbxNode* pNode)
{
	pMaterialList_ = new MATERIAL[materialCount_];

	for (int i = 0; i < materialCount_; i++)
	{
		//i番目のマテリアル情報を取得
		FbxSurfaceMaterial* pMaterial = pNode->GetMaterial(i);

		//テクスチャ情報
		FbxProperty  lProperty = pMaterial->FindProperty(FbxSurfaceMaterial::sDiffuse);

		//テクスチャの数数
		int fileTextureCount = lProperty.GetSrcObjectCount<FbxFileTexture>();

		//テクスチャあり
		if (fileTextureCount > 0)
		{
			FbxFileTexture* textureInfo = lProperty.GetSrcObject<FbxFileTexture>(0);
			const char* textureFilePath = textureInfo->GetRelativeFileName();

			//ファイル名+拡張だけにする
			char name[_MAX_FNAME];	//ファイル名
			char ext[_MAX_EXT];	//拡張子
			_splitpath_s(textureFilePath, nullptr, 0, nullptr, 0, name, _MAX_FNAME, ext, _MAX_EXT);
			wsprintf(name, "%s%s", name, ext);

			//ファイルからテクスチャ作成
			pMaterialList_[i].pTexture = new Texture();
			HRESULT hr = pMaterialList_[i].pTexture->Load(name);
			assert(hr == S_OK);
		}
		//テクスチャ無し
		else
		{
			pMaterialList_[i].pTexture = nullptr;

			//マテリアルの色
			FbxSurfaceLambert* pMaterial = (FbxSurfaceLambert*)pNode->GetMaterial(i);
			FbxDouble3  diffuse = pMaterial->Diffuse;
			pMaterialList_[i].diffuse = XMFLOAT4((float)diffuse[0], (float)diffuse[1], (float)diffuse[2], 1.0f);
		}
	}
	return S_OK;
}

HRESULT FbxParts::InitSkelton(FbxMesh* pMesh)
{
	// デフォーマ情報（ボーンとモデルの関連付け）の取得
	FbxDeformer* pDeformer = pMesh->GetDeformer(0);
	if (pDeformer == nullptr)
	{
		//ボーン情報なし
		return S_OK;
	}

	// デフォーマ情報からスキンメッシュ情報を取得
	pSkinInfo_ = (FbxSkin*)pDeformer;

	// 頂点からポリゴンを逆引きするための情報を作成する
	struct  POLY_INDEX
	{
		int* polyIndex;      // ポリゴンの番号
		int* vertexIndex;    // 頂点の番号
		int     numRef;         // 頂点を共有するポリゴンの数
	};

	POLY_INDEX* polyTable = new POLY_INDEX[vertexCount_];
	for (DWORD i = 0; i < vertexCount_; i++)
	{
		// 三角形ポリゴンに合わせて、頂点とポリゴンの関連情報を構築する
		// 総頂点数＝ポリゴン数×３頂点
		polyTable[i].polyIndex = new int[polygonCount_ * 3];
		polyTable[i].vertexIndex = new int[polygonCount_ * 3];
		polyTable[i].numRef = 0;
		ZeroMemory(polyTable[i].polyIndex, sizeof(int) * polygonCount_ * 3);
		ZeroMemory(polyTable[i].vertexIndex, sizeof(int) * polygonCount_ * 3);

		// ポリゴン間で共有する頂点を列挙する
		for (DWORD k = 0; k < polygonCount_; k++)
		{
			for (int m = 0; m < 3; m++)
			{
				if (pMesh->GetPolygonVertex(k, m) == i)
				{
					polyTable[i].polyIndex[polyTable[i].numRef] = k;
					polyTable[i].vertexIndex[polyTable[i].numRef] = m;
					polyTable[i].numRef++;
				}
			}
		}
	}

	// ボーン情報を取得する
	numBone_ = pSkinInfo_->GetClusterCount();
	ppCluster_ = new FbxCluster * [numBone_];
	for (int i = 0; i < numBone_; i++)
	{
		ppCluster_[i] = pSkinInfo_->GetCluster(i);
	}

	// ボーンの数に合わせてウェイト情報を準備する
	pWeightArray_ = new Weight[vertexCount_];
	for (DWORD i = 0; i < vertexCount_; i++)
	{
		XMStoreFloat3(&pWeightArray_[i].posOrigin, pVertices_[i].position);
		XMStoreFloat3(&pWeightArray_[i].normalOrigin, pVertices_[i].normal);
		pWeightArray_[i].pBoneIndex = new int[numBone_];
		pWeightArray_[i].pBoneWeight = new float[numBone_];
		for (int j = 0; j < numBone_; j++)
		{
			pWeightArray_[i].pBoneIndex[j] = -1;
			pWeightArray_[i].pBoneWeight[j] = 0.0f;
		}
	}

	// それぞれのボーンに影響を受ける頂点を調べる
	// そこから逆に、頂点ベースでボーンインデックス・重みを整頓する
	for (int i = 0; i < numBone_; i++)
	{
		int numIndex = ppCluster_[i]->GetControlPointIndicesCount();   //このボーンに影響を受ける頂点数
		int* piIndex = ppCluster_[i]->GetControlPointIndices();       //ボーン/ウェイト情報の番号
		double* pdWeight = ppCluster_[i]->GetControlPointWeights();     //頂点ごとのウェイト情報

		//頂点側からインデックスをたどって、頂点サイドで整理する
		for (int k = 0; k < numIndex; k++)
		{
			// 頂点に関連付けられたウェイト情報がボーン５本以上の場合は、重みの大きい順に４本に絞る
			for (int m = 0; m < 4; m++)
			{
				if (m >= numBone_)
					break;

				if (pdWeight[k] > pWeightArray_[piIndex[k]].pBoneWeight[m])
				{
					for (int n = numBone_ - 1; n > m; n--)
					{
						pWeightArray_[piIndex[k]].pBoneIndex[n] = pWeightArray_[piIndex[k]].pBoneIndex[n - 1];
						pWeightArray_[piIndex[k]].pBoneWeight[n] = pWeightArray_[piIndex[k]].pBoneWeight[n - 1];
					}
					pWeightArray_[piIndex[k]].pBoneIndex[m] = i;
					pWeightArray_[piIndex[k]].pBoneWeight[m] = (float)pdWeight[k];
					break;
				}
			}

		}
	}

	//ボーンを生成
	pBoneArray_ = new Bone[numBone_];
	for (int i = 0; i < numBone_; i++)
	{
		// ボーンのデフォルト位置を取得する
		FbxAMatrix  matrix;
		ppCluster_[i]->GetTransformLinkMatrix(matrix);

		// 行列コピー（Fbx形式からDirectXへの変換）
		XMFLOAT4X4 pose;
		for (DWORD x = 0; x < 4; x++)
		{
			for (DWORD y = 0; y < 4; y++)
			{
				pose(x, y) = (float)matrix.Get(x, y);
			}
		}
		pBoneArray_[i].bindPose = XMLoadFloat4x4(&pose);
	}

	// 一時的なメモリ領域を解放する
	for (DWORD i = 0; i < vertexCount_; i++)
	{
		SAFE_DELETE(polyTable[i].polyIndex);
		SAFE_DELETE(polyTable[i].vertexIndex);
	}
	SAFE_DELETE(polyTable);

	return S_OK;
}

void FbxParts::Draw(Transform& transform)
{
	Direct3D::SetShader(SHADER_3D);
	transform.Calclation();//トランスフォームを計算

	for (int i = 0; i < materialCount_; i++)
	{
		//コンスタントバッファに渡す情報
		CONSTANT_BUFFER cb;
		cb.matWVP = XMMatrixTranspose(transform.GetWorldMatrix() * Camera::GetViewMatrix() * Camera::GetProjectionMatrix());
		cb.matNormal = XMMatrixTranspose(transform.GetWorldMatrix());
		cb.diffuseColor = pMaterialList_[i].diffuse;
		cb.isTexture = pMaterialList_[i].pTexture != nullptr;


		D3D11_MAPPED_SUBRESOURCE pdata;
		Direct3D::pContext_->Map(pConstantBuffer_, 0, D3D11_MAP_WRITE_DISCARD, 0, &pdata);	// GPUからのデータアクセスを止める
		memcpy_s(pdata.pData, pdata.RowPitch, (void*)(&cb), sizeof(cb));	// データを値を送る

		Direct3D::pContext_->Unmap(pConstantBuffer_, 0);	//再開

		//頂点バッファ
		UINT stride = sizeof(VERTEX);
		UINT offset = 0;
		Direct3D::pContext_->IASetVertexBuffers(0, 1, &pVertexBuffer_, &stride, &offset);

		// インデックスバッファーをセット
		stride = sizeof(int);
		offset = 0;
		Direct3D::pContext_->IASetIndexBuffer(pIndexBuffer_[i], DXGI_FORMAT_R32_UINT, 0);

		//コンスタントバッファ
		Direct3D::pContext_->VSSetConstantBuffers(0, 1, &pConstantBuffer_);	//頂点シェーダー用	
		Direct3D::pContext_->PSSetConstantBuffers(0, 1, &pConstantBuffer_);	//ピクセルシェーダー用

		if (pMaterialList_[i].pTexture)
		{
			ID3D11SamplerState* pSampler = pMaterialList_[i].pTexture->GetSampler();
			Direct3D::pContext_->PSSetSamplers(0, 1, &pSampler);
			ID3D11ShaderResourceView* pSRV = pMaterialList_[i].pTexture->GetSRV();
			Direct3D::pContext_->PSSetShaderResources(0, 1, &pSRV);
		}


		//描画
		Direct3D::pContext_->DrawIndexed(indexCount_[i], 0, 0);
	}
}

void FbxParts::DrawSkinAnime(Transform& transform, FbxTime time)
{
	// ボーンごとの現在の行列を取得する
	for (int i = 0; i < numBone_; i++)
	{
		FbxAnimEvaluator* evaluator = ppCluster_[i]->GetLink()->GetScene()->GetAnimationEvaluator();
		FbxMatrix mCurrentOrentation = evaluator->GetNodeGlobalTransform(ppCluster_[i]->GetLink(), time);

		// 行列コピー（Fbx形式からDirectXへの変換）
		XMFLOAT4X4 pose;
		for (DWORD x = 0; x < 4; x++)
		{
			for (DWORD y = 0; y < 4; y++)
			{
				pose(x, y) = (float)mCurrentOrentation.Get(x, y);
			}
		}

		// オフセット時のポーズの差分を計算する
		pBoneArray_[i].newPose = XMLoadFloat4x4(&pose);
		pBoneArray_[i].diffPose = XMMatrixInverse(nullptr, pBoneArray_[i].bindPose);
		pBoneArray_[i].diffPose *= pBoneArray_[i].newPose;
	}

	// 各ボーンに対応した頂点の変形制御
	for (DWORD i = 0; i < vertexCount_; i++)
	{
		// 各頂点ごとに、「影響するボーン×ウェイト値」を反映させた関節行列を作成する
		XMMATRIX  matrix;
		ZeroMemory(&matrix, sizeof(matrix));
		for (int m = 0; m < numBone_; m++)
		{
			if (pWeightArray_[i].pBoneIndex[m] < 0)
			{
				break;
			}
			matrix += pBoneArray_[pWeightArray_[i].pBoneIndex[m]].diffPose * pWeightArray_[i].pBoneWeight[m];

		}

		// 作成された関節行列を使って、頂点を変形する
		XMVECTOR Pos = XMLoadFloat3(&pWeightArray_[i].posOrigin);
		XMVECTOR Normal = XMLoadFloat3(&pWeightArray_[i].normalOrigin);
		pVertices_[i].position = XMVector3TransformCoord(Pos, matrix);
		pVertices_[i].normal = XMVector3TransformCoord(Normal, matrix);

	}

	// 頂点バッファをロックして、変形させた後の頂点情報で上書きする
	D3D11_MAPPED_SUBRESOURCE msr = {};
	ID3D11Resource* pr = pVertexBuffer_;
	HRESULT hr = Direct3D::pContext_->Map(pr, 0, D3D11_MAP_WRITE_DISCARD, 0, &msr);

	if (msr.pData)
	{
		memcpy_s(msr.pData, msr.RowPitch, pVertices_, sizeof(VERTEX) * vertexCount_);
		Direct3D::pContext_->Unmap(pVertexBuffer_, 0);
	}


	Draw(transform);
}

void FbxParts::DrawMeshAnime(Transform& transform, FbxTime time, FbxScene* scene)
{
	Draw(transform);
}

bool FbxParts::GetBonePosition(std::string boneName, XMFLOAT3* position)
{
	for (int i = 0; i < numBone_; i++)
	{
		if (boneName == ppCluster_[i]->GetLink()->GetName())
		{
			FbxAMatrix  matrix;
			ppCluster_[i]->GetTransformLinkMatrix(matrix);

			position->x = (float)matrix[3][0];
			position->y = (float)matrix[3][1];
			position->z = (float)matrix[3][2];

			return true;
		}

	}

	return false;
}

void FbxParts::RayCast(RayCastData& rayData)
{
	for (int material = 0; material < materialCount_; material++)
	{
		for (int poly = 0; poly < indexCount_[material] / 3; poly++)
		{
			int i0 = ppIndex_[material][poly * 3 + 0];
			int i1 = ppIndex_[material][poly * 3 + 1];
			int i2 = ppIndex_[material][poly * 3 + 2];
			XMVECTOR v0 = pVertices_[i0].position;
			XMVECTOR v1 = pVertices_[i1].position;
			XMVECTOR v2 = pVertices_[i2].position;

			XMVECTOR start = XMLoadFloat4(&rayData.start);
			XMVECTOR dir = XMLoadFloat4(&rayData.dir);

			rayData.hit = TriangleTests::Intersects(start,
				XMVector4Normalize(dir), v0, v1, v2, rayData.dist);
			if (rayData.hit)
			{
				return;
			}
		}
	}
}
