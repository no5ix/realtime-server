#include "realtime_srv/RealtimeServer.h"
#include <INIReader.h>

using namespace realtime_srv;


RealtimeServer::RealtimeServer()
{
	ReadConfigFile();

	if ( srvConf_.daemonize && !RealtimeSrvHelper::Daemonize() )
		LOG( " Become Daemon Failed!! " );

	srand( static_cast< uint32_t >( time( nullptr ) ) );

	world_.reset( new World() ); assert( world_ );

	networkMgr_.reset( new NetworkMgr( srvConf_ ) ); assert( networkMgr_ );

	networkMgr_->SetWorldUpdateCallback( [&]() { world_->Update(); } );
	networkMgr_->SetWorldRegistryCallback( std::bind(
		&World::Registry, world_.get(), _1, _2 ) );
	networkMgr_->SetLetCliProxyGetWorldStateCallback( std::bind(
		&World::WhenClientProxyHere, world_.get(), _1 ) );

	world_->OnObjCreateOrDestoryCallback( std::bind(
		&NetworkMgr::OnObjCreateOrDestory, networkMgr_.get(), _1, _2 ) );
}


void RealtimeServer::ReadConfigFile()
{
	INIReader reader( "./config/rs_config.ini" );
	//	0 on success, 
	//	line number of first error on parse error,
	//	or -1 on file open error.
	assert( reader.ParseError() == 0 );

	srvConf_.daemonize =
		reader.GetBoolean( "RealtimeServer", "daemonize", true );

	srvConf_.is_unregist_obj_when_cli_disconn =
		reader.GetBoolean( "NetworkMgr", "is_unregist_obj_when_cli_disconn", false );
	srvConf_.action_count_per_tick =
		reader.GetInteger( "NetworkMgr", "action_count_per_tick", 2 );

	srvConf_.send_packet_interval =
		reader.GetReal( "PktHandler", "send_packet_interval", 0.033333 );
	srvConf_.client_disconnect_timeout =
		reader.GetReal( "PktHandler", "client_disconnect_timeout", 6.0 );
	srvConf_.fps =
		reader.GetInteger( "PktHandler", "fps", 30 );
	srvConf_.packet_dispatcher_thread_count =
		reader.GetInteger( "PktHandler", "packet_dispatcher_thread_count", 7 );
	srvConf_.max_packets_count_per_fetch =
		reader.GetInteger( "PktHandler", "max_packets_count_per_round", 10 );
	srvConf_.port =
		reader.GetInteger( "PktHandler", "port", 44444 );

	LOG( "srvConf_.daemonize = %d", ( srvConf_.daemonize ? 1 : 0 ) );
	LOG( "srvConf_.is_unregist_obj_when_cli_disconn = %d", ( srvConf_.is_unregist_obj_when_cli_disconn ? 1 : 0 ) );
	LOG( "srvConf_.action_count_per_tick = %d", srvConf_.action_count_per_tick );
	LOG( "srvConf_.send_packet_interval = %f", srvConf_.send_packet_interval );
	LOG( "srvConf_.fps = %d", srvConf_.fps );
	LOG( "srvConf_.client_disconnect_timeout = %f", srvConf_.client_disconnect_timeout );
	LOG( "srvConf_.packet_dispatcher_thread_count = %d", srvConf_.packet_dispatcher_thread_count );
	LOG( "srvConf_.max_packets_count_per_fetch = %d", srvConf_.max_packets_count_per_fetch );
	LOG( "srvConf_.port = %d", srvConf_.port );
}