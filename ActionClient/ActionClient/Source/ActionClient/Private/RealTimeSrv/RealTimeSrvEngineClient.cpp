// Fill out your copyright notice in the Description page of Project Settings.

#include "ActionClient.h"
#include "RealTimeSrvEngineClient.h"


// Sets default values
ARealTimeSrvEngineClient::ARealTimeSrvEngineClient()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	//PrimaryActorTick.bCanEverTick = true;
	RealTimeSrvEngine = CreateDefaultSubobject<URealTimeSrvEngine>( TEXT( "RealTimeSrvEngine" ) );
}

//// Called when the game starts or when spawned
//void ARealTimeSrvEngineClient::BeginPlay()
//{
//	Super::BeginPlay();
//	
//}
//
//// Called every frame
//void ARealTimeSrvEngineClient::Tick(float DeltaTime)
//{
//	Super::Tick(DeltaTime);
//
//}

