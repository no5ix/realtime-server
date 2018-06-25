
namespace realtime_srv
{
	class UdpSockInterfc
	{
	public:

		~UdpSockInterfc();



		static bool			StaticInit();
		static void			CleanUp();

		static void			ReportError( const char* inOperationDesc );
		static int			GetLastError();

		static shared_ptr< UdpSockInterfc > CreateUDPSocket();

		int Bind( const SockAddrInterfc& inToAddress );
		int SendTo( const void* inToSend, int inLength, const SockAddrInterfc& inToAddress );
		int ReceiveFrom( void* inToReceive, int inMaxLength, SockAddrInterfc& outFromAddress );
		int Connect( const SockAddrInterfc& inAddress );

		int SetReUse();
		int32_t Send( const void* inData, size_t inLen );
		int32_t Recv( void* inData, size_t inLen );
		int SetNonBlockingMode( bool inShouldBeNonBlocking );
		SOCKET GetSocket() { return mSocket; }

	private:
		UdpSockInterfc( SOCKET inSocket ) : mSocket( inSocket ) {}
		SOCKET mSocket;

	};

	typedef shared_ptr< UdpSockInterfc >	UDPSocketPtr;
}