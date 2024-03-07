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

    //�e���̌����擾
    vertexCount_ = mesh->GetControlPointsCount();	//���_�̐�
    polygonCount_ = mesh->GetPolygonCount();	//�|���S���̐�
    materialCount_ = pNode->GetMaterialCount();

	HRESULT hr;

	hr = InitVertex(mesh);		//���_�o�b�t�@����
	if (FAILED(hr))
	{
		//�G���[����
		MessageBox(nullptr, "���_�f�[�^�p�o�b�t�@�̍쐬�Ɏ��s���܂���", "�G���[", MB_OK);
		return hr;
	}
	hr = InitIndex(mesh);		//�C���f�b�N�X�o�b�t�@����
	if (FAILED(hr))
	{
		//�G���[����
		MessageBox(nullptr, "�C���f�b�N�X�o�b�t�@�̍쐬�Ɏ��s���܂���", "�G���[", MB_OK);
		return hr;
	}
	hr = IntConstantBuffer();	//�R���X�^���g�o�b�t�@����
	if (FAILED(hr))
	{
		//�G���[����
		MessageBox(nullptr, "�R���X�^���g�o�b�t�@�̍쐬�Ɏ��s���܂���", "�G���[", MB_OK);
		return hr;
	}
	hr = InitMaterial(pNode);
	if (FAILED(hr))
	{
		//�G���[����
		MessageBox(nullptr, "�R���X�^���g�o�b�t�@�̍쐬�Ɏ��s���܂���", "�G���[", MB_OK);
		return hr;
	}
	hr = InitSkelton(mesh);		//���̏�������
	if (FAILED(hr))
	{
		//�G���[����
		MessageBox(nullptr, "�X�P���g�����̏������Ɏ��s���܂����B", "�G���[", MB_OK);
	}

	return hr;
}

HRESULT FbxParts::InitVertex(fbxsdk::FbxMesh* mesh)
{
	//���_��������z��
	pVertices_ = new VERTEX[vertexCount_];

	//�S�|���S��
	for (DWORD poly = 0; poly < polygonCount_; poly++)
	{
		//3���_��
		for (int vertex = 0; vertex < 3; vertex++)
		{
			int index = mesh->GetPolygonVertex(poly, vertex);

			/////////////////////////���_�̈ʒu/////////////////////////////////////
			FbxVector4 pos = mesh->GetControlPointAt(index);
			pVertices_[index].position = XMVectorSet((float)-pos[0], (float)pos[1], (float)pos[2], 0);

			/////////////////////////���_�̖@��/////////////////////////////////////
			FbxVector4 Normal;
			mesh->GetPolygonVertexNormal(poly, vertex, Normal);	//���Ԗڂ̃|���S���́A���Ԗڂ̒��_�̖@�����Q�b�g
			pVertices_[index].normal = XMVectorSet((float)-Normal[0], (float)Normal[1], (float)Normal[2], 0);

			///////////////////////////���_�̂t�u/////////////////////////////////////
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

	// ���_�f�[�^�p�o�b�t�@�̐ݒ�
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
		//�G���[����
		MessageBox(nullptr, "���_�f�[�^�p�o�b�t�@�̍쐬�Ɏ��s���܂���", "�G���[", MB_OK);
		return hr;
	}

	return S_OK;
}

HRESULT FbxParts::InitIndex(fbxsdk::FbxMesh* mesh)
{
	pIndexBuffer_ = new ID3D11Buffer * [materialCount_];
	indexCount_ = std::vector<int>(materialCount_);
	ppIndex_ = new int* [materialCount_];


	//std::vector<int> index(polygonCount_ * 3);//�|���S����*3���S���_���p��
	for (int i = 0; i < materialCount_; i++)
	{
		ppIndex_[i] = new int[polygonCount_ * 3];

		int count = 0;

		//�S�|���S��
		for (DWORD poly = 0; poly < polygonCount_; poly++)
		{
			FbxLayerElementMaterial* mtl = mesh->GetLayer(0)->GetMaterials();
			int mtlId = mtl->GetIndexArray().GetAt(poly);

			if (mtlId == i)
			{
				//3���_��
				for (DWORD vertex = 0; vertex < 3; vertex++)
				{
					ppIndex_[i][count] = mesh->GetPolygonVertex(poly, vertex);
					count++;
				}
			}
		}
		indexCount_[i] = count;

		HRESULT hr;

		// �C���f�b�N�X�o�b�t�@�𐶐�����
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
			//�G���[����
			MessageBox(nullptr, "�C���f�b�N�X�o�b�t�@�̍쐬�Ɏ��s���܂���", "�G���[", MB_OK);
			return hr;
		}
	}

	return S_OK;
}

HRESULT FbxParts::IntConstantBuffer()
{
	HRESULT hr;

	//�R���X�^���g�o�b�t�@�쐬
	D3D11_BUFFER_DESC cb;
	cb.ByteWidth = sizeof(CONSTANT_BUFFER);
	cb.Usage = D3D11_USAGE_DYNAMIC;
	cb.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cb.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	cb.MiscFlags = 0;
	cb.StructureByteStride = 0;

	// �R���X�^���g�o�b�t�@�̍쐬
	hr = Direct3D::pDevice_->CreateBuffer(&cb, nullptr, &pConstantBuffer_);
	if (FAILED(hr))
	{
		//�G���[����
		MessageBox(nullptr, "�R���X�^���g�o�b�t�@�̍쐬�Ɏ��s���܂���", "�G���[", MB_OK);
		return hr;
	}

	return S_OK;
}

HRESULT FbxParts::InitMaterial(fbxsdk::FbxNode* pNode)
{
	pMaterialList_ = new MATERIAL[materialCount_];

	for (int i = 0; i < materialCount_; i++)
	{
		//i�Ԗڂ̃}�e���A�������擾
		FbxSurfaceMaterial* pMaterial = pNode->GetMaterial(i);

		//�e�N�X�`�����
		FbxProperty  lProperty = pMaterial->FindProperty(FbxSurfaceMaterial::sDiffuse);

		//�e�N�X�`���̐���
		int fileTextureCount = lProperty.GetSrcObjectCount<FbxFileTexture>();

		//�e�N�X�`������
		if (fileTextureCount > 0)
		{
			FbxFileTexture* textureInfo = lProperty.GetSrcObject<FbxFileTexture>(0);
			const char* textureFilePath = textureInfo->GetRelativeFileName();

			//�t�@�C����+�g�������ɂ���
			char name[_MAX_FNAME];	//�t�@�C����
			char ext[_MAX_EXT];	//�g���q
			_splitpath_s(textureFilePath, nullptr, 0, nullptr, 0, name, _MAX_FNAME, ext, _MAX_EXT);
			wsprintf(name, "%s%s", name, ext);

			//�t�@�C������e�N�X�`���쐬
			pMaterialList_[i].pTexture = new Texture();
			HRESULT hr = pMaterialList_[i].pTexture->Load(name);
			assert(hr == S_OK);
		}
		//�e�N�X�`������
		else
		{
			pMaterialList_[i].pTexture = nullptr;

			//�}�e���A���̐F
			FbxSurfaceLambert* pMaterial = (FbxSurfaceLambert*)pNode->GetMaterial(i);
			FbxDouble3  diffuse = pMaterial->Diffuse;
			pMaterialList_[i].diffuse = XMFLOAT4((float)diffuse[0], (float)diffuse[1], (float)diffuse[2], 1.0f);
		}
	}
	return S_OK;
}

HRESULT FbxParts::InitSkelton(FbxMesh* pMesh)
{
	// �f�t�H�[�}���i�{�[���ƃ��f���̊֘A�t���j�̎擾
	FbxDeformer* pDeformer = pMesh->GetDeformer(0);
	if (pDeformer == nullptr)
	{
		//�{�[�����Ȃ�
		return S_OK;
	}

	// �f�t�H�[�}��񂩂�X�L�����b�V�������擾
	pSkinInfo_ = (FbxSkin*)pDeformer;

	// ���_����|���S�����t�������邽�߂̏����쐬����
	struct  POLY_INDEX
	{
		int* polyIndex;      // �|���S���̔ԍ�
		int* vertexIndex;    // ���_�̔ԍ�
		int     numRef;         // ���_�����L����|���S���̐�
	};

	POLY_INDEX* polyTable = new POLY_INDEX[vertexCount_];
	for (DWORD i = 0; i < vertexCount_; i++)
	{
		// �O�p�`�|���S���ɍ��킹�āA���_�ƃ|���S���̊֘A�����\�z����
		// �����_�����|���S�����~�R���_
		polyTable[i].polyIndex = new int[polygonCount_ * 3];
		polyTable[i].vertexIndex = new int[polygonCount_ * 3];
		polyTable[i].numRef = 0;
		ZeroMemory(polyTable[i].polyIndex, sizeof(int) * polygonCount_ * 3);
		ZeroMemory(polyTable[i].vertexIndex, sizeof(int) * polygonCount_ * 3);

		// �|���S���Ԃŋ��L���钸�_��񋓂���
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

	// �{�[�������擾����
	numBone_ = pSkinInfo_->GetClusterCount();
	ppCluster_ = new FbxCluster * [numBone_];
	for (int i = 0; i < numBone_; i++)
	{
		ppCluster_[i] = pSkinInfo_->GetCluster(i);
	}

	// �{�[���̐��ɍ��킹�ăE�F�C�g������������
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

	// ���ꂼ��̃{�[���ɉe�����󂯂钸�_�𒲂ׂ�
	// ��������t�ɁA���_�x�[�X�Ń{�[���C���f�b�N�X�E�d�݂𐮓ڂ���
	for (int i = 0; i < numBone_; i++)
	{
		int numIndex = ppCluster_[i]->GetControlPointIndicesCount();   //���̃{�[���ɉe�����󂯂钸�_��
		int* piIndex = ppCluster_[i]->GetControlPointIndices();       //�{�[��/�E�F�C�g���̔ԍ�
		double* pdWeight = ppCluster_[i]->GetControlPointWeights();     //���_���Ƃ̃E�F�C�g���

		//���_������C���f�b�N�X�����ǂ��āA���_�T�C�h�Ő�������
		for (int k = 0; k < numIndex; k++)
		{
			// ���_�Ɋ֘A�t����ꂽ�E�F�C�g��񂪃{�[���T�{�ȏ�̏ꍇ�́A�d�݂̑傫�����ɂS�{�ɍi��
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

	//�{�[���𐶐�
	pBoneArray_ = new Bone[numBone_];
	for (int i = 0; i < numBone_; i++)
	{
		// �{�[���̃f�t�H���g�ʒu���擾����
		FbxAMatrix  matrix;
		ppCluster_[i]->GetTransformLinkMatrix(matrix);

		// �s��R�s�[�iFbx�`������DirectX�ւ̕ϊ��j
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

	// �ꎞ�I�ȃ������̈���������
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
	transform.Calclation();//�g�����X�t�H�[�����v�Z

	for (int i = 0; i < materialCount_; i++)
	{
		//�R���X�^���g�o�b�t�@�ɓn�����
		CONSTANT_BUFFER cb;
		cb.matWVP = XMMatrixTranspose(transform.GetWorldMatrix() * Camera::GetViewMatrix() * Camera::GetProjectionMatrix());
		cb.matNormal = XMMatrixTranspose(transform.GetWorldMatrix());
		cb.diffuseColor = pMaterialList_[i].diffuse;
		cb.isTexture = pMaterialList_[i].pTexture != nullptr;


		D3D11_MAPPED_SUBRESOURCE pdata;
		Direct3D::pContext_->Map(pConstantBuffer_, 0, D3D11_MAP_WRITE_DISCARD, 0, &pdata);	// GPU����̃f�[�^�A�N�Z�X���~�߂�
		memcpy_s(pdata.pData, pdata.RowPitch, (void*)(&cb), sizeof(cb));	// �f�[�^��l�𑗂�

		Direct3D::pContext_->Unmap(pConstantBuffer_, 0);	//�ĊJ

		//���_�o�b�t�@
		UINT stride = sizeof(VERTEX);
		UINT offset = 0;
		Direct3D::pContext_->IASetVertexBuffers(0, 1, &pVertexBuffer_, &stride, &offset);

		// �C���f�b�N�X�o�b�t�@�[���Z�b�g
		stride = sizeof(int);
		offset = 0;
		Direct3D::pContext_->IASetIndexBuffer(pIndexBuffer_[i], DXGI_FORMAT_R32_UINT, 0);

		//�R���X�^���g�o�b�t�@
		Direct3D::pContext_->VSSetConstantBuffers(0, 1, &pConstantBuffer_);	//���_�V�F�[�_�[�p	
		Direct3D::pContext_->PSSetConstantBuffers(0, 1, &pConstantBuffer_);	//�s�N�Z���V�F�[�_�[�p

		if (pMaterialList_[i].pTexture)
		{
			ID3D11SamplerState* pSampler = pMaterialList_[i].pTexture->GetSampler();
			Direct3D::pContext_->PSSetSamplers(0, 1, &pSampler);
			ID3D11ShaderResourceView* pSRV = pMaterialList_[i].pTexture->GetSRV();
			Direct3D::pContext_->PSSetShaderResources(0, 1, &pSRV);
		}


		//�`��
		Direct3D::pContext_->DrawIndexed(indexCount_[i], 0, 0);
	}
}

void FbxParts::DrawSkinAnime(Transform& transform, FbxTime time)
{
	// �{�[�����Ƃ̌��݂̍s����擾����
	for (int i = 0; i < numBone_; i++)
	{
		FbxAnimEvaluator* evaluator = ppCluster_[i]->GetLink()->GetScene()->GetAnimationEvaluator();
		FbxMatrix mCurrentOrentation = evaluator->GetNodeGlobalTransform(ppCluster_[i]->GetLink(), time);

		// �s��R�s�[�iFbx�`������DirectX�ւ̕ϊ��j
		XMFLOAT4X4 pose;
		for (DWORD x = 0; x < 4; x++)
		{
			for (DWORD y = 0; y < 4; y++)
			{
				pose(x, y) = (float)mCurrentOrentation.Get(x, y);
			}
		}

		// �I�t�Z�b�g���̃|�[�Y�̍������v�Z����
		pBoneArray_[i].newPose = XMLoadFloat4x4(&pose);
		pBoneArray_[i].diffPose = XMMatrixInverse(nullptr, pBoneArray_[i].bindPose);
		pBoneArray_[i].diffPose *= pBoneArray_[i].newPose;
	}

	// �e�{�[���ɑΉ��������_�̕ό`����
	for (DWORD i = 0; i < vertexCount_; i++)
	{
		// �e���_���ƂɁA�u�e������{�[���~�E�F�C�g�l�v�𔽉f�������֐ߍs����쐬����
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

		// �쐬���ꂽ�֐ߍs����g���āA���_��ό`����
		XMVECTOR Pos = XMLoadFloat3(&pWeightArray_[i].posOrigin);
		XMVECTOR Normal = XMLoadFloat3(&pWeightArray_[i].normalOrigin);
		pVertices_[i].position = XMVector3TransformCoord(Pos, matrix);
		pVertices_[i].normal = XMVector3TransformCoord(Normal, matrix);

	}

	// ���_�o�b�t�@�����b�N���āA�ό`��������̒��_���ŏ㏑������
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
