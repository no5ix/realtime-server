//// Fill out your copyright notice in the Description page of Project Settings.
//
//#include "ShooterGame.h"
//#include "RamaUDPSender.h"
//
//
//// Sets default values
//ARamaUDPSender::ARamaUDPSender()
//{
// 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
//	PrimaryActorTick.bCanEverTick = true;
//
//}
//
//// Called when the game starts or when spawned
//void ARamaUDPSender::BeginPlay()
//{
//	Super::BeginPlay();
//	
//}
//
//// Called every frame
//void ARamaUDPSender::Tick(float DeltaTime)
//{
//	Super::Tick(DeltaTime);
//
//}
//


/*

RamaUDPSender

by Rama
*/
#include "ShooterGame.h"
#include "RamaUDPSender.h"
#include "MemoryBitStream.h"

ARamaUDPSender::ARamaUDPSender(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{	
	SenderSocket = NULL;

	ShowOnScreenDebugMessages = true;
}

void ARamaUDPSender::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
	//~~~~~~~~~~~~~~~~

	if(SenderSocket)
	{
		SenderSocket->Close();
		ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->DestroySocket(SenderSocket);
	}
}

bool ARamaUDPSender::StartUDPSender(
	const FString& YourChosenSocketName,
	const FString& TheIP, 
	const int32 ThePort,
	bool UDP
){	
	//Create Remote Address.
	RemoteAddr = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->CreateInternetAddr();

	bool bIsValid;
	RemoteAddr->SetIp(*TheIP, bIsValid);
	RemoteAddr->SetPort(ThePort);

	if(!bIsValid)
	{
		ScreenMsg("Rama UDP Sender>> IP address was not valid!", TheIP);
		return false;
	}

	SenderSocket = FUdpSocketBuilder(*YourChosenSocketName)
		.AsReusable()
		.WithBroadcast()
		;


	//check(SenderSocket->GetSocketType() == SOCKTYPE_Datagram);

	//Set Send Buffer Size
	int32 SendSize = 2*1024*1024;
	SenderSocket->SetSendBufferSize(SendSize,SendSize);
	SenderSocket->SetReceiveBufferSize(SendSize, SendSize);

	UE_LOG(LogTemp,Log,TEXT("\n\n\n~~~~~~~~~~~~~~~~~~~~~~~~~~~~~"));
	UE_LOG(LogTemp,Log,TEXT("Rama ****UDP**** Sender Initialized Successfully!!!"));
	UE_LOG(LogTemp,Log,TEXT("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n\n\n"));

	return true;
}

bool ARamaUDPSender::RamaUDPSender_SendString(FString ToSend)
{
	if(!SenderSocket) 
	{
		ScreenMsg("No sender socket");
		return false;
	}
	//~~~~~~~~~~~~~~~~

	//int32 BytesSent = 0;

	//FAnyCustomData NewData;
	//NewData.Scale = FMath::FRandRange(0,1000);
	//NewData.Count = FMath::RandRange(0,100);
	//NewData.Color = FLinearColor(FMath::FRandRange(0,1),FMath::FRandRange(0,1),FMath::FRandRange(0,1),1);

	//FArrayWriter Writer;

	//Writer << NewData; //Serializing our custom data, thank you UE4!

	//SenderSocket->SendTo(Writer.GetData(),Writer.Num(),BytesSent,*RemoteAddr);

	//if(BytesSent <= 0)
	//{
	//	const FString Str = "Socket is valid but the receiver received 0 bytes, make sure it is listening properly!";
	//	UE_LOG(LogTemp,Error,TEXT("%s"),*Str);
	//	ScreenMsg(Str);
	//	return false;
	//}

	//ScreenMsg("UDP~ Send Succcess! Bytes Sent = ",BytesSent );

	//return true;


	//~~~~~~~~~~~~~~~~  
	//发送消息  
	//int32 BytesSent = 0;

	//FString serialized = ToSend;
	//TCHAR *serializedChar = serialized.GetCharArray().GetData();
	//int32 size = FCString::Strlen( serializedChar );
	//int32 sent = 0;
	////SenderSocket->SendTo(Writer.GetData(), Writer.Num(), BytesSent, *RemoteAddr);  
	//SenderSocket->SendTo( ( uint8* )TCHAR_TO_UTF8( serializedChar ), size, BytesSent, *RemoteAddr );//发送给远端地址  




	//~~~~~~~~~~~~~~~~  
	//发送消息  
	int32 BytesSent = 0;
	OutputMemoryBitStream helloPacket;
	uint32_t kHelloCC = 'HELO';
	std::string mName = "lj";
	helloPacket.Write( kHelloCC );
	helloPacket.Write( mName );
	//int sentByteCount = mSocket->SendTo( inOutputStream.GetBufferPtr(), inOutputStream.GetByteLength(), inFromAddress );
	//if (sentByteCount > 0)
	//{
	//	mBytesSentThisFrame += sentByteCount;
	//}
	SenderSocket->SendTo( ( uint8* )helloPacket.GetBufferPtr(), helloPacket.GetByteLength(), BytesSent, *RemoteAddr );//发送给远端地址  

	if (BytesSent <= 0)
	{
		const FString Str = "Socket is valid but the receiver received 0 bytes, make sure it is listening properly!";
		UE_LOG( LogTemp, Error, TEXT( "%s" ), *Str );
		ScreenMsg( Str );
		return false;
	}

	ScreenMsg( "UDP Send Succcess! INFO Sent = ", ToSend );
	UE_LOG( LogTemp, Log, TEXT( "\n\n\n~~~~~~~~~~~~~~~~~~~~~~~~~~~~~" ) );
	UE_LOG( LogTemp, Log, TEXT( "****UDP**** Send Hello Packet Successfully!!!" ) );
	UE_LOG( LogTemp, Log, TEXT( "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n\n\n" ) );

	return true;
}



void ARamaUDPSender::DataRecv( FString& str, bool& success )              //接收消息处理  
{
	if (!SenderSocket)
	{
		ScreenMsg( "No sender socket" );
		success = false;
		return;
	}
	TSharedRef<FInternetAddr> targetAddr = ISocketSubsystem::Get( PLATFORM_SOCKETSUBSYSTEM )->CreateInternetAddr();
	//TArray<uint8> ReceivedData;//定义一个接收器  
	uint32 Size;


	static bool isFirstFlagForDebug = false;
	if (!isFirstFlagForDebug)
	{

		UE_LOG( LogTemp, Log, TEXT( "\n\n\n~~~~~~~~~~~~~~~~~~~~~~~~~~~~~" ) );
		UE_LOG( LogTemp, Log, TEXT( "****UDP**** DataRecv come in!!!" ) );
		UE_LOG( LogTemp, Log, TEXT( "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n\n\n" ) );

		isFirstFlagForDebug = true;
	}

	while (SenderSocket->HasPendingData( Size ))
	{

		//      str = "";  
		//      uint8 *Recv = new uint8[Size];  
		//      int32 BytesRead = 0;  


		//      ReceivedData.SetNumUninitialized(FMath::Min(Size, 65507u));  
		//      SenderSocket->RecvFrom(ReceivedData.GetData(), ReceivedData.Num(), BytesRead, *targetAddr);//创建远程接收地址  
		//      char ansiiData[1024];  
		//      memcpy(ansiiData, ReceivedData.GetData(), BytesRead);//拷贝数据到接收器  
		//      ansiiData[BytesRead] = 0;                            //判断数据结束  
		//      FString debugData = ANSI_TO_TCHAR(ansiiData);         //字符串转换  
		//      str = debugData;  
		//      // memset(ansiiData,0,1024);//清空   
		//SenderSocket->RecvFrom( ReceivedData.GetData(), ReceivedData.Num(), BytesRead, *targetAddr );//创建远程接收地址  


		char ReceivedDataPacketMem[1500];
		int32 packetSize = sizeof( ReceivedDataPacketMem );
		InputMemoryBitStream inputStream( ReceivedDataPacketMem, packetSize * 8 );
		int32 readByteCount = 0;

		SenderSocket->RecvFrom( ( uint8* )ReceivedDataPacketMem, packetSize, readByteCount, *targetAddr );//创建远程接收地址  

		inputStream.ResetToCapacity( readByteCount );
		const uint32_t	kHelloCC = 'HELO';
		const uint32_t	kWelcomeCC = 'WLCM';
		uint32_t	packetType;
		inputStream.Read( packetType );

		switch (packetType)
		{
		case kWelcomeCC:
			//HandleWelcomePacket( inputStream );
			break;
		default:
			UE_LOG( LogTemp, Log, TEXT( "\n\n\n~~~~~~~~~~~~~~~~~~~~~~~~~~~~~" ) );
			UE_LOG( LogTemp, Log, TEXT( "****UDP**** DataRecv wrong packet type!!!" ) );
			UE_LOG( LogTemp, Log, TEXT( "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n\n\n" ) );
			success = false;
			break;
		}

		//if we got a player id, we've been welcomed!
		int playerId = 0;
		inputStream.Read( playerId );

		ScreenMsg( "welcome on client as playerid = ", playerId );

		UE_LOG( LogTemp, Log, TEXT( "\n\n\n~~~~~~~~~~~~~~~~~~~~~~~~~~~~~" ) );
		UE_LOG( LogTemp, Log, TEXT( "****UDP**** DataRecv Successfully!!!" ) );
		UE_LOG( LogTemp, Log, TEXT( "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n\n\n" ) );

	}
	success = true;
}