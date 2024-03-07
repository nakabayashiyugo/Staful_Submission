#pragma once
#include "GameObject.h"

class RootJob :
    public GameObject
{
public:
    RootJob(GameObject* parent);
    ~RootJob();

    virtual void Initialize() override;
    virtual void Update() override;
    virtual void Draw() override;
    virtual void Release() override;
};

