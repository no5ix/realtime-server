

class UDPSocketInterface
{
public:

	~UDPSocketInterface();



	static bool			StaticInit();
	static void			CleanUp();

	static void			ReportError(const char* inOperationDesc);
	static int			GetLastError();

	static shared_ptr< UDPSocketInterface > CreateUDPSocket();

	int Bind(const SocketAddressInterface& inToAddress);
	int SendTo(const void* inToSend, int inLength, const SocketAddressInterface& inToAddress);
	int ReceiveFrom(void* inToReceive, int inMaxLength, SocketAddressInterface& outFromAddress);
	int Connect( const SocketAddressInterface& inAddress );

	int SetReUse();
	int32_t Send( const void* inData, size_t inLen );
	int32_t Recv( void* inData, size_t inLen );
	int SetNonBlockingMode( bool inShouldBeNonBlocking );
	SOCKET GetSocket() { return mSocket; }

private:
	UDPSocketInterface(SOCKET inSocket) : mSocket(inSocket) {}
	SOCKET mSocket;

};

typedef shared_ptr< UDPSocketInterface >	UDPSocketPtr;