// Fill out your copyright notice in the Description page of Project Settings.


#include "ReplicationMgr.h"
#include "NetworkMgr.h"
#include "RealTimeSrvEntity.h"
#include "RealTimeSrvHelper.h"
#include "RealTimeSrvEntityFactory.h"



void ReplicationMgr::Read( InputBitStream& inInputStream )
{
	//A_LOG_1( "ReplicationManagerClient::Read, start" );
	//int countForTest = 0;

	while (inInputStream.GetRemainingBitCount() >= 32)
	{
		//read the network id...
		int networkId;
		inInputStream.Read( networkId );

		//only need 2 bits for action...
		uint8_t action;
		inInputStream.Read( action, 2 );

		//A_LOG_1( "into while" );

		switch (action)
		{
		case RA_Create:
			ReadAndDoCreateAction( inInputStream, networkId );
			break;
		case RA_Update:
			ReadAndDoUpdateAction( inInputStream, networkId );
			break;
		case RA_Destroy:
			ReadAndDoDestroyAction( inInputStream, networkId );
			break;
		}

		//++countForTest;
	}

	//A_LOG_N( "ReplicationManagerClient::Read, end with countForTest = ", countForTest );
}

void ReplicationMgr::ReadAndDoCreateAction( InputBitStream& inInputStream, int inNetworkId )
{

	uint32_t fourCCName;
	inInputStream.Read( fourCCName );

	RealTimeSrvEntityPtr gameObject = NetworkMgr::sInstance->GetGameObject( inNetworkId );
	if (!gameObject)
	{
		gameObject = RealTimeSrvEntityFactory::sInstance->CreateGameObject( fourCCName );
	
		check( gameObject );
		gameObject->SetNetworkId( inNetworkId );
		NetworkMgr::sInstance->AddToNetworkIdToGameObjectMap( gameObject );

		check( gameObject->GetClassId() == fourCCName );
	}

	gameObject->Read( inInputStream );

	A_LOG_1( "= = == = = == = = = = == = = = = =" );
	A_LOG();
	A_LOG_1( "= = == = = == = = = = == = = = = =" );
	
}

void ReplicationMgr::ReadAndDoUpdateAction( InputBitStream& inInputStream, int inNetworkId )
{
	RealTimeSrvEntityPtr gameObject = NetworkMgr::sInstance->GetGameObject( inNetworkId );

	gameObject->Read( inInputStream );

	//A_LOG_1( "= = == = = == = = = = == = = = = =" );
	//A_LOG();
	//A_LOG_1( "= = == = = == = = = = == = = = = =" );
}

void ReplicationMgr::ReadAndDoDestroyAction( InputBitStream& inInputStream, int inNetworkId )
{
//	//if something was destroyed before the create went through, we'll never get it
//	//but we might get the destroy request, so be tolerant of being asked to destroy something that wasn't created
//	GameObjectPtr gameObject = NetworkManagerClient::sInstance->GetGameObject( inNetworkId );
//	if (gameObject)
//	{
//		gameObject->SetDoesWantToDie( true );
//		NetworkManagerClient::sInstance->RemoveFromNetworkIdToGameObjectMap( gameObject );
//	}
}