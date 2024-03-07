#include "RootJob.h"
#include "SceneManager.h"

RootJob::RootJob(GameObject* parent)
{
}

RootJob::~RootJob()
{
}

void RootJob::Initialize()
{
	Instantiate<SceneManager>(this);
}

void RootJob::Update()
{
}

void RootJob::Draw()
{
	/*for (auto& e : childList_)
	{
		e->Draw();
	}*/
}

void RootJob::Release()
{
}
