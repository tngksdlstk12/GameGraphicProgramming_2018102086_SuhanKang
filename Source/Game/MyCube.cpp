#include "MyCube.h"

MyCube::MyCube() {
    m_rotatingAmount = 0;
}
MyCube::~MyCube() {}
void MyCube::Update(_In_ FLOAT deltaTime) {
    m_rotatingAmount += deltaTime;
    XMMATRIX mOrbit = XMMatrixRotationX(-2.0f * m_rotatingAmount);
    XMMATRIX mSpin = XMMatrixRotationZ(-1.0f * m_rotatingAmount);
    XMMATRIX mTranslate = XMMatrixTranslation(0.0f, 4.0f, 0.0f);
    XMMATRIX mScale = XMMatrixScaling(0.5f, 0.5f, 0.5f);
    m_world = mScale * mSpin * mTranslate * mOrbit;
}