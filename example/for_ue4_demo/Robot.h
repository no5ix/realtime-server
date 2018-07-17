//#pragma once
//
//
//#include "Character.h"
//
//
//
//class Robot : public Character
//{
//	virtual void AfterProcessInput()
//	{
//
//		curRotation_ = Vector3( 0.f,
//			RealtimeSrvMath::GetRandomFloat() * 180.f,
//			0.f );
//
//		curCameraRotation_ = Vector3( RealtimeSrvMath::GetRandomFloat() * 180.f,
//			RealtimeSrvMath::GetRandomFloat() * 180.f,
//			RealtimeSrvMath::GetRandomFloat() * 180.f );
//
//
//		AddActionInput( curCameraRotation_.ToQuaternion() * Vector3::Forward(),
//			inInputState->GetDesiredMoveForwardAmount() );
//
//		AddActionInput( curCameraRotation_.ToQuaternion() * Vector3::Right(),
//			inInputState->GetDesiredMoveRightAmount() );
//	}
//
//};