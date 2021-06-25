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
#include "GameFramework/CharacterMovementComponent.h"


// Sets default values
AC_MyCharacter::AC_MyCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	bGenerateOverlapEventsDuringLevelStreaming = 0;
  SpringArmComp = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArmComp"));
	SpringArmComp->bUsePawnControlRotation = true;
	SpringArmComp->SetupAttachment(RootComponent);

	GetCapsuleComponent()->SetCollisionResponseToChannel(COLLISION_WEAPON, ECR_Ignore);
	GetCapsuleComponent()->SetGenerateOverlapEvents(true);

	GetMovementComponent()->GetNavAgentPropertiesRef().bCanCrouch = true;

	CameraComp = CreateDefaultSubobject<UCameraComponent>(TEXT("CameraComp"));
	CameraComp->SetupAttachment(SpringArmComp);

	ZoomedFOV = 65.0f;

	WeaponAttachSocketName = "WeaponSocket";

	HealthComponent = CreateDefaultSubobject<UC_HealthComponent>(TEXT("HealthComp"));

	vehiclePossed = false;
	bNotMoving = false;

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

  if (GEngine) {
    //GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Yellow, FString::Printf(TEXT("AC_MyCharacter: %f MoveForward called"), Value));
  }
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


	PlayerInputComponent->BindAction("EnterVehicle", IE_Pressed, this, &AC_MyCharacter::EnterVehicle);

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


void AC_MyCharacter::SetVehicleInRange(APawn* VehiclePawn)
{
  VehicleInRangePawn = VehiclePawn;
}

//void AC_MyCharacter::ExitVehicle() {
//
//
//  // UE_LOG(LogTemp, Log, TEXT("APawnSpaceShip OnOverlapEnd called"));
//  if (GEngine) {
//    GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Yellow, TEXT("AC_MyCharacter ExitVehicle called"));
//  }
//
//	if (!VehicleInRangePawn && !vehiclePossed) {
//		return;
//	}
//
//  //DidAlreadyOverlap = true;
//	
//  ///DetachFromActor(FDetachmentTransformRules(EDetachmentRule::KeepWorld, EDetachmentRule::KeepRelative, EDetachmentRule::KeepWorld, false));
//  //CurrentPilot->AttachToComponent(CurrentPilot->GetCapsuleComponent(), FAttachmentTransformRules::SnapToTargetNotIncludingScale);
//	DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
//
//  //APlayerController* ShipController = Cast<APlayerController>(GetController());
//
//  AController* PawnController = VehicleInRangePawn->GetController();
//  PawnController->UnPossess();
//  PawnController->Possess(this);
//	SetActorLocation(GetActorLocation() + FVector(500.0f, 500.0f, 500.0f), false);
//	SetActorRotation(FRotator(0.0f, 0.0f, 0.0f));
//  vehiclePossed = false;
//	VehicleInRangePawn = nullptr;
//	SetActorEnableCollision(true);
//	GetCharacterMovement()->StopMovementImmediately();
//	//GetMovementComponent()->StopMovementImmediately
//  GetCapsuleComponent()->SetCollisionResponseToChannel(COLLISION_WEAPON, ECR_Ignore);
//  GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
//  GetCapsuleComponent()->SetEnableGravity(true);
//  GetCapsuleComponent()->SetCollisionProfileName(TEXT("Pawn"));
//  GetCapsuleComponent()->CanCharacterStepUpOn = ECanBeCharacterBase::ECB_No;
//  GetCapsuleComponent()->SetGenerateOverlapEvents(true);
//	GetCapsuleComponent()->SetSimulatePhysics(true);
//
//	GetCharacterMovement()->SetActive(true, true);
//	GetCharacterMovement()->SetMovementMode(MOVE_Walking);
//	
//	//GetMovementComponent()->SetMovementMode()
//  GetMesh()->BodyInstance.bLockYRotation = true;
//	GetMesh()->BodyInstance.bLockXRotation = true;
//	GetMesh()->SetCollisionProfileName(TEXT("CharacterMesh"));
//  //GetWorldTimerManager().SetTimer(TimerHandle_FlipDidAlreadyOverlap, this, &AC_MyShip::FlipDidAlreadyOverlap, 1.0f, false, 3.0f);
//
//}


//void AC_MyCharacter::EnterVehicle()
//{
//  if (!VehicleInRangePawn) {
//		return;
//  }
//	if (vehiclePossed) {
//		return;
//	}
//
//	bNotMoving = true;
//  TArray<UStaticMeshComponent*> StaticComps;
//	VehicleInRangePawn->GetComponents<UStaticMeshComponent>(StaticComps);
//	UStaticMeshComponent* VehicleStaticMesh = StaticComps[0];
//
//  AttachToComponent(VehicleStaticMesh, FAttachmentTransformRules::SnapToTargetNotIncludingScale, "PilotSeat");
//  //APlayerController* MyCharacterController = Cast<APlayerController>(Pilot->GetController());
//  //MyCharacterController->Possess(this);
//	GetCharacterMovement()->StopMovementImmediately();
//  //AC_MyCharacter* MyCharacter = Cast<AC_MyCharacter>(OtherActor);
//  //GetMovementComponent()->StopMovementImmediately();
//  GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
//	GetCapsuleComponent()->SetCollisionResponseToAllChannels(ECR_Ignore);
//  GetCapsuleComponent()->SetGenerateOverlapEvents(false);
//	GetCapsuleComponent()->SetSimulatePhysics(false);
//	this->SetActorEnableCollision(false);
//
//  AController* PawnController = GetController();
//  PawnController->UnPossess();
//  PawnController->Possess(VehicleInRangePawn);
//	VehicleStaticMesh->SetEnableGravity(false);
//
//
//
//	vehiclePossed = true;
//
//
//}


//Unreal pawn possesssion
//http://jollymonsterstudio.com/2019/09/05/unreal-engine-c-fundamentals-character-possession-and-changing-materials/
//unreal phyics goes crazy after calling attachtocoponent
//https://answers.unrealengine.com/questions/664076/actors-physics-go-crazy-on-attach.html
// Collision Overview
//https://docs.unrealengine.com/4.26/en-US/InteractiveExperiences/Physics/Collision/Overview/
void AC_MyCharacter::EnterVehicle()
{
  if (!VehicleInRangePawn) {
    return;
  }
  if (vehiclePossed) {
    return;
  }

  TArray<UStaticMeshComponent*> StaticComps;
  VehicleInRangePawn->GetComponents<UStaticMeshComponent>(StaticComps);
  UStaticMeshComponent* VehicleStaticMesh = StaticComps[0];

  
  // save a copy of our controller
  AController* SavedController = GetController();
  // unpossess first ( helps with multiplayer )
  // disable movement mode
	GetCharacterMovement()->StopMovementImmediately();
  GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_None);
	MoveIgnoreActorAdd(VehicleInRangePawn);
	SavedController->UnPossess();
  // possess our new actor
  SavedController->Possess(VehicleInRangePawn);
	VehicleInRangePawn->MoveIgnoreActorAdd(this);
	VehicleStaticMesh->SetSimulatePhysics(false);
	AttachToComponent(VehicleStaticMesh, FAttachmentTransformRules::SnapToTargetNotIncludingScale, "PilotSeat");
	VehicleStaticMesh->SetSimulatePhysics(true);
	VehicleStaticMesh->SetEnableGravity(false);
	GetMesh()->SetEnableGravity(false);
	GetCapsuleComponent()->SetEnableGravity(false);
  vehiclePossed = true;


}




void AC_MyCharacter::ExitVehicle() {


  // UE_LOG(LogTemp, Log, TEXT("APawnSpaceShip OnOverlapEnd called"));
  if (GEngine) {
    GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Yellow, TEXT("AC_MyCharacter ExitVehicle called"));
  }

  if (!VehicleInRangePawn && !vehiclePossed) {
    return;
  }

  //DidAlreadyOverlap = true;

  ///DetachFromActor(FDetachmentTransformRules(EDetachmentRule::KeepWorld, EDetachmentRule::KeepRelative, EDetachmentRule::KeepWorld, false));
  //CurrentPilot->AttachToComponent(CurrentPilot->GetCapsuleComponent(), FAttachmentTransformRules::SnapToTargetNotIncludingScale);
  DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);


  // save a copy of our controller
  AController* SavedController = VehicleInRangePawn->GetController();
  // unpossess first ( helps with multiplayer )
  SavedController->UnPossess();
	SavedController->Possess(this);
	GetMesh()->SetEnableGravity(true);
	GetCapsuleComponent()->SetEnableGravity(true);
	SetActorLocation(GetActorLocation() + FVector(500.0f, 500.0f, 500.0f), false);
  // disable movement mode
  VehicleInRangePawn->MoveIgnoreActorRemove(this);
  this->MoveIgnoreActorRemove(VehicleInRangePawn);
  GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_Walking);

  TArray<UStaticMeshComponent*> StaticComps;
  VehicleInRangePawn->GetComponents<UStaticMeshComponent>(StaticComps);
  UStaticMeshComponent* VehicleStaticMesh = StaticComps[0];
  // possess our new actor


	VehicleInRangePawn = nullptr;
	vehiclePossed = false;
}