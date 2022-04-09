#pragma once
#include "Cube/BaseCube.h"

class CenterCube : public BaseCube {
public:
	CenterCube();
	~CenterCube();
	virtual void Update(_In_ FLOAT deltaTime) override;
};