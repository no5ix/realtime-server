// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
#include "MemoryBitStream.h"

/**
 * 
 */

class ReplicationManagerClient
{
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

private:

	void ReadAndDoCreateAction( InputMemoryBitStream& inInputStream, int inNetworkId );
	void ReadAndDoUpdateAction( InputMemoryBitStream& inInputStream, int inNetworkId );
	void ReadAndDoDestroyAction( InputMemoryBitStream& inInputStream, int inNetworkId );

};