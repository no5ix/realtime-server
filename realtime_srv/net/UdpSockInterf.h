
namespace realtime_srv
{
class UdpSockInterf
{
public:

	~UdpSockInterf();



	static bool			StaticInit();
	static void			CleanUp();

	static void			ReportError( const char* inOperationDesc );
	static int			GetLastError();

	static shared_ptr< UdpSockInterf > CreateUDPSocket();

	int Bind( const SockAddrInterf& inToAddress );
	int SendTo( const void* inToSend, int inLength, const SockAddrInterf& inToAddress );
	int ReceiveFrom( void* inToReceive, int inMaxLength, SockAddrInterf& outFromAddress );
	int Connect( const SockAddrInterf& inAddress );

	int SetReUse();
	int32_t Send( const void* inData, size_t inLen );
	int32_t Recv( void* inData, size_t inLen );
	int SetNonBlockingMode( bool inShouldBeNonBlocking );
	SOCKET GetSocket() { return mSocket; }

private:
	UdpSockInterf( SOCKET inSocket ) : mSocket( inSocket ) {}
	SOCKET mSocket;

};

typedef shared_ptr< UdpSockInterf >	UDPSocketPtr;
}