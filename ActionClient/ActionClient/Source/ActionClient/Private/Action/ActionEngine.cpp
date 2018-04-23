// Fill out your copyright notice in the Description page of Project Settings.

#include "ActionClient.h"
#include "ActionEngine.h"
#include "NetworkManager.h"
#include "InputManager.h"
#include "ActionTiming.h"

// Sets default values for this component's properties
UActionEngine::UActionEngine( const FObjectInitializer& ObjectInitializer ) : Super( ObjectInitializer )
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	//bWantsBeginPlay = true;
	PrimaryComponentTick.bCanEverTick = true;
	bWantsInitializeComponent = true;

	// ...

	ip = TEXT( "127.0.0.1" );
	port = 45000;
	player_name = TEXT( "ActionClientTestPlayerName" );

	InputManager::StaticInit();
}

// Called when the game starts
void UActionEngine::BeginPlay()
{
	Super::BeginPlay();

	// ...


	//GameObjectRegistry::sInstance->RegisterCreationFunction( 'RCAT', RoboCatClient::StaticCreate );
	//GameObjectRegistry::sInstance->RegisterCreationFunction( 'MOUS', MouseClient::StaticCreate );
	//GameObjectRegistry::sInstance->RegisterCreationFunction( 'YARN', YarnClient::StaticCreate );

	NetworkManager::StaticInit( ip, port, player_name );
	//NetworkManager::sInstance->GetReplicationManagerClient()->SetDefaultCharacterClasses( DefaultCharacterClasses );
	NetworkManager::sInstance->GetGameObjectRegistryUObj()->SetDefaultCharacterClasses( DefaultCharacterClasses );
}


// Called every frame
void UActionEngine::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
	ActionTiming::sInstance.Update();

	InputManager::sInstance->Update();

	//Engine::DoFrame();

	NetworkManager::sInstance->ProcessIncomingPackets();

	//RenderManager::sInstance->Render();

	NetworkManager::sInstance->SendOutgoingPackets();
}