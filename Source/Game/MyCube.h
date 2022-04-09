#pragma once
#include "Cube/BaseCube.h"

class MyCube : public BaseCube {
public:
	MyCube();
	~MyCube();
	virtual void Update(_In_ FLOAT deltaTime) override;

private:
	float  m_rotatingAmount;
};