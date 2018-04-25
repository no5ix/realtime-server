// Fill out your copyright notice in the Description page of Project Settings.

#include "ActionClient.h"
#include "ActionGameModeBase.h"
#include "ActionPlayerController.h"




AActionGameModeBase::AActionGameModeBase()
{
	PlayerControllerClass = AActionPlayerController::StaticClass();
}

//UGameObjectRegistryUObj* AActionGameModeBase::GetGameObjectRegistryUObj()
//{
//	if (mGameObjectRegistryUObj == nullptr)
//	{
//		mGameObjectRegistryUObj = NewObject<UGameObjectRegistryUObj>();
//		mGameObjectRegistryUObj->SetDefaultCharacterClasses( DefaultCharacterClasses );
//		//if (mGameObjectRegistryUObj)
//		//{
//		//	ActionHelper::OutputLog( "mGameObjectRegistryUObj is not null" );
//		//}
//		//else
//		//{
//		//	ActionHelper::OutputLog( "mGameObjectRegistryUObj is null" );
//		//}
//	}
//	return mGameObjectRegistryUObj;
//
//}
