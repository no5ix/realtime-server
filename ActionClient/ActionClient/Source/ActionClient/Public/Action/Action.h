// Fill out your copyright notice in the Description page of Project Settings.

//#pragma once

//#include <memory>
//
//#include <vector>
//#include <unordered_map>
//#include <string>
//#include <list>
//#include <queue>
//#include <deque>
//#include <unordered_set>
//#include <cassert>
//
//using std::shared_ptr;
//using std::unique_ptr;
//using std::vector;
//using std::queue;
//using std::list;
//using std::deque;
//using std::unordered_map;
//using std::string;
//using std::unordered_set;
//
//class Move;
//class InputManager;
//class ActionInputState;
//
//#include "ActionMath.h"
//#include "MemoryBitStream.h"
//#include "ActionSocketUtil.h"
//
//#include "ActionInputState.h"
//
//#include "Move.h"
////#include "MoveList.h"
//
//#include "ActionTiming.h"
//#include "InputManager.h"
//
//#include "ActionHelper.h"
//
//#include "NetworkManager.h"


#pragma once
#include "ActionInputState.h"
#include "MemoryBitStream.h"

class Action{
public:

	Action() {}

	Action( const ActionInputState& inInputState, float inTimestamp, float inDeltaTime ) :
		mInputState( inInputState ),
		mTimestamp( inTimestamp ),
		mDeltaTime( inDeltaTime )
	{}


	const ActionInputState&	GetInputState()	const { return mInputState; }
	float				GetTimestamp()	const { return mTimestamp; }
	float				GetDeltaTime()	const { return mDeltaTime; }

	bool Write( OutputMemoryBitStream& inOutputStream ) const;
	bool Read( InputMemoryBitStream& inInputStream );

private:
	ActionInputState	mInputState;
	float		mTimestamp;
	float		mDeltaTime;

};


