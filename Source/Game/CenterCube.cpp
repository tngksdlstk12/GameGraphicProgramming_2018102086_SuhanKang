#include "CenterCube.h"

CenterCube::CenterCube() {}
CenterCube::~CenterCube(){}
void CenterCube::Update(_In_ FLOAT deltaTime) {
	RotateY(deltaTime);
}