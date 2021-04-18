// Fill out your copyright notice in the Description page of Project Settings.


#include "C_MyCharacter.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/PawnMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "C_Weapon.h"
#include "ScratchGameCPlusPlus/ScratchGameCPlusPlus.h"
#include "C_HealthComponent.h"
#include "Net/UnrealNetwork.h"


// Sets default values
AC_MyCharacter::AC_MyCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;


    SpringArmComp = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArmComp"));
	SpringArmComp->bUsePawnControlRotation = true;
	SpringArmComp->SetupAttachment(RootComponent);

	GetCapsuleComponent()->SetCollisionResponseToChannel(COLLISION_WEAPON, ECR_Ignore);

	GetMovementComponent()->GetNavAgentPropertiesRef().bCanCrouch = true;

	CameraComp = CreateDefaultSubobject<UCameraComponent>(TEXT("CameraComp"));
	CameraComp->SetupAttachment(SpringArmComp);

	ZoomedFOV = 65.0f;

	WeaponAttachSocketName = "WeaponSocket";

	HealthComponent = CreateDefaultSubobject<UC_HealthComponent>(TEXT("HealthComp"));

}

// Called when the game starts or when spawned
void AC_MyCharacter::BeginPlay()
{
	Super::BeginPlay();

	DefaultFOV = CameraComp->FieldOfView;
	HealthComponent->OnHealthChanged.AddDynamic(this, &AC_MyCharacter::OnHealthChanged);

	//if (GetLocalRole() == ROLE_Authority) {
	//HasAuthority is the same thing as above, its just inlined to it
	if (GetLocalRole() == ROLE_Authority){
    FActorSpawnParameters SpawnParams;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
    // Spawn a default weapon
    CurrentWeapon = GetWorld()->SpawnActor<AC_Weapon>(StarterWeaponClass, FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
    if (CurrentWeapon) {
      UE_LOG(LogTemp, Warning, TEXT("Have current weapon"));
      CurrentWeapon->SetOwner(this);
      CurrentWeapon->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, WeaponAttachSocketName);
    }
	}




}

void AC_MyCharacter::MoveForward(float Value)
{
	AddMovementInput(GetActorForwardVector() * Value);

}

void AC_MyCharacter::MoveRight(float Value)
{
	AddMovementInput(GetActorRightVector() * Value);
}

void AC_MyCharacter::BeginCrouch()
{
	Crouch();
}

void AC_MyCharacter::EndCrouch()
{
	UnCrouch();
}

void AC_MyCharacter::BeginJump()
{
	Jump();
}


void AC_MyCharacter::BeginZoom()
{
	WantsToZoom = true;
}

void AC_MyCharacter::EndZoom()
{
	WantsToZoom = false;
}

void AC_MyCharacter::StartFire()
{
	if (CurrentWeapon) {
		CurrentWeapon->StartFire();
	}
}

void AC_MyCharacter::StopFire()
{
  if (CurrentWeapon) {
    CurrentWeapon->StopFire();
  }
}

void AC_MyCharacter::OnHealthChanged(UC_HealthComponent* OwningHealthComp, float Health, float HealthDelta, const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser)
{
	if (Health <= 0.0f && !bDied) {
		// Die!
		bDied = true;
		GetMovementComponent()->StopMovementImmediately();
		GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

		DetachFromControllerPendingDestroy();
		SetLifeSpan(10.0f);

	}
}

// Called every frame
void AC_MyCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	float TargetFOV = WantsToZoom ? ZoomedFOV : DefaultFOV;

	float NewFOV = FMath::FInterpTo(CameraComp->FieldOfView,TargetFOV, DeltaTime, ZoomedInterpSpeed);
	CameraComp->SetFieldOfView(NewFOV);

}

// Called to bind functionality to input
void AC_MyCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	PlayerInputComponent->BindAxis("MoveForward", this, &AC_MyCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AC_MyCharacter::MoveRight);

	PlayerInputComponent->BindAxis("LookUp", this, &AC_MyCharacter::AddControllerPitchInput);
	PlayerInputComponent->BindAxis("Turn", this, &AC_MyCharacter::AddControllerYawInput);

	PlayerInputComponent->BindAction("Crouch", IE_Pressed, this, &AC_MyCharacter::BeginCrouch);
	PlayerInputComponent->BindAction("Crouch", IE_Released, this, &AC_MyCharacter::EndCrouch);


    PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &AC_MyCharacter::BeginJump);


	PlayerInputComponent->BindAction("Zoom", IE_Pressed, this, &AC_MyCharacter::BeginZoom);
	PlayerInputComponent->BindAction("Zoom", IE_Released, this, &AC_MyCharacter::EndZoom);


	PlayerInputComponent->BindAction("Fire", IE_Pressed, this, &AC_MyCharacter::StartFire);
	PlayerInputComponent->BindAction("Fire", IE_Released, this, &AC_MyCharacter::StopFire);
}

FVector AC_MyCharacter::GetPawnViewLocation() const
{

	if (CameraComp) {
		return CameraComp->GetComponentLocation();
	}

	return Super::GetPawnViewLocation();

}


void AC_MyCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
  Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AC_MyCharacter, CurrentWeapon);
	DOREPLIFETIME(AC_MyCharacter, bDied);

}
