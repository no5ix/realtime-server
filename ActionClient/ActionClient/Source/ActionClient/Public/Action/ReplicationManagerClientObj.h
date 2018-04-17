// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "UObject/NoExportTypes.h"
#include "ActionCharacter.h"
#include "ReplicationManagerClientObj.generated.h"

/**
 * 
 */
UCLASS()
class UReplicationManagerClientObj : public UObject
{
	GENERATED_BODY()

public:
	enum ReplicationAction
	{
		RA_Create,
		RA_Update,
		RA_Destroy,
		RA_RPC,
		RA_MAX
	};
public:
	void Read( InputMemoryBitStream& inInputStream );

	virtual UWorld* GetWorld() const override;

	void SetDefaultCharacterClasses( TSubclassOf<class AActionCharacter> inDefaultCharacterClasses ) { DefaultCharacterClasses = inDefaultCharacterClasses; }

private:

	void ReadAndDoCreateAction( InputMemoryBitStream& inInputStream, int inNetworkId );
	void ReadAndDoUpdateAction( InputMemoryBitStream& inInputStream, int inNetworkId );
	void ReadAndDoDestroyAction( InputMemoryBitStream& inInputStream, int inNetworkId );

	TSubclassOf<class AActionCharacter> DefaultCharacterClasses;
	
	
};
