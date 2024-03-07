#pragma once
#include <string>
#include <vector>
#include "Transform.h"
#include "Sprite.h"

namespace Image
{
	void SetTransform(int hImage, Transform transform);
	int Load(std::string filename);
	void Draw(int hImage);
	void Release(int _handle);
	void AllRelease();

	void SetRect(int handle, int x, int y, int width, int height);
	void ResetRect(int handle);

	XMFLOAT3 GetTextureSize(int hImage);
}
