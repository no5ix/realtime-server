#pragma once
#include <deque>
#include "realtime_srv/game_obj/Action.h"


namespace realtime_srv
{

class Action;

class ActionList
{
public:

	typedef std::deque<Action>::const_iterator			const_iterator;
	typedef std::deque<Action>::const_reverse_iterator	const_reverse_iterator;

	ActionList() :
		mLastMoveTimestamp( -1.f )
	{}

	bool AddMoveIfNew( const Action& inAction );

	void	RemovedProcessedMoves( float inLastMoveProcessedOnServerTimestamp );

	float			GetLastMoveTimestamp()	const { return mLastMoveTimestamp; }

	const Action&		GetLatestMove()			const { return actionQ_.back(); }

	void			Clear() { actionQ_.clear(); }
	bool			HasMoves()				const { return !actionQ_.empty(); }
	int				GetMoveCount()			const { return actionQ_.size(); }

	const_iterator	begin()					const { return actionQ_.begin(); }
	const_iterator	end()					const { return actionQ_.end(); }

	const Action&		operator[]( size_t i )	const { return actionQ_[i]; }
private:

	float			mLastMoveTimestamp;
	std::deque<Action>	actionQ_;
};
}
