// Fill out your copyright notice in the Description page of Project Settings.

#include "ShooterGame.h"
#include "ActionCharacter.h"
#include "InputManager.h"


// Sets default values
//AActionCharacter::AActionCharacter()
//{

AActionCharacter::AActionCharacter( const FObjectInitializer& ObjectInitializer )
	: Super( ObjectInitializer )
{
	Mesh1P = ObjectInitializer.CreateDefaultSubobject<USkeletalMeshComponent>( this, TEXT( "PawnMesh1P" ) );
	Mesh1P->SetupAttachment( GetCapsuleComponent() );
	Mesh1P->bOnlyOwnerSee = true;
	Mesh1P->bOwnerNoSee = false;
	Mesh1P->bCastDynamicShadow = false;
	Mesh1P->bReceivesDecals = false;
	Mesh1P->MeshComponentUpdateFlag = EMeshComponentUpdateFlag::OnlyTickPoseWhenRendered;
	Mesh1P->PrimaryComponentTick.TickGroup = TG_PrePhysics;
	Mesh1P->SetCollisionObjectType( ECC_Pawn );
	Mesh1P->SetCollisionEnabled( ECollisionEnabled::NoCollision );
	Mesh1P->SetCollisionResponseToAllChannels( ECR_Ignore );

	GetMesh()->bOnlyOwnerSee = false;
	GetMesh()->bOwnerNoSee = true;
	GetMesh()->bReceivesDecals = false;
	GetMesh()->SetCollisionObjectType( ECC_Pawn );
	GetMesh()->SetCollisionEnabled( ECollisionEnabled::QueryAndPhysics );
	GetMesh()->SetCollisionResponseToChannel( COLLISION_WEAPON, ECR_Block );
	GetMesh()->SetCollisionResponseToChannel( COLLISION_PROJECTILE, ECR_Block );
	GetMesh()->SetCollisionResponseToChannel( ECC_Visibility, ECR_Block );

	GetCapsuleComponent()->SetCollisionResponseToChannel( ECC_Camera, ECR_Ignore );
	GetCapsuleComponent()->SetCollisionResponseToChannel( COLLISION_PROJECTILE, ECR_Block );
	GetCapsuleComponent()->SetCollisionResponseToChannel( COLLISION_WEAPON, ECR_Ignore );


 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	BaseTurnRate = 45.f;
	BaseLookUpRate = 45.f;

}

// Called when the game starts or when spawned
void AActionCharacter::BeginPlay()
{
	Super::BeginPlay();
	
}


// Called every frame
void AActionCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void AActionCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	//Super::SetupPlayerInputComponent(PlayerInputComponent);

	check( PlayerInputComponent );
	PlayerInputComponent->BindAxis( "MoveForward", this, &AActionCharacter::MoveForward );
	PlayerInputComponent->BindAxis( "MoveRight", this, &AActionCharacter::MoveRight );    
	PlayerInputComponent->BindAxis( "MoveUp", this, &AActionCharacter::MoveUp );          
	//PlayerInputComponent->BindAxis( "Turn", this, &APawn::AddControllerYawInput );
	PlayerInputComponent->BindAxis( "Turn", this, &AActionCharacter::Turn );
	PlayerInputComponent->BindAxis( "TurnRate", this, &AActionCharacter::TurnAtRate );    
	//PlayerInputComponent->BindAxis( "LookUp", this, &APawn::AddControllerPitchInput );
	PlayerInputComponent->BindAxis( "LookUp", this, &AActionCharacter::LookUp );
	PlayerInputComponent->BindAxis( "LookUpRate", this, &AActionCharacter::LookUpAtRate );

	//PlayerInputComponent->BindAction( "Fire", IE_Pressed, this, &AActionCharacter::OnStartFire );
	//PlayerInputComponent->BindAction( "Fire", IE_Released, this, &AActionCharacter::OnStopFire );

	//PlayerInputComponent->BindAction( "Targeting", IE_Pressed, this, &AActionCharacter::OnStartTargeting );
	//PlayerInputComponent->BindAction( "Targeting", IE_Released, this, &AActionCharacter::OnStopTargeting );

	//PlayerInputComponent->BindAction( "NextWeapon", IE_Pressed, this, &AActionCharacter::OnNextWeapon );
	//PlayerInputComponent->BindAction( "PrevWeapon", IE_Pressed, this, &AActionCharacter::OnPrevWeapon );

	//PlayerInputComponent->BindAction( "Reload", IE_Pressed, this, &AActionCharacter::OnReload );

	PlayerInputComponent->BindAction( "Jump", IE_Pressed, this, &AActionCharacter::OnStartJump );
	PlayerInputComponent->BindAction( "Jump", IE_Released, this, &AActionCharacter::OnStopJump );

	//PlayerInputComponent->BindAction( "Run", IE_Pressed, this, &AActionCharacter::OnStartRunning );
	//PlayerInputComponent->BindAction( "RunToggle", IE_Pressed, this, &AActionCharacter::OnStartRunningToggle );
	//PlayerInputComponent->BindAction( "Run", IE_Released, this, &AActionCharacter::OnStopRunning );

	//GetWorld()->GetFirstPlayerController()->PlayerInput->GetKeysForAction(...);
}



void AActionCharacter::MoveForward( float Val )
{
	if (Controller && Val != 0.f)
	{
		// Limit pitch when walking or falling
		const bool bLimitRotation = ( GetCharacterMovement()->IsMovingOnGround() || GetCharacterMovement()->IsFalling() );
		const FRotator Rotation = bLimitRotation ? GetActorRotation() : Controller->GetControlRotation();
		const FVector Direction = FRotationMatrix( Rotation ).GetScaledAxis( EAxis::X );
		AddMovementInput( Direction, Val );


		InputManager::sInstance->HandleInput( InputManager::EIA_MoveForward, Val );
	}
}

void AActionCharacter::MoveRight( float Val )
{
	if (Val != 0.f)
	{
		const FQuat Rotation = GetActorQuat();
		const FVector Direction = FQuatRotationMatrix( Rotation ).GetScaledAxis( EAxis::Y );
		AddMovementInput( Direction, Val );

		InputManager::sInstance->HandleInput( InputManager::EIA_MoveRight, Val );
	}
}

void AActionCharacter::MoveUp( float Val )
{
	if (Val != 0.f)
	{
		// Not when walking or falling.
		if (GetCharacterMovement()->IsMovingOnGround() || GetCharacterMovement()->IsFalling())
		{
			return;
		}

		AddMovementInput( FVector::UpVector, Val );

		InputManager::sInstance->HandleInput( InputManager::EIA_MoveUp, Val );
	}
}

void AActionCharacter::Turn( float Val )
{
	APawn::AddControllerYawInput( Val );


	InputManager::sInstance->HandleInput( InputManager::EIA_Turn, Val );

}

void AActionCharacter::LookUp( float Val )
{
	AddControllerPitchInput( Val );

	InputManager::sInstance->HandleInput( InputManager::EIA_LookUp, Val );
}

void AActionCharacter::TurnAtRate( float Val )
{
	// calculate delta for this frame from the rate information
	AddControllerYawInput( Val * BaseTurnRate * GetWorld()->GetDeltaSeconds() );

	InputManager::sInstance->HandleInput( InputManager::EIA_TurnRate, Val );


}

void AActionCharacter::LookUpAtRate( float Val )
{
	// calculate delta for this frame from the rate information
	AddControllerPitchInput( Val * BaseLookUpRate * GetWorld()->GetDeltaSeconds() );


	InputManager::sInstance->HandleInput( InputManager::EIA_LookUpRate, Val );

}


void AActionCharacter::OnStartJump()
{
	//AShooterPlayerController* MyPC = Cast<AShooterPlayerController>( Controller );
	//if (MyPC && MyPC->IsGameInputAllowed())
	//{
	//	bPressedJump = true;
	//}
	bPressedJump = true;

	InputManager::sInstance->HandleInput( InputManager::EIA_OnStartJump, 1.0f );
}

void AActionCharacter::OnStopJump()
{
	bPressedJump = false;
	StopJumping();

	InputManager::sInstance->HandleInput( InputManager::EIA_OnStopJump, 0.0f );
}

bool AActionCharacter::IsFirstPerson() const
{
	return true;
	//return IsAlive() && Controller && Controller->IsLocalPlayerController();
}

void AActionCharacter::OnCameraUpdate( const FVector& CameraLocation, const FRotator& CameraRotation )
{
	USkeletalMeshComponent* DefMesh1P = Cast<USkeletalMeshComponent>( GetClass()->GetDefaultSubobjectByName( TEXT( "PawnMesh1P" ) ) );
	const FMatrix DefMeshLS = FRotationTranslationMatrix( DefMesh1P->RelativeRotation, DefMesh1P->RelativeLocation );
	const FMatrix LocalToWorld = ActorToWorld().ToMatrixWithScale();

	// Mesh rotating code expect uniform scale in LocalToWorld matrix

	const FRotator RotCameraPitch( CameraRotation.Pitch, 0.0f, 0.0f );
	const FRotator RotCameraYaw( 0.0f, CameraRotation.Yaw, 0.0f );

	const FMatrix LeveledCameraLS = FRotationTranslationMatrix( RotCameraYaw, CameraLocation ) * LocalToWorld.Inverse();
	const FMatrix PitchedCameraLS = FRotationMatrix( RotCameraPitch ) * LeveledCameraLS;
	const FMatrix MeshRelativeToCamera = DefMeshLS * LeveledCameraLS.Inverse();
	const FMatrix PitchedMesh = MeshRelativeToCamera * PitchedCameraLS;

	Mesh1P->SetRelativeLocationAndRotation( PitchedMesh.GetOrigin(), PitchedMesh.Rotator() );
}
