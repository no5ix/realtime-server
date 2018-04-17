// Fill out your copyright notice in the Description page of Project Settings.

#include "ShooterGame.h"
#include "ReplicationManagerClientObj.h"




void UReplicationManagerClientObj::Read( InputMemoryBitStream& inInputStream )
{
	UWorld* const World = GetWorld();

	UE_LOG( LogTemp, Log, TEXT( "\n\n\n~~~~~~~~~~~~~~~~~~~~~~~~~~~~~" ) );
	UE_LOG( LogTemp, Log, TEXT( "****Read;****  Successfully!!!" ) );
	UE_LOG( LogTemp, Log, TEXT( "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n\n\n" ) );

	UE_LOG( LogTemp, Log, TEXT( "\n\n\n~~~~~~~~~~~~~~~~~~~~~~~~~~~~~" ) );
	UE_LOG( LogTemp, Log, TEXT( "****if (World);****  Successfully!!!" ) );
	UE_LOG( LogTemp, Log, TEXT( "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n\n\n" ) );

	UE_LOG( LogTemp, Log, TEXT( "\n\n\n~~~~~~~~~~~~~~~~~~~~~~~~~~~~~" ) );
	UE_LOG( LogTemp, Log, TEXT( "****nnnnnnsif (World);****  Successfully!!!" ) );
	UE_LOG( LogTemp, Log, TEXT( "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n\n\n" ) );

	if (World)
	{

		UE_LOG( LogTemp, Log, TEXT( "\n\n\n~~~~~~~~~~~~~~~~~~~~~~~~~~~~~" ) );
		UE_LOG( LogTemp, Log, TEXT( "****World;****  Successfully!!!" ) );
		UE_LOG( LogTemp, Log, TEXT( "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n\n\n" ) );
		AActionPlayerController* const FirstPC = Cast<AActionPlayerController>( UGameplayStatics::GetPlayerController( GetWorld(), 0 ) );
		if (FirstPC != nullptr)
		{

			UE_LOG( LogTemp, Log, TEXT( "\n\n\n~~~~~~~~~~~~~~~~~~~~~~~~~~~~~" ) );
			UE_LOG( LogTemp, Log, TEXT( "****FirstPC;****  Successfully!!!" ) );
			UE_LOG( LogTemp, Log, TEXT( "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n\n\n" ) );

			//FRotator Rot( 0.f, 0.f, 0.f );
			//FTransform SpawnTransform( Rot, FVector::ZeroVector );

			//TSubclassOf<class AActionCharacter> CharacterClass;
			//AActionCharacter* DeferredActor = Cast<AActionCharacter>( UGameplayStatics::BeginDeferredActorSpawnFromClass( this, CharacterClass, SpawnTransform ) );
			//if (DeferredActor)
			//{
			//	UGameplayStatics::FinishSpawningActor( DeferredActor, SpawnTransform );
			//}

			//TSubclassOf<AActor> ActorClass
			FActorSpawnParameters SpawnParams;
			SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
			AActionCharacter* const newActionCharacter = World->SpawnActor<AActionCharacter>( AActionCharacter::StaticClass(), FTransform::Identity, SpawnParams );

			UE_LOG( LogTemp, Log, TEXT( "\n\n\n~~~~~~~~~~~~~~~~~~~~~~~~~~~~~" ) );
			UE_LOG( LogTemp, Log, TEXT( "***newActionCharacter****  Successfully!!!" ) );
			UE_LOG( LogTemp, Log, TEXT( "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n\n\n" ) );

			FirstPC->Possess( newActionCharacter );

			UE_LOG( LogTemp, Log, TEXT( "\n\n\n~~~~~~~~~~~~~~~~~~~~~~~~~~~~~" ) );
			UE_LOG( LogTemp, Log, TEXT( "****FirstPC->Possess( newActionCharacter );****  Successfully!!!" ) );
			UE_LOG( LogTemp, Log, TEXT( "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n\n\n" ) );
		}
	}
}

UWorld* UReplicationManagerClientObj::GetWorld() const
{
	return GWorld;
}

void UReplicationManagerClientObj::ReadAndDoCreateAction( InputMemoryBitStream& inInputStream, int inNetworkId )
{

}

void UReplicationManagerClientObj::ReadAndDoUpdateAction( InputMemoryBitStream& inInputStream, int inNetworkId )
{

}

void UReplicationManagerClientObj::ReadAndDoDestroyAction( InputMemoryBitStream& inInputStream, int inNetworkId )
{

}
