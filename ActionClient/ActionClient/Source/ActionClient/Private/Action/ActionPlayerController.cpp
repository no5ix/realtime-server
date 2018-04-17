
// Fill out your copyright notice in the Description page of Project Settings.

#include "ActionClient.h"
#include "ActionPlayerController.h"
#include "ActionPlayerCameraManager.h"




AActionPlayerController::AActionPlayerController()
{

	PlayerCameraManagerClass = AActionPlayerCameraManager::StaticClass();

}
