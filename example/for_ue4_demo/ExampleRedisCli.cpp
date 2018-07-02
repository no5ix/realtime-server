

#include "ExampleRedisCli.h"

#ifdef HAS_REDIS


using namespace muduo;
using namespace muduo::net;
using namespace hiredis;

ExampleRedisCli::ExampleRedisCli()
{}

void ExampleRedisCli::Init( EventLoop *loop )
{
	loop_ = loop;
	InetAddress serverAddr( "127.0.0.1", 6379 );
	redisCli_.reset( new Hiredis( loop_, serverAddr ) );

	redisCli_->setConnectCallback( std::bind(
		&ExampleRedisCli::connectCallback, this, _1, _2 ) );
	redisCli_->setDisconnectCallback( std::bind(
		&ExampleRedisCli::disconnectCallback, this, _1, _2 ) );
	redisCli_->connect();
}

ExampleRedisCli::~ExampleRedisCli()
{
	redisCli_->disconnect();
}

void ExampleRedisCli::connectCallback( hiredis::Hiredis* c, int status )
{
	if ( status != REDIS_OK )
	{
		LOG_ERROR << "connectCallback Error:" << c->errstr();
	}
	else
	{
		LOG_INFO << "Connected...";
	}
}

void ExampleRedisCli::disconnectCallback( hiredis::Hiredis* c, int status )
{
	if ( status != REDIS_OK )
	{
		LOG_ERROR << "disconnectCallback Error:" << c->errstr();
	}
	else
	{
		LOG_INFO << "Disconnected...";
	}
}

std::string ExampleRedisCli::toString( long long value )
{
	char buf[32];
	snprintf( buf, sizeof buf, "%lld", value );
	return buf;
}

std::string ExampleRedisCli::redisReplyToString( const redisReply* reply )
{
	static const char* const types[] = { "",
		"REDIS_REPLY_STRING", "REDIS_REPLY_ARRAY",
		"REDIS_REPLY_INTEGER", "REDIS_REPLY_NIL",
		"REDIS_REPLY_STATUS", "REDIS_REPLY_ERROR" };
	std::string str;
	if ( !reply ) return str;

	str += types[reply->type] + std::string( "(" ) + toString( reply->type ) + ") ";

	str += "{ ";
	if ( reply->type == REDIS_REPLY_STRING ||
		reply->type == REDIS_REPLY_STATUS ||
		reply->type == REDIS_REPLY_ERROR )
	{
		str += '"' + std::string( reply->str, reply->len ) + '"';
	}
	else if ( reply->type == REDIS_REPLY_INTEGER )
	{
		str += toString( reply->integer );
	}
	else if ( reply->type == REDIS_REPLY_ARRAY )
	{
		str += toString( reply->elements ) + " ";
		for ( size_t i = 0; i < reply->elements; i++ )
		{
			str += " " + redisReplyToString( reply->element[i] );
		}
	}
	str += " }";

	return str;
}

void ExampleRedisCli::SaveNewPlayerCb( hiredis::Hiredis *c, redisReply *reply,
	int playerId, const std::string& playerName )
{
	LOG_INFO << "ZADD player " << "playerId = " << playerId << " "
		<< "playerName = " << playerName << " " << redisReplyToString( reply );
}

void ExampleRedisCli::SaveNewPlayer( int newPlayerId, const std::string& newPlayerName )
{
	// ZADD key score member
	int score = newPlayerId;
	const char *member =
		( newPlayerName + ":" + std::to_string( newPlayerId ) ).c_str();

	auto temp_func = [&, score, member]() {
		redisCli_->command( std::bind( &ExampleRedisCli::SaveNewPlayerCb, this,
			_1, _2, score, member ),
			"ZADD player %d %s", score, member );
	};

	loop_->runInLoop( temp_func );
}


#endif // HAS_REDIS
