#ifndef REALTIME_SRV_COMMON_NONCOPYABLE_H
#define REALTIME_SRV_COMMON_NONCOPYABLE_H

namespace realtime_srv
{

class noncopyable
{
protected:
	noncopyable() = default;
	~noncopyable() = default;

private:
	noncopyable( const noncopyable& ) = delete;
	void operator=( const noncopyable& ) = delete;
};

}

#endif  // REALTIME_SRV_COMMON_NONCOPYABLE_H
