
namespace realtime_srv
{
	class RealtimeSrvTiming
	{
	public:

		RealtimeSrvTiming();

		void Update();

		float GetDeltaTime() const { return mDeltaTime; }

		double GetGameTimeD() const;

		float GetCurrentGameTime() const
		{
			return static_cast< float >( GetGameTimeD() );
		}

		float GetFrameStartTime() const { return mFrameStartTimef; }


		static RealtimeSrvTiming sInstance;

	private:
		float		mDeltaTime;
		uint64_t	mDeltaTick;

		double		mLastFrameStartTime;
		float		mFrameStartTimef;
		double		mPerfCountDuration;

	};
}