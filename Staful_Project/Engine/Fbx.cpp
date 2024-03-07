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
	//�}�l�[�W���𐶐�
	pFbxManager_ = FbxManager::Create();

	//�C���|�[�^�[�𐶐�
	FbxImporter* fbxImporter = FbxImporter::Create(pFbxManager_, "imp");
	fbxImporter->Initialize(fileName.c_str(), -1, pFbxManager_->GetIOSettings());

	//�V�[���I�u�W�F�N�g��FBX�t�@�C���̏��𗬂�����
	pFbxScene_ = FbxScene::Create(pFbxManager_, "fbxscene");
	fbxImporter->Import(pFbxScene_);
	fbxImporter->Destroy();

	// �A�j���[�V�����̃^�C�����[�h�̎擾
	frameRate_ = pFbxScene_->GetGlobalSettings().GetTimeMode();

	//���݂̃J�����g�f�B���N�g�����o���Ă���
	char defaultCurrentDir[MAX_PATH];
	GetCurrentDirectory(MAX_PATH, defaultCurrentDir);

	//������fileName����f�B���N�g���������擾
	char dir[MAX_PATH];
	_splitpath_s(fileName.c_str(), nullptr, 0, dir, MAX_PATH, nullptr, 0, nullptr, 0);

	//�J�����g�f�B���N�g���ύX
	SetCurrentDirectory(dir);

	FbxNode* rootNode = pFbxScene_->GetRootNode();
	//�����̎q���̐��𒲂ׂ�
	int childCount = rootNode->GetChildCount();
	//1���`�F�b�N
	for (int i = 0; childCount > i; i++)
	{
		CheckNode(rootNode->GetChild(i), &parts_);
	}

	//�J�����g�f�B���N�g�������ɖ߂�
	SetCurrentDirectory(defaultCurrentDir);

	//�}�l�[�W�����
	//pFbxManager_->Destroy();
	return S_OK;
}

void Fbx::CheckNode(FbxNode* pNode, std::vector<FbxParts*>* pPartsList)
{
	//���̃m�[�h�ɂ̓��b�V����񂪓����Ă��邾�낤���H
	FbxNodeAttribute* attr = pNode->GetNodeAttribute();
	if (attr != nullptr && attr->GetAttributeType() == FbxNodeAttribute::eMesh)
	{
		//�p�[�c��p��
		FbxParts* pParts = new FbxParts;
		if (FAILED(pParts->Init(pNode)))
		{
			//�G���[����
			MessageBox(nullptr, "�X�P���g�����̏������Ɏ��s���܂����B", "�G���[", MB_OK);
		}

		//�p�[�c���𓮓I�z��ɒǉ�
		pPartsList->push_back(pParts);
	}


	//�q�m�[�h�ɂ��f�[�^�����邩���I�I
	{
		//�q���̐��𒲂ׂ�
		int childCount = pNode->GetChildCount();

		//��l���`�F�b�N
		for (int i = 0; i < childCount; i++)
		{
			CheckNode(pNode->GetChild(i), pPartsList);
		}
	}
}

void Fbx::Draw(Transform& transform, int frame)
{
	Direct3D::SetShader(SHADER_3D);

	//�p�[�c��1���`��
	for (int k = 0; k < parts_.size(); k++)
	{
		// ���̏u�Ԃ̎����̎p���s��𓾂�
		FbxTime     time;
		time.SetTime(0, 0, 0, frame, 0, 0, frameRate_);

		//�X�L���A�j���[�V�����i�{�[���L��j�̏ꍇ
		if (parts_[k]->GetSkinInfo() != nullptr)
		{
			parts_[k]->DrawSkinAnime(transform, time);
		}

		//���b�V���A�j���[�V�����̏ꍇ
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
	//���ׂẴp�[�c�Ɣ���
	for (int i = 0; i < parts_.size(); i++)
	{
		parts_[i]->RayCast(rayData);
	}
}
