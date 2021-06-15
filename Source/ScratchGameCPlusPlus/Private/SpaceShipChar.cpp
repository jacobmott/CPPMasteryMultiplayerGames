// Fill out your copyright notice in the Description page of Project Settings.


#include "SpaceShipChar.h"
#include "Components/BoxComponent.h"

// Sets default values
ASpaceShipChar::ASpaceShipChar()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	//CollisionMesh = CreateDefaultSubobject<UBoxComponent>(FName("Collision Mesh"));
	//CollisionMesh->SetSimulatePhysics(true);
	//CollisionMesh->SetCollisionObjectType(ECollisionChannel::ECC_PhysicsBody);

	MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComp"));
	//MeshComp->SetSimulatePhysics(true);
	//MeshComp->SetCollisionObjectType(ECollisionChannel::ECC_PhysicsBody);
	RootComponent = MeshComp;

	//CollisionMesh->SetupAttachment(RootComponent);
}

// Called when the game starts or when spawned
void ASpaceShipChar::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ASpaceShipChar::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void ASpaceShipChar::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	InputComponent->BindAxis("UpVectorMove", this, &ASpaceShipChar::UpVectorMove);

	InputComponent->BindAxis("YawMoveRotate", this, &ASpaceShipChar::AddControllerYawInput);
	InputComponent->BindAxis("MoveForward", this, &ASpaceShipChar::MoveForward);
}


void ASpaceShipChar::UpVectorMove(float Value) {
	AddMovementInput(GetActorUpVector() * Value);
	UE_LOG(LogTemp, Log, TEXT("UpVectorMove: %s)"), *FString::SanitizeFloat(Value));
}

void ASpaceShipChar::MoveForward(float Value) {
	AddMovementInput(GetActorForwardVector() * Value);
}

