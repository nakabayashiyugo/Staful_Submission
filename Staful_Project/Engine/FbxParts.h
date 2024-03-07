#pragma once
#include <fbxsdk.h>
#include <d3d11.h>
#include <DirectXMath.h>
#include "Texture.h"
#include "Transform.h"
#include <vector>

using namespace DirectX;

class Fbx;
struct RayCastData;


#pragma comment(lib, "LibFbxSDK-MD.lib")
#pragma comment(lib, "LibXml2-MD.lib")
#pragma comment(lib, "zlib-MD.lib")

struct RayCastData;


class FbxParts
{
	//�}�e���A��
	struct MATERIAL
	{
		Texture* pTexture;
		XMFLOAT4	diffuse;
	};

	struct CONSTANT_BUFFER
	{
		XMMATRIX	matWVP;
		XMMATRIX	matNormal;
		XMFLOAT4    diffuseColor;		// �f�B�t���[�Y�J���[�i�}�e���A���̐F�j
		int		    isTexture;		// �e�N�X�`���\���Ă��邩�ǂ���
	};

	struct VERTEX
	{
		XMVECTOR position;
		XMVECTOR uv;
		XMVECTOR normal;
	};

	// �{�[���\���́i�֐ߏ��j
	struct  Bone
	{
		XMMATRIX  bindPose;      // �����|�[�Y���̃{�[���ϊ��s��
		XMMATRIX  newPose;       // �A�j���[�V�����ŕω������Ƃ��̃{�[���ϊ��s��
		XMMATRIX  diffPose;      // mBindPose �ɑ΂��� mNowPose �̕ω���
	};

	// �E�F�C�g�\���́i�{�[���ƒ��_�̊֘A�t���j
	struct Weight
	{
		XMFLOAT3	posOrigin;		// ���X�̒��_���W
		XMFLOAT3	normalOrigin;	// ���X�̖@���x�N�g��
		int* pBoneIndex;		// �֘A����{�[����ID
		float* pBoneWeight;	// �{�[���̏d��
	};

	

	VERTEX* pVertices_;
	int** ppIndex_;
	int vertexCount_;	//���_��
	int polygonCount_;	//�|���S����
	int materialCount_;	//�}�e���A���̌�

	ID3D11Buffer* pVertexBuffer_;
	ID3D11Buffer** pIndexBuffer_;
	ID3D11Buffer* pConstantBuffer_;
	MATERIAL* pMaterialList_;
	std::vector<int> indexCount_;

	//�{�[�����
	FbxSkin*		pSkinInfo_;		// �X�L�����b�V�����i�X�L�����b�V���A�j���[�V�����̃f�[�^�{�́j
	FbxCluster**	ppCluster_;		// �N���X�^���i�֐߂��ƂɊ֘A�t����ꂽ���_���j
	int				numBone_;		// Fbx�Ɋ܂܂�Ă���֐߂̐�
	Bone*			pBoneArray_;	// �e�֐߂̏��
	Weight*			pWeightArray_;	// �E�F�C�g���i���_�̑΂���e�֐߂̉e���x�����j

public:

	FbxParts();
	HRESULT Init(FbxNode* pNode);

	HRESULT InitVertex(fbxsdk::FbxMesh* mesh);
	HRESULT InitIndex(fbxsdk::FbxMesh* mesh);		//�C���f�b�N�X�o�b�t�@����
	HRESULT IntConstantBuffer();
	HRESULT InitMaterial(fbxsdk::FbxNode* pNode);
	HRESULT InitSkelton(FbxMesh* pMesh);			//���̏�������

	void    Draw(Transform& transform);

	//�{�[���L��̃��f����`��
	//�����Ftransform	�s����
	//�����Ftime		�t���[�����i�P�A�j���[�V�������̍��ǂ����j
	void DrawSkinAnime(Transform& transform, FbxTime time);

	//�{�[�������̃��f����`��
	//�����Ftransform	�s����
	//�����Ftime		�t���[�����i�P�A�j���[�V�������̍��ǂ����j
	//�����Fscene		Fbx�t�@�C������ǂݍ��񂾃V�[�����
	void DrawMeshAnime(Transform& transform, FbxTime time, FbxScene* scene);

	//�C�ӂ̃{�[���̈ʒu���擾
	//�����FboneName	�擾�������{�[���̈ʒu
	//�ߒl�F�{�[���̈ʒu
	bool GetBonePosition(std::string boneName, XMFLOAT3* position);

	//�X�L�����b�V�������擾
	//�ߒl�F�X�L�����b�V�����
	FbxSkin* GetSkinInfo() { return pSkinInfo_; }

	void RayCast(RayCastData& rayData);
};