// Fill out your copyright notice in the Description page of Project Settings.

#include "ActionClient.h"
#include "ReplicationManagerClient.h"
#include "NetworkManager.h"
#include "ActionEntity.h"
#include "GameObjectRegistryUObj.h"



void ReplicationManagerClient::Read( InputMemoryBitStream& inInputStream )
{
	while (inInputStream.GetRemainingBitCount() >= 34)
	{
		//read the network id...
		int networkId;
		inInputStream.Read( networkId );

		//only need 2 bits for action...
		uint8_t action;
		inInputStream.Read( action, 2 );

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

	}
}

void ReplicationManagerClient::ReadAndDoCreateAction( InputMemoryBitStream& inInputStream, int inNetworkId )
{

	uint32_t fourCCName;
	inInputStream.Read( fourCCName );

	GameObjectPtr gameObject = NetworkManager::sInstance->GetGameObject( inNetworkId );
	if (!gameObject)
	{
		//UGameObjectRegistryUObj* gameObjRegistry =  NetworkManager::sInstance->GetGameObjectRegistryUObj();
		gameObject = NetworkManager::sInstance->GetGameObjectRegistryUObj()->CreateGameObject( fourCCName );
		if (!gameObject)
			return;
		gameObject->SetNetworkId( inNetworkId );
		NetworkManager::sInstance->AddToNetworkIdToGameObjectMap( gameObject );

		check( gameObject->GetClassId() == fourCCName );
	}

	gameObject->Read( inInputStream );
}

void ReplicationManagerClient::ReadAndDoUpdateAction( InputMemoryBitStream& inInputStream, int inNetworkId )
{
	GameObjectPtr gameObject = NetworkManager::sInstance->GetGameObject( inNetworkId );

	gameObject->Read( inInputStream );
}

void ReplicationManagerClient::ReadAndDoDestroyAction( InputMemoryBitStream& inInputStream, int inNetworkId )
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