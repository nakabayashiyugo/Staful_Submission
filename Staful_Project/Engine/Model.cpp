#include "Model.h"
#include "Direct3D.h"

namespace Model
{
	struct ModelData
	{
		Fbx* pFbx_;
		Transform transform_;
		std::string fileName_;
		//�A�j���[�V�����̃t���[��
		float nowFrame, animSpeed;
		int startFrame, endFrame;
		//������
		ModelData() : pFbx_(nullptr), nowFrame(0), startFrame(0), endFrame(0), animSpeed(0),
			fileName_("")
		{
		}
		//�A�j���[�V�����̃t���[�������Z�b�g
		//�����FstartFrame	�J�n�t���[��
		//�����FendFrame	�I���t���[��
		//�����FanimSpeed	�A�j���[�V�������x
		void SetAnimFrame(int start, int end, float speed)
		{
			nowFrame = (float)start;
			startFrame = start;
			endFrame = end;
			animSpeed = speed;
		}
	};

	//���f���̃|�C���^�����Ă����|�C���^
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
			//�G���[����
			MessageBox(nullptr, "���f���̃��[�h�Ɏ��s���܂����B", "�G���[", MB_OK);
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

	//�A�j���[�V������i�߂�
	modelList[hModel]->nowFrame += modelList[hModel]->animSpeed;

	//�Ō�܂ŃA�j���[�V����������߂�
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
	//�@���[���h�s��̋t�s��
	XMMATRIX wInv = XMMatrixInverse(nullptr, modelList[hModel]->transform_.GetWorldMatrix());
	//�A���C�̒ʉߓ_�����߂�
	XMVECTOR vpass{ rayData.start.x + rayData.dir.x,
					rayData.start.y + rayData.dir.y,
					rayData.start.z + rayData.dir.z,
					rayData.start.w + rayData.dir.w };
	//�BrayData.start�����f����Ԃɕϊ�
	XMVECTOR vstart = XMLoadFloat4(&rayData.start);
	vstart = XMVector3TransformCoord(vstart, wInv);
	XMStoreFloat4(&rayData.start, vstart);
	//�C(�n�_��������x�N�g����L�΂�����)�ʉߓ_(�A)�ɇ@��������
	vpass = XMVector3TransformCoord(vpass, wInv);
	//�DrayData.dir���B����C�Ɍ������x�N�g��
	vpass = vpass - vstart;
	XMStoreFloat4(&rayData.dir, vpass);


	//�w�肳�ꂽ���f���ԍ���FBX�Ƀ��C�L���X�g
	modelList[hModel]->pFbx_->RayCast(rayData);
}
