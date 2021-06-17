// Fill out your copyright notice in the Description page of Project Settings.


#include "SpaceShipChar.h"
#include "Components/BoxComponent.h"
#include "GameFramework/PawnMovementComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/ArrowComponent.h"
	
// Sets default values
ASpaceShipChar::ASpaceShipChar()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	//CollisionMesh = CreateDefaultSubobject<UBoxComponent>(FName("Collision Mesh"));

	MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComp"));
	MeshComp->SetSimulatePhysics(true);
	//MeshComp->SetCollisionObjectType(ECollisionChannel::ECC_PhysicsBody);
	RootComponent = MeshComp;

	//CollisionMesh->SetBoxExtent(FVector(200,200,200));
	//CollisionMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	//CollisionMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block);
	//CollisionMesh->SetCollisionResponseToChannel(ECC_Pawn, ECollisionResponse::ECR_Overlap);
	//CollisionMesh->SetupAttachment(RootComponent);

	//MeshComp->GetStaticMesh()->GetBounds().BoxExtent
	GetCapsuleComponent()->SetCapsuleRadius(200);
    GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	GetCapsuleComponent()->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block);


	auto components = GetComponents();
	for (auto& component : components)
	{
		if (auto arrowComponent = Cast<UArrowComponent>(component))
		{
			arrowComponent->DestroyComponent();
			break;
		}
	}


}

// Called when the game starts or when spawned
void ASpaceShipChar::BeginPlay()
{
	Super::BeginPlay();
	GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_Flying);
	
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
	PlayerInputComponent->BindAxis("UpVectorMove", this, &ASpaceShipChar::UpVectorMove);

	PlayerInputComponent->BindAxis("YawMoveRotate", this, &ASpaceShipChar::AddControllerYawInput);
	PlayerInputComponent->BindAxis("MoveForward", this, &ASpaceShipChar::MoveForward);
}


void ASpaceShipChar::UpVectorMove(float Value) {
	AddMovementInput(GetActorUpVector() * Value);
	UE_LOG(LogTemp, Log, TEXT("UpVectorMove: %s)"), *FString::SanitizeFloat(Value), true);

}

void ASpaceShipChar::MoveForward(float Value) {
	AddMovementInput(GetActorForwardVector() * Value);
	UE_LOG(LogTemp, Log, TEXT("MoveForward: %s)"), *FString::SanitizeFloat(Value), true);
}

