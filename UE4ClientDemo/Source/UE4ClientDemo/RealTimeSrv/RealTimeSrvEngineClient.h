// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/Actor.h"
#include "RealTimeSrvEngine.h"
#include "RealTimeSrvEngineClient.generated.h"

UCLASS()
class ARealTimeSrvEngineClient : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ARealTimeSrvEngineClient();

//protected:
//	// Called when the game starts or when spawned
//	virtual void BeginPlay() override;
//
//public:	
//	// Called every frame
//	virtual void Tick(float DeltaTime) override;

public:

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = RealTimeSrvEngine )
		URealTimeSrvEngine* RealTimeSrvEngine;
	
};
