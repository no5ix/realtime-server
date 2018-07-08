#include "Hiredis.h"

#include <muduo/base/Logging.h>
#include <muduo/net/EventLoop.h>

#include <string>

using namespace muduo;
using namespace muduo::net;

string toString( long long value )
{
	char buf[32];
	snprintf( buf, sizeof buf, "%lld", value );
	return buf;
}

string redisReplyToString( const redisReply* reply )
{
	static const char* const types[] = { "",
		"REDIS_REPLY_STRING", "REDIS_REPLY_ARRAY",
		"REDIS_REPLY_INTEGER", "REDIS_REPLY_NIL",
		"REDIS_REPLY_STATUS", "REDIS_REPLY_ERROR" };
	string str;
	if ( !reply ) return str;

	str += types[reply->type] + string( "(" ) + toString( reply->type ) + ") ";

	str += "{ ";
	if ( reply->type == REDIS_REPLY_STRING ||
		reply->type == REDIS_REPLY_STATUS ||
		reply->type == REDIS_REPLY_ERROR )
	{
		str += '"' + string( reply->str, reply->len ) + '"';
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

void connectCallback( hiredis::Hiredis* c, int status )
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

void disconnectCallback( hiredis::Hiredis* c, int status )
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

void timeCallback( hiredis::Hiredis* c, redisReply* reply )
{
	LOG_INFO << "time " << redisReplyToString( reply );
}

void echoCallback( hiredis::Hiredis* c, redisReply* reply, string* echo )
{
	LOG_INFO << *echo << " " << redisReplyToString( reply );
	//c->disconnect();
}

void dbsizeCallback( hiredis::Hiredis* c, redisReply* reply )
{
	LOG_INFO << "dbsize " << redisReplyToString( reply );
}

void selectCallback( hiredis::Hiredis* c, redisReply* reply, uint16_t* index )
{
	LOG_INFO << "select " << *index << " " << redisReplyToString( reply );
}

void authCallback( hiredis::Hiredis* c, redisReply* reply, string* password )
{
	LOG_INFO << "auth " << *password << " " << redisReplyToString( reply );
}

void echo( hiredis::Hiredis* c, string* s )
{
	c->command( std::bind( echoCallback, _1, _2, s ), "echo %s", s->c_str() );
}

//void testTime( hiredis::Hiredis* c, string* s ))
//{
//c->command( std::bind( timeCallback, _1, _2 ), "time", s->c_str() );
//
//}


void SETCallback( hiredis::Hiredis *c, redisReply *reply, string *key )
{
	LOG_INFO << "SET " << *key << " " << redisReplyToString( reply );
}

void set( hiredis::Hiredis *c, string *key )
{
	c->command( std::bind( SETCallback, _1, _2, key ), "SET %s 0", key->c_str() );
}

void GETCallback( hiredis::Hiredis *c, redisReply *reply, string *key )
{
	LOG_INFO << "GET " << *key << " " << redisReplyToString( reply );
}

void get( hiredis::Hiredis *c, string *key )
{
	c->command( std::bind( GETCallback, _1, _2, key ), "GET %s", key->c_str() );
}

void INCRCallback( hiredis::Hiredis *c, redisReply *reply, string *key )
{
	LOG_INFO << "INCR " << *key << " " << redisReplyToString( reply );
}

void incr( hiredis::Hiredis *c, string *key )
{
	c->command( std::bind( INCRCallback, _1, _2, key ), "INCR %s", key->c_str() );
}

int main( int argc, char** argv )
{
	Logger::setLogLevel( Logger::DEBUG );

	EventLoop loop;

	InetAddress serverAddr( "127.0.0.1", 6379 );
	hiredis::Hiredis hiredis( &loop, serverAddr );

	hiredis.setConnectCallback( connectCallback );
	hiredis.setDisconnectCallback( disconnectCallback );
	hiredis.connect();

	//hiredis.ping();
	//loop.runEvery( 1.0, std::bind( &hiredis::Hiredis::ping, &hiredis ) );
	//loop.runInLoop( std::bind( &hiredis::Hiredis::ping, &hiredis ) );

	//hiredis.command( timeCallback, "time" );

	string hi = "hi";
	//hiredis.command( std::bind( echoCallback, _1, _2, &hi ), "echo %s", hi.c_str() );
	//loop.runEvery( 2.0, std::bind( echo, &hiredis, &hi ) );
	//loop.runEvery( 2.0, std::bind( echo, &hiredis, &hi ) );
	loop.runAfter( 3.0, std::bind( echo, &hiredis, &hi ) );
	//loop.runAfter( 6.0, std::bind( echo, &hiredis, &hi ) );

	string key_hiredis( "key_hiredis" );
	loop.runEvery( 2.0, std::bind( incr, &hiredis, &key_hiredis ) );

	// hiredis.command(std::bind(GETCallback, _1, _2, &key_hiredis),
	//                "GET %s", key_hiredis.c_str());
	loop.runEvery( 2.0, std::bind( get, &hiredis, &key_hiredis ) );

	hiredis.command( dbsizeCallback, "dbsize" );

	uint16_t index = 8;
	hiredis.command( std::bind( selectCallback, _1, _2, &index ), "select %d", index );

	string password = "password";
	hiredis.command( std::bind( authCallback, _1, _2, &password ), "auth %s", password.c_str() );

	loop.loop();

	loop.runAfter( 6.0, std::bind( echo, &hiredis, &hi ) );
	//loop.runInLoop( std::bind( &hiredis::Hiredis::ping, &hiredis ) );

	return 0;
}
