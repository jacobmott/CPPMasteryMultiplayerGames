// Fill out your copyright notice in the Description page of Project Settings.


#include "PawnSpaceShip.h"
#include "Components/BoxComponent.h"
#include "GameFramework/PawnMovementComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/InputComponent.h"
#include "Components/StaticMeshComponent.h"
#include "C_MyCharacter.h"
#include "Components/CapsuleComponent.h"

#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerController.h"// Sets default values

APawnSpaceShip::APawnSpaceShip()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
  //Since the collision mesh is dealing with overlap events when a character/pawn gets close to the space ship
  //we turn it off for the mesh compoenent of this actor... the mesh component only really needs block physics
	bGenerateOverlapEventsDuringLevelStreaming = false;


	bWantToYaw = false;
	bMoveUp = false;
	bMoveForward = false;
	bFixRotation = false;
	bCameraRotateX = false;
	bCameraRotateY = false;
	CameraXDirection = 0.0f;
	CameraYDirection = 0.0f;
	MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComp"));
	MeshComp->SetSimulatePhysics(true);
	MeshComp->SetCollisionProfileName(UCollisionProfile::CustomCollisionProfileName);
	MeshComp->SetCollisionObjectType(ECollisionChannel::ECC_PhysicsBody);
	MeshComp->SetCollisionResponseToChannel(ECC_Visibility, ECollisionResponse::ECR_Ignore);
  //Collision section in blueprints
  MeshComp->CanCharacterStepUpOn = ECB_Yes;
	RootComponent = MeshComp;
	MeshComp->BodyInstance.bLockYRotation = true;
	MeshComp->BodyInstance.bLockXRotation = true;
	MeshComp->BodyInstance.SetUseCCD(true);
	MeshComp->SetUseCCD(true);

  //Physics section in blueprints
  MeshComp->SetSimulatePhysics(true);
  MeshComp->SetMassOverrideInKg(NAME_None, 100000.0f);
  MeshComp->bReplicatePhysicsToAutonomousProxy = true;
  MeshComp->bApplyImpulseOnDamage = true;
  MeshComp->SetEnableGravity(false);
  MeshComp->SetLinearDamping(1.0f);
  MeshComp->SetAngularDamping(1.0f);
	//Since the collision mesh is dealing with overlap events when a character/pawn gets close to the space ship
	//we turn it off for the mesh compoenent of this actor... the mesh component only really needs block physics
	MeshComp->SetGenerateOverlapEvents(false);

	CollisionMesh = CreateDefaultSubobject<UBoxComponent>(FName("Area Decision Mesh"));
	CollisionMesh->SetBoxExtent(FVector(500,500,100));
	//CollisionMesh->SetSimulatePhysics(true);
	CollisionMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	CollisionMesh->SetGenerateOverlapEvents(true);
	CollisionMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	//Collision Response set in C++ but not visible in Bluep
	//https://forums.unrealengine.com/t/collision-response-set-in-c-but-not-visible-in-blueprint/136334/2
	CollisionMesh->SetCollisionResponseToChannel(ECC_Pawn, ECollisionResponse::ECR_Overlap);
	//In order to recieve hit events, you have to set the channel to block, not everlap
	CollisionMesh->SetCollisionResponseToChannel(ECC_WorldStatic, ECollisionResponse::ECR_Block);
	CollisionMesh->CanCharacterStepUpOn = ECB_No;
	// This is equivalent to Simulate hit events in the pawns/collision component in the unreal editor details pane
	//CollisionMesh->SetNotifyRigidBodyCollision(true);
	CollisionMesh->BodyInstance.bNotifyRigidBodyCollision = true;
	CollisionMesh->IgnoreActorWhenMoving(this, true);
	CollisionMesh->SetEnableGravity(false);
	CollisionMesh->SetupAttachment(RootComponent);

	SpringArmComp = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArmComp"));
	SpringArmComp->bUsePawnControlRotation = false;
	SpringArmComp->SetupAttachment(RootComponent);
	SpringArmComp->bInheritPitch = false;
	SpringArmComp->bInheritYaw = false;
	SpringArmComp->bInheritRoll = false;

	CameraComp = CreateDefaultSubobject<UCameraComponent>(TEXT("CameraComp"));
	CameraComp->SetupAttachment(SpringArmComp);
	CameraComp->bUsePawnControlRotation = false;

	ZoomedFOV = 65.0f;

	isPossesed = false;

	bPilotExited = false;




}

// Called when the game starts or when spawned
void APawnSpaceShip::BeginPlay()
{
	Super::BeginPlay();
	DefaultFOV = CameraComp->FieldOfView;
	InitalRotation = GetActorRotation();
  CollisionMesh->OnComponentBeginOverlap.AddDynamic(this, &APawnSpaceShip::OnBoxOverlapBegin);
  CollisionMesh->OnComponentEndOverlap.AddDynamic(this, &APawnSpaceShip::OnBoxOverlapEnd);

	CollisionMesh->OnComponentHit.AddDynamic(this, &APawnSpaceShip::OnComponentHitCollisionMesh);
}

// Called every frame
void APawnSpaceShip::Tick(float DeltaTime)
{

	Super::Tick(DeltaTime);
	
	float TargetFOV = WantsToZoom ? ZoomedFOV : DefaultFOV;

	if (bCameraRotateX || bCameraRotateY) {
		FRotator NewRotation = FRotator(SpringArmComp->GetRelativeRotation().Pitch + CameraXDirection, SpringArmComp->GetRelativeRotation().Yaw + CameraYDirection, SpringArmComp->GetRelativeRotation().Roll);
		SpringArmComp->SetRelativeRotation(NewRotation);
		CameraXDirection = 0.0f;
		CameraYDirection = 0.0f;
		bCameraRotateX = false;
		bCameraRotateY = false;
	}

	float NewFOV = FMath::FInterpTo(CameraComp->FieldOfView, TargetFOV, DeltaTime, ZoomedInterpSpeed);
	CameraComp->SetFieldOfView(NewFOV);

	if(bMoveUp) {
		const FVector NewLocation = GetActorLocation() + (GetActorUpVector() * DeltaTime * UpDirection * MovementSpeed);
		FHitResult* Hit = new FHitResult();
		SetActorLocation(NewLocation, true, Hit, ETeleportType::None);
		HandleCollision(Hit);
	}
	if (bMoveForward) {
		const FVector NewLocation = GetActorLocation() + (GetActorForwardVector() * DeltaTime * ForwardDirection * MovementSpeed);
		FHitResult* Hit = new FHitResult();
		SetActorLocation(NewLocation, true, Hit, ETeleportType::None);
		HandleCollision(Hit);
	}

	if (bWantToYaw) {
		auto newYaw = Yaw * DeltaTime * RotationSpeed;
		FHitResult* Hit = new FHitResult();
		AddActorLocalRotation(FRotator(0, newYaw, 0), true, Hit, ETeleportType::None);
		HandleCollision(Hit);
	}
	FHitResult* Hit = new FHitResult();
	HandleCollision(Hit);



}


// Called to bind functionality to input
void APawnSpaceShip::HandleCollision(FHitResult* Hit)
{

	if (Hit->Actor != nullptr && Hit->Actor == CurrentPilot) {
		return;
	}
	if (Hit->IsValidBlockingHit() || bFixRotation) {
		bFixRotation = false;
		//SetActorRotation(InitalRotation);
		// In your some onTick or some such that is called each frame:
		float MyMaximumSpeedValue = 0.0f;
		FVector currentVelocity = MeshComp->GetPhysicsLinearVelocity();
		FVector clampedVelocity = currentVelocity.GetClampedToMaxSize(MyMaximumSpeedValue);
		MeshComp->SetPhysicsLinearVelocity(clampedVelocity);
		MeshComp->SetPhysicsAngularVelocity(clampedVelocity);
		// In your some onTick or some such that is called each frame:
		MyMaximumSpeedValue = 10000.0f;
		currentVelocity = MeshComp->GetPhysicsLinearVelocity();
		clampedVelocity = currentVelocity.GetClampedToMaxSize(MyMaximumSpeedValue);
		MeshComp->SetPhysicsLinearVelocity(clampedVelocity);
		MeshComp->SetPhysicsAngularVelocity(clampedVelocity);
	}
}

// Called to bind functionality to input
void APawnSpaceShip::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	PlayerInputComponent->BindAxis("UpVectorMove", this, &APawnSpaceShip::UpVectorMove);

	PlayerInputComponent->BindAxis("YawMoveRotate", this, &APawnSpaceShip::YawMoveRotate);
	PlayerInputComponent->BindAxis("MoveForward", this, &APawnSpaceShip::MoveForward);

	PlayerInputComponent->BindAction("Zoom", IE_Pressed, this, &APawnSpaceShip::BeginZoom);
	PlayerInputComponent->BindAction("Zoom", IE_Released, this, &APawnSpaceShip::EndZoom);

	PlayerInputComponent->BindAction("FixRotation", IE_Pressed, this, &APawnSpaceShip::FixRotation);

	PlayerInputComponent->BindAction("CameraZoomOut", IE_Pressed, this, &APawnSpaceShip::CameraZoomOut);
	PlayerInputComponent->BindAction("CameraZoomIn", IE_Pressed, this, &APawnSpaceShip::CameraZoomIn);


	PlayerInputComponent->BindAxis("CameraRotateX", this, &APawnSpaceShip::CameraRotateX);
	PlayerInputComponent->BindAxis("CameraRotateY", this, &APawnSpaceShip::CameraRotateY);

	PlayerInputComponent->BindAction("ExitVehicle", IE_Pressed, this, &APawnSpaceShip::ExitVehicle);

}

void APawnSpaceShip::ExitVehicle() {
  // UE_LOG(LogTemp, Log, TEXT("APawnSpaceShip OnOverlapEnd called"));
  if (GEngine) {
    GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Yellow, TEXT("APawnSpaceShip ExitVehicle called"));
  }

  if (!CurrentPilot) {
    return;
  }
	CurrentPilot->ExitVehicle();
	CurrentPilot = nullptr;
	bPilotExited = true;
	MeshComp->SetEnableGravity(true);

}

void APawnSpaceShip::YawMoveRotate(float Value) {
	bWantToYaw = false;
	if (Value > 0) {
		Yaw = 1;
		bWantToYaw = true;
		//UE_LOG(LogTemp, Log, TEXT("YawMoveRotate: %s)"), *FString::SanitizeFloat(Value), true);
	}
	if (Value < 0) {
		Yaw = -1;
		bWantToYaw = true;
		//UE_LOG(LogTemp, Log, TEXT("YawMoveRotate: %s)"), *FString::SanitizeFloat(Value), true);
	}



}

void APawnSpaceShip::UpVectorMove(float Value) {
	bMoveUp = false;
	if (Value > 0) {
		UpDirection = 1;
		bMoveUp = true;
	}
	if (Value < 0) {
		UpDirection = -1;
		bMoveUp = true;
	}
  //UE_LOG(LogTemp, Log, TEXT("UpVectorMove: %s)"), *FString::SanitizeFloat(Value), true);

}

void APawnSpaceShip::MoveForward(float Value) {
	bMoveForward = false;
	if (Value > 0 ) {
		ForwardDirection = 1;
		bMoveForward = true;
	}
	if ( Value < 0) {
		ForwardDirection = -1;
		bMoveForward = true;
	}
	//UE_LOG(LogTemp, Log, TEXT("MoveForward: %s)"), *FString::SanitizeFloat(Value), true);
}

void APawnSpaceShip::FixRotation()
{
	bFixRotation = true;
}

void APawnSpaceShip::BeginZoom()
{
	WantsToZoom = true;
}

void APawnSpaceShip::EndZoom()
{
	WantsToZoom = false;
}


void APawnSpaceShip::CameraRotateX(float Value) {
	bCameraRotateX = false;
	if (Value > 0 || Value < 0) {
		CameraXDirection = Value;
		bCameraRotateX = true;
	}
	//UE_LOG(LogTemp, Log, TEXT("CameraRotateX: %s)"), *FString::SanitizeFloat(Value), true);
}

void APawnSpaceShip::CameraRotateY(float Value) {
	bCameraRotateY = false;
	if (Value > 0 || Value < 0) {
		CameraYDirection = Value;
		bCameraRotateY = true;
	}
	//UE_LOG(LogTemp, Log, TEXT("CameraRotateY: %s)"), *FString::SanitizeFloat(Value), true);
}


void APawnSpaceShip::CameraZoomOut()
{
	SpringArmComp->TargetArmLength += 200;
}

void APawnSpaceShip::CameraZoomIn()
{
	SpringArmComp->TargetArmLength -= 200;
}


//void APawnSpaceShip::OnPossess(APawn* InPawn) {
//	Super::OnPossess(InPawn);
//}

void APawnSpaceShip::OnBoxOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{

	if (CurrentPilot) {
		return;
	}

  //UE_LOG(LogTemp, Log, TEXT("APawnSpaceShip OnOverlapBegin called"));
  if (GEngine) {
    GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Yellow, TEXT("APawnSpaceShip OnOverlapBegin called"));
  }

  //if (isPossesed) {
  //  return;
  //}

  APawn* Pawn = Cast<APawn>(OtherActor);
  if (Pawn == this) {
    return;
  }

	AC_MyCharacter* MyCharacter = Cast<AC_MyCharacter>(OtherActor);
	CurrentPilot = MyCharacter;
  //isPossesed = true;
	MyCharacter->SetVehicleInRange(this);

	//AController* PawnController = Pawn->GetController();
	//PawnController->UnPossess();
	//PawnController->Possess(this);

	//CurrentPilot->GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
  //TriggerCapsule->SetCollisionEnabled(ECollisionEnabled::NoCollision);
  //MyCharacter->GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
  //APlayerController* ShipController = Cast<APlayerController>(GetController());
  //ShipController->Possess(MyCharacter);
  //UE_LOG(LogTemp, Warning, TEXT("We Began"));
  //CurrentPilot = MyCharacter;

}



//void APawnSpaceShip::Enter(AC_MyCharacter* Pilot)
//{
//
//  Pilot->AttachToComponent(MeshComp, FAttachmentTransformRules::SnapToTargetNotIncludingScale, "PilotSeat");
//  //APlayerController* MyCharacterController = Cast<APlayerController>(Pilot->GetController());
//  //MyCharacterController->Possess(this);
//
//  //AC_MyCharacter* MyCharacter = Cast<AC_MyCharacter>(OtherActor);
//
//
//  AController* PawnController = Pilot->GetController();
//  PawnController->UnPossess();
//  PawnController->Possess(this);
//
//
//	Pilot->GetMovementComponent()->StopMovementImmediately();
//  Pilot->GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
//
//	isPossesed = true;
//  CurrentPilot = Pilot;
//}

void APawnSpaceShip::OnBoxOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex)
{
  //UE_LOG(LogTemp, Log, TEXT("APawnSpaceShip OnOverlapEnd called"));
  if (GEngine) {
    //GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Yellow, TEXT("APawnSpaceShip OnOverlapEnd called"));
  }
	//CurrentPilot = nullptr;
}

void APawnSpaceShip::OnComponentHitCollisionMesh(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
  if (GEngine) {
    GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Yellow, TEXT("APawnSpaceShip OnComponentHitCollisionMesh called"));
  }
	//Do nothing, we only want to dampen the landing when the pilot ejected, since the ship may be falling
	if (CurrentPilot) {
		return;
	}
	if (!bPilotExited) {
		return;
	}

	if (CollisionMesh->GetComponentVelocity().Z < 0) {
		//we are falling, and we hit the ground, dampen the fall and stop us from moving
		bPilotExited = false;
		MeshComp->SetEnableGravity(false);
    float MyMaximumSpeedValue = 0.0f;
    FVector currentVelocity = MeshComp->GetPhysicsLinearVelocity();
    FVector clampedVelocity = currentVelocity.GetClampedToMaxSize(MyMaximumSpeedValue);
    MeshComp->SetPhysicsLinearVelocity(clampedVelocity);
		MyMaximumSpeedValue = 10000.0f;
		clampedVelocity = currentVelocity.GetClampedToMaxSize(MyMaximumSpeedValue);
		MeshComp->SetPhysicsLinearVelocity(clampedVelocity);
	}

}

