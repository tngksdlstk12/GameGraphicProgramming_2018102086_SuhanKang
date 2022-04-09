#pragma once
#include "Cube/BaseCube.h"

class SatelliteCube : public BaseCube {
public:
	SatelliteCube();
	~SatelliteCube();
	virtual void Update(_In_ FLOAT deltaTime) override;

private: 
	float  m_rotatingAmount;
};