// Fill out your copyright notice in the Description page of Project Settings.

#include "ActionClient.h"
#include "RealTimeSrvEngine.h"
#include "NetworkMgr.h"
#include "InputManager.h"
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

	ip = TEXT( "127.0.0.1" );
	port = 44444;
	player_name = TEXT( "RealTimeSrvTestPlayerName" );

	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass( TEXT( "/Game/RealTimeSrv/Script/BP_RealTimeSrvPawn" ) );
	if ( PlayerPawnBPClass.Class != NULL )
	{
		DefaultCharacterClasses = PlayerPawnBPClass.Class;
	}
}

// Called when the game starts
void URealTimeSrvEngine::BeginPlay()
{
	Super::BeginPlay();

	// ...

	//int32 testFloatPrint = 88;
	//A_LOG_N( "testFloatPrint : ", testFloatPrint );
	//A_LOG_N( "( float )testFloatPrint : ", ( float )testFloatPrint );

	////A_SCREENMSG_F
	//A_SCREENMSG_F( "testFloatPrint : ", testFloatPrint );
	//A_SCREENMSG_F( "( float )testFloatPrint : ", ( float )testFloatPrint );

	RealTimeSrvWorld::StaticInit();
	InputManager::StaticInit();
	RealTimeSrvEntityFactory::StaticInit( GetWorld() );
	NetworkMgr::StaticInit( ip, port, player_name );


	RealTimeSrvEntityFactory::sInstance->SetDefaultCharacterClasses( DefaultCharacterClasses );
}





// Called every frame
void URealTimeSrvEngine::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
	RealTimeSrvTiming::sInstance.Update();

	InputManager::sInstance->Update();

	RealTimeSrvWorld::sInstance->Update();

	NetworkMgr::sInstance->ProcessIncomingPackets();

	NetworkMgr::sInstance->SendOutgoingPackets();
}