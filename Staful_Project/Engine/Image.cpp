#include "Image.h"

namespace Image
{
	struct ImageData
	{
		Sprite* pSprite_;
		Transform transform_;
		std::string fileName_;
		RECT rect_;
		XMFLOAT3 size_;
	};

	//モデルのポインタを入れておくポインタ
	std::vector<ImageData*> imageList;
}

void Image::SetTransform(int hImage, Transform transform)
{
	imageList[hImage]->transform_ = transform;
}

int Image::Load(std::string filename)
{
	ImageData* pData = new ImageData();
	pData->fileName_ = filename;
	pData->pSprite_ = nullptr;
	for (auto itr = imageList.begin(); itr != imageList.end(); itr++)
	{
		if ((*itr)->fileName_ == filename)
		{
			pData->pSprite_ = (*itr)->pSprite_;
			break;
		}
	}
	if (pData->pSprite_ == nullptr)
	{
		pData->pSprite_ = new Sprite();
		pData->pSprite_->Load(filename);
	}
	pData->size_ = pData->pSprite_->GetTextureSize();
	imageList.push_back(pData);
	return (imageList.size() - 1);
}

void Image::Draw(int hImage)
{
	imageList[hImage]->pSprite_->Draw(imageList[hImage]->transform_, imageList[hImage]->rect_);
}

void Image::Release(int handle)
{
	if (handle < 0 || handle >= imageList.size())
	{
		return;
	}

	//同じモデルを他でも使っていないか
	bool isExist = false;
	for (int i = 0; i < imageList.size(); i++)
	{
		//すでに開いている場合
		if (imageList[i] != nullptr && i != handle && imageList[i]->pSprite_ == imageList[handle]->pSprite_)
		{
			isExist = true;
			break;
		}
	}

	//使ってなければモデル解放
	if (isExist == false)
	{
		SAFE_DELETE(imageList[handle]->pSprite_);
	}

	SAFE_DELETE(imageList[handle]);

}

void Image::AllRelease()
{
	for (int i = 0; i < imageList.size(); i++)
	{
		Release(i);
	}
	imageList.clear();
}

void Image::SetRect(int handle, int x, int y, int width, int height)
{
	if (handle < 0 || handle >= imageList.size())
	{
		return;
	}

	imageList[handle]->rect_.left = x;
	imageList[handle]->rect_.top = y;
	imageList[handle]->rect_.right = width;
	imageList[handle]->rect_.bottom = height;
}

void Image::ResetRect(int handle)
{
	if (handle < 0 || handle >= imageList.size())
	{
		return;
	}

	XMFLOAT3 size = imageList[handle]->pSprite_->GetTextureSize();

	imageList[handle]->rect_.left = 0;
	imageList[handle]->rect_.top = 0;
	imageList[handle]->rect_.right = (long)imageList[handle]->size_.x;
	imageList[handle]->rect_.bottom = (long)imageList[handle]->size_.y;
}

XMFLOAT3 Image::GetTextureSize(int hImage)
{
	return imageList[hImage]->size_;
}
