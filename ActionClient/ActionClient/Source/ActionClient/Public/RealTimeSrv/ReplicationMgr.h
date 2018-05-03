// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
#include "BitStream.h"

/**
 * 
 */

class ReplicationMgr
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
	void Read( InputBitStream& inInputStream );

private:

	void ReadAndDoCreateAction( InputBitStream&	inInputStream, int inNetworkId );
	void ReadAndDoUpdateAction( InputBitStream& inInputStream, int inNetworkId );
	void ReadAndDoDestroyAction( InputBitStream& inInputStream, int inNetworkId );

};