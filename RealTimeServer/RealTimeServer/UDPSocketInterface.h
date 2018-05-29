

class UDPSocketInterface
{
public:

	~UDPSocketInterface();

	static bool			StaticInit();
	static void			CleanUp();

	static void			ReportError(const char* inOperationDesc);
	static int			GetLastError();

	static shared_ptr< UDPSocketInterface > CreateUDPSocket( SOCKET s = 0 );

	int Bind(const SocketAddrInterface& inToAddress);
	int SendTo(const void* inToSend, int inLength, const SocketAddrInterface& inToAddress);
	int ReceiveFrom(void* inToReceive, int inMaxLength, SocketAddrInterface& outFromAddress);
	int SetNonBlockingMode( bool inShouldBeNonBlocking );

	int Connect( const SocketAddrInterface& inAddress );
	int SetReUse();
	int32_t Send( const void* inData, size_t inLen );
	int32_t Recv( void* inData, size_t inLen );
	SOCKET GetSocket() const { return mSocket; }

private:
	UDPSocketInterface(SOCKET inSocket) : mSocket(inSocket) {}
	SOCKET mSocket;

};

typedef shared_ptr< UDPSocketInterface >	UDPSocketPtr;