// Fill out your copyright notice in the Description page of Project Settings.


#include "SpaceShip2.h"
#include "Components/BoxComponent.h"


// Sets default values
ASpaceShip2::ASpaceShip2()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	CollisionMesh = CreateDefaultSubobject<UBoxComponent>(FName("Collision Mesh"));
	CollisionMesh->SetSimulatePhysics(true);
	CollisionMesh->SetCollisionObjectType(ECollisionChannel::ECC_PhysicsBody);

	MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComp"));
	//MeshComp->SetSimulatePhysics(true);
	//MeshComp->SetCollisionObjectType(ECollisionChannel::ECC_PhysicsBody);
	RootComponent = MeshComp;

	CollisionMesh->SetupAttachment(RootComponent);

}

// Called when the game starts or when spawned
void ASpaceShip2::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ASpaceShip2::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void ASpaceShip2::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	InputComponent->BindAxis("UpVectorMove", this, &ASpaceShip2::UpVectorMove);

	InputComponent->BindAxis("YawMoveRotate", this, &ASpaceShip2::AddControllerYawInput);
	InputComponent->BindAxis("MoveForward", this, &ASpaceShip2::MoveForward);
}

void ASpaceShip2::UpVectorMove(float Value) {
	AddMovementInput(GetActorUpVector() * Value);
	UE_LOG(LogTemp, Log, TEXT("UpVectorMove: %s)"), *FString::SanitizeFloat(Value));
}

void ASpaceShip2::MoveForward(float Value) {
	AddMovementInput(GetActorForwardVector() * Value);
}


