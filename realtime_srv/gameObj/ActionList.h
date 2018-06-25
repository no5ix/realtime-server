#pragma once

namespace realtime_srv
{
	class ActionList
	{
	public:

		typedef deque< Action >::const_iterator			const_iterator;
		typedef deque< Action >::const_reverse_iterator	const_reverse_iterator;

		ActionList() :
			mLastMoveTimestamp( -1.f )
		{}

		const	Action&	AddMove( const InputState& inInputState, float inTimestamp );
		bool	AddMoveIfNew( const Action& inMove );

		void	RemovedProcessedMoves( float inLastMoveProcessedOnServerTimestamp );

		float			GetLastMoveTimestamp()	const { return mLastMoveTimestamp; }

		const Action&		GetLatestMove()			const { return mMoves.back(); }

		void			Clear() { mMoves.clear(); }
		bool			HasMoves()				const { return !mMoves.empty(); }
		int				GetMoveCount()			const { return mMoves.size(); }

		//for for each, we have to match stl calling convention
		const_iterator	begin()					const { return mMoves.begin(); }
		const_iterator	end()					const { return mMoves.end(); }

		const Action&		operator[]( size_t i )	const { return mMoves[i]; }
	private:

		float			mLastMoveTimestamp;
		deque< Action >	mMoves;
	};
}
