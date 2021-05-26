
#pragma once

#include "realtime_srv/common/RealtimeSrvShared.h"

#ifdef IS_LINUX


#include <muduo_contrib/hiredis/Hiredis.h>

#include <muduo/base/Logging.h>
#include <muduo/net/EventLoop.h>

#include <string>


class ExampleRedisCli
{
public:
	ExampleRedisCli();
	void Init(muduo::net::EventLoop *loop);
	~ExampleRedisCli();

	void connectCallback(hiredis::Hiredis* c, int status);
	void disconnectCallback(hiredis::Hiredis* c, int status);

	std::string toString(long long value);
	std::string redisReplyToString(const redisReply* reply);

	void SaveNewPlayer(int newPlayerId, const std::string&  newPlayerName);
	void SaveNewPlayerCb(hiredis::Hiredis *c, redisReply *reply,
		int playerId, const std::string& playerName);

private:
	std::unique_ptr<hiredis::Hiredis> redisCli_;
	muduo::net::EventLoop *loop_;
};

#endif // IS_LINUX