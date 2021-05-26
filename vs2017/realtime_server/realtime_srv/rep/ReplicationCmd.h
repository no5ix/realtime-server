#pragma once


namespace realtime_srv
{

enum ReplicationAction
{
	RA_Create,
	RA_Update,
	RA_Destroy,
	RA_RPC,
	RA_MAX
};


struct ReplicationCmd
{
public:

	ReplicationCmd() {}
	ReplicationCmd(uint32_t initialDirtyState)
		: repAction_(RA_Create), dirtyState_(initialDirtyState) {}

	void HandleCreateAckd() { if (repAction_ == RA_Create) { repAction_ = RA_Update; } }
	void AddDirtyState(uint32_t inState) { dirtyState_ |= inState; }
	void SetDestroy() { repAction_ = RA_Destroy; }

	bool HasDirtyState() const
	{ return (repAction_ == RA_Destroy) || (dirtyState_ != 0); }

	void SetAction(ReplicationAction inAction) { repAction_ = inAction; }
	ReplicationAction	GetAction()	const { return repAction_; }
	uint32_t GetDirtyState() const { return dirtyState_; }
	inline void ClearDirtyState(uint32_t inStateToClear);

private:

	ReplicationAction		repAction_;
	uint32_t				dirtyState_;
};

inline void	 ReplicationCmd::ClearDirtyState(uint32_t stateToClear)
{
	dirtyState_ &= ~stateToClear;

	if (repAction_ == RA_Destroy)
	{
		repAction_ = RA_Update;
	}
}

}