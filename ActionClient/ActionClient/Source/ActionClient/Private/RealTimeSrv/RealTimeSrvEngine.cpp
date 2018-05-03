// Fill out your copyright notice in the Description page of Project Settings.

#include "ActionClient.h"
#include "RealTimeSrvEngine.h"
#include "NetworkMgr.h"
#include "InputMgr.h"
#include "RealTimeSrvTiming.h"
#include "RealTimeSrvHelper.h"
#include "RealTimeSrvWorld.h"
#include "RealTimeSrvPawn.h"
#include "RealTimeSrvEntityFactory.h"


// Sets default values for this component's properties
URealTimeSrvEngine::URealTimeSrvEngine( const FObjectInitializer& ObjectInitializer ) : Super( ObjectInitializer )
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	//bWantsBeginPlay = true;
	PrimaryComponentTick.bCanEverTick = true;
	bWantsInitializeComponent = true;

	// ...

	IP = TEXT( "127.0.0.1" );
	Port = 44444;
	Player_Name = TEXT( "RealTimeSrvTestPlayerName" );

	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass( TEXT( "/Game/RealTimeSrv/Script/BP_RealTimeSrvPawn" ) );
	if ( PlayerPawnBPClass.Class != NULL )
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}

// Called when the game starts
void URealTimeSrvEngine::BeginPlay()
{
	Super::BeginPlay();

	RealTimeSrvWorld::StaticInit();
	InputMgr::StaticInit();
	RealTimeSrvEntityFactory::StaticInit( GetWorld() );
	NetworkMgr::StaticInit( IP, Port, Player_Name );


	RealTimeSrvEntityFactory::sInstance->SetDefaultPawnClass( DefaultPawnClass );
}





// Called every frame
void URealTimeSrvEngine::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
	RealTimeSrvTiming::sInstance.Update();

	InputMgr::sInstance->Update();

	RealTimeSrvWorld::sInstance->Update();

	NetworkMgr::sInstance->ProcessIncomingPackets();

	NetworkMgr::sInstance->SendOutgoingPackets();
}