// Fill out your copyright notice in the Description page of Project Settings.

#pragma once


#include "GameFramework/Actor.h"
#include "RealTimeSrvEngine.h"
#include "PutMeInTheMap.generated.h"

UCLASS()
class APutMeInTheMap : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	APutMeInTheMap();

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
