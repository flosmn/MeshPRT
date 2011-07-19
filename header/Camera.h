#ifndef CAMERA_H
#define CAMERA_H

#include "d3dUtil.h"
#include "DirectInput.h"

class Camera
{
public:
	Camera();

	const D3DXMATRIX& view() const;
	D3DXVECTOR3& pos();
	void setSpeed(float s);

	void update(float dt);

protected:
	void buildView();

protected:
	D3DXMATRIX mView;

	// Relative to world space.
	D3DXVECTOR3 mPosW;
	D3DXVECTOR3 mRightW;
	D3DXVECTOR3 mUpW;
	D3DXVECTOR3 mLookW;

	float mSpeed;
};

#endif // CAMERA_H
