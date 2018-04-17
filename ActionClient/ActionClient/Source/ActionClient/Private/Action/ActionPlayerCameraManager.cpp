// Fill out your copyright notice in the Description page of Project Settings.

#include "ActionClient.h"
#include "ActionPlayerCameraManager.h"
#include "ActionCharacter.h"




AActionPlayerCameraManager::AActionPlayerCameraManager( const FObjectInitializer& ObjectInitializer ) : Super( ObjectInitializer )
{
	//NormalFOV = 90.0f;
	//TargetingFOV = 60.0f;
	ViewPitchMin = -87.0f;
	ViewPitchMax = 87.0f;
	bAlwaysApplyModifiers = true;
}

void AActionPlayerCameraManager::UpdateCamera( float DeltaTime )
{
	AActionCharacter* MyPawn = PCOwner ? Cast<AActionCharacter>( PCOwner->GetPawn() ) : NULL;
	//if (MyPawn && MyPawn->IsFirstPerson())
	//{
	//	const float TargetFOV = MyPawn->IsTargeting() ? TargetingFOV : NormalFOV;
	//	DefaultFOV = FMath::FInterpTo( DefaultFOV, TargetFOV, DeltaTime, 20.0f );
	//}

	Super::UpdateCamera( DeltaTime );

	if (MyPawn && MyPawn->IsFirstPerson())
	{
		MyPawn->OnCameraUpdate( GetCameraLocation(), GetCameraRotation() );
	}
}

