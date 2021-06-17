// Fill out your copyright notice in the Description page of Project Settings.


#include "PawnSpaceShip.h"
#include "Components/BoxComponent.h"
#include "GameFramework/PawnMovementComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/InputComponent.h"
#include "Components/StaticMeshComponent.h"



#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerController.h"// Sets default values

APawnSpaceShip::APawnSpaceShip()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
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
	MeshComp->SetCollisionObjectType(ECollisionChannel::ECC_PhysicsBody);
	RootComponent = MeshComp;
	MeshComp->BodyInstance.bLockYRotation = true;
	MeshComp->BodyInstance.bLockXRotation = true;
	MeshComp->BodyInstance.SetUseCCD(true);
	MeshComp->SetUseCCD(true);

	//CollisionMesh = CreateDefaultSubobject<UBoxComponent>(FName("Collision Mesh"));
	//CollisionMesh->SetBoxExtent(FVector(200,200,200));
	//CollisionMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	//CollisionMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block);
	//CollisionMesh->SetCollisionResponseToChannel(ECC_Pawn, ECollisionResponse::ECR_Overlap);
	//CollisionMesh->SetupAttachment(RootComponent);

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




}

// Called when the game starts or when spawned
void APawnSpaceShip::BeginPlay()
{
	Super::BeginPlay();
	DefaultFOV = CameraComp->FieldOfView;
	InitalRotation = GetActorRotation();
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

}

void APawnSpaceShip::YawMoveRotate(float Value) {
	bWantToYaw = false;
	if (Value > 0) {
		Yaw = 1;
		bWantToYaw = true;
		UE_LOG(LogTemp, Log, TEXT("YawMoveRotate: %s)"), *FString::SanitizeFloat(Value), true);
	}
	if (Value < 0) {
		Yaw = -1;
		bWantToYaw = true;
		UE_LOG(LogTemp, Log, TEXT("YawMoveRotate: %s)"), *FString::SanitizeFloat(Value), true);
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
	UE_LOG(LogTemp, Log, TEXT("UpVectorMove: %s)"), *FString::SanitizeFloat(Value), true);

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
	UE_LOG(LogTemp, Log, TEXT("MoveForward: %s)"), *FString::SanitizeFloat(Value), true);
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
	UE_LOG(LogTemp, Log, TEXT("CameraRotateX: %s)"), *FString::SanitizeFloat(Value), true);
}

void APawnSpaceShip::CameraRotateY(float Value) {
	bCameraRotateY = false;
	if (Value > 0 || Value < 0) {
		CameraYDirection = Value;
		bCameraRotateY = true;
	}
	UE_LOG(LogTemp, Log, TEXT("CameraRotateY: %s)"), *FString::SanitizeFloat(Value), true);
}


void APawnSpaceShip::CameraZoomOut()
{
	SpringArmComp->TargetArmLength += 200;
}

void APawnSpaceShip::CameraZoomIn()
{
	SpringArmComp->TargetArmLength -= 200;
}
