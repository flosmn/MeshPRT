#include "Camera.h"

Camera::Camera()
{
	D3DXMatrixIdentity(&mView);

	mPosW   = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
	mRightW = D3DXVECTOR3(1.0f, 0.0f, 0.0f);
	mUpW    = D3DXVECTOR3(0.0f, 1.0f, 0.0f);
	mLookW  = D3DXVECTOR3(0.0f, 0.0f, 1.0f);

	// Client should adjust to a value that makes sense for application's
	// unit scale, and the object the camera is attached to--e.g., car, jet,
	// human walking, etc.
	mSpeed  = 10.0f;
}

const D3DXMATRIX& Camera::view() const
{
	return mView;
}

D3DXVECTOR3& Camera::pos()
{
	return mPosW;
}

void Camera::setSpeed(float s)
{
	mSpeed = s;
}

void Camera::update(float dt)
{
	// Find the net direction the camera is traveling in (since the
	// camera could be running and strafing).
	D3DXVECTOR3 dir(0.0f, 0.0f, 0.0f);
	if( gDInput->keyDown(DIK_W) )
		dir += mLookW;
	if( gDInput->keyDown(DIK_S) )
		dir -= mLookW;
	if( gDInput->keyDown(DIK_D) )
		dir += mRightW;
	if( gDInput->keyDown(DIK_A) )
		dir -= mRightW;

	// Move at mSpeed along net direction.
	D3DXVec3Normalize(&dir, &dir);
	mPosW = mPosW + dir*mSpeed*dt;

	// We rotate at a fixed speed.
	float pitch  = 0.0f;
	float yAngle = 0.0f;

  if( gDInput->keyDown(DIK_G) )
		yAngle -= dt;
	if( gDInput->keyDown(DIK_J) )
		yAngle += dt;
	if( gDInput->keyDown(DIK_Y) )
		pitch -= dt;
	if( gDInput->keyDown(DIK_H) )
		pitch += dt;

	// Rotate camera's look and up vectors around the camera's right vector.
	D3DXMATRIX R;
	D3DXMatrixRotationAxis(&R, &mRightW, pitch);
	D3DXVec3TransformCoord(&mLookW, &mLookW, &R);
	D3DXVec3TransformCoord(&mUpW, &mUpW, &R);

	// Rotate camera axes about the world's y-axis.
	D3DXMatrixRotationAxis(&R, &mUpW, yAngle);
	D3DXVec3TransformCoord(&mRightW, &mRightW, &R);
	D3DXVec3TransformCoord(&mUpW, &mUpW, &R);
	D3DXVec3TransformCoord(&mLookW, &mLookW, &R);

	// Rebuild the view matrix to reflect changes.
	buildView();
}

void Camera::buildView()
{
	// Keep camera's axes orthogonal to each other and of unit length.
	D3DXVec3Normalize(&mLookW, &mLookW);

	D3DXVec3Cross(&mUpW, &mLookW, &mRightW);
	D3DXVec3Normalize(&mUpW, &mUpW);

	D3DXVec3Cross(&mRightW, &mUpW, &mLookW);
	D3DXVec3Normalize(&mRightW, &mRightW);

  D3DXMatrixLookAtLH(&mView, &mPosW, &(mPosW+mLookW), &mUpW);
}
 
