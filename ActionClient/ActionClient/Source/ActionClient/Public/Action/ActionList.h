// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
#include <deque>
#include "ActionInputState.h"
#include "Action.h"


class ActionList
{
public:

	ActionList() :
		mLastActionTimestamp( -1.f )
	{}

	const	Action&	AddAction( const ActionInputState& inInputState, float inTimestamp );
	bool	AddActionIfNew( const Action& inAction );

	void	RemovedProcessedActions( float inLastActionProcessedOnServerTimestamp );

	float			GetLastActionTimestamp()	const { return mLastActionTimestamp; }

	const Action&		GetLatestAction()			const { return mActions.back(); }

	void			Clear() { mActions.clear(); }
	bool			HasActions()				const { return !mActions.empty(); }
	int				GetActionCount()			const { return mActions.size(); }

	//for for each, we have to match stl calling convention
	std::deque< Action >::const_iterator  begin()	const { return mActions.begin(); }
	std::deque< Action >::const_iterator	 end()		const { return mActions.end(); }

	const Action&		operator[]( size_t i )	const { return mActions[i]; }

private:

	float			mLastActionTimestamp;
	std::deque< Action >	mActions;




};