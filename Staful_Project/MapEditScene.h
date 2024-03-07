#pragma once
#include "StageOrigin.h"

class Text;

class MapEditScene
	:public GameObject, StageOrigin
{
	int hPict_[MATH_MAX + 1];
	int hTgtgRoute_;


	int YSIZE;
	int mathtype_;
	int save_Num_;
	int mathChangeNumLimit_;

	XMFLOAT3 texture_size_;

	std::vector<std::vector<MATHDEDAIL>> math_origin_;
	std::vector<std::vector<bool>> isConvRot_;

	Text* pText_;
public:
	MapEditScene(GameObject* parent);

	//‰Šú‰»
	void Initialize() override;

	//XV
	void Update() override;

	//•`‰æ
	void Draw() override;

	//ŠJ•ú
	void Release() override;

	BOOL DialogProc(HWND hDlg, UINT msg, WPARAM wp, LPARAM lp);

	void Write();
	void Read();

	bool isMathChangeNumLimit();
};

