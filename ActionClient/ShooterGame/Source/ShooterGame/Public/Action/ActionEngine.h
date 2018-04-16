// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Components/ActorComponent.h"
#include "ActionEngine.generated.h"


UCLASS(ClassGroup = "ActionEngine", blueprintable, editinlinenew, hidecategories = (Object, LOD, Lighting, TextureStreaming), meta = (DisplayName = "ActionEngine Main", BlueprintSpawnableComponent))
class UActionEngine : public UActorComponent
{

	GENERATED_UCLASS_BODY()

public:	
	// Sets default values for this component's properties
	UActionEngine();
	
	protected:
		// Called when the game starts
		virtual void BeginPlay() override;
	
	public:	
		// Called every frame
		virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = KBEngine)
		FString ip;

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = KBEngine )
		FString player_name;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = KBEngine)
		int port;
};
