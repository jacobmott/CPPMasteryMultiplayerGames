// Fill out your copyright notice in the Description page of Project Settings.


#include "C_PickupActor.h"
#include "Components/SphereComponent.h"
#include "Components/DecalComponent.h"
#include "TimerManager.h"
#include "C_PowerUpActor.h"

// Sets default values
AC_PickupActor::AC_PickupActor()
{


	SphereComp = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComp"));
	SphereComp->SetSphereRadius(75.0f);
	SetRootComponent(SphereComp);

	DecalComp = CreateDefaultSubobject<UDecalComponent>(TEXT("DecalComp"));
	DecalComp->SetupAttachment(SphereComp);
	DecalComp->DecalSize = FVector(64, 75, 75);
	DecalComp->SetRelativeRotation(FRotator(90.0f, 0.0f, 0.0f));

	CooldownDuration = 10.0f;

	bReplicates = true;

}

// Called when the game starts or when spawned
void AC_PickupActor::BeginPlay()
{

	Super::BeginPlay();
  //if (GetLocalRole() == ROLE_Authority) {
  //HasAuthority is the same thing as above, its just inlined to it
	//if (GetLocalRole() == ROLE_Authority) {
	if (HasAuthority()){
		Respawn();
	}
}

void AC_PickupActor::Respawn()
{

	if (PowerUpClass == nullptr) {
		UE_LOG(LogTemp, Warning, TEXT("PowerupClass is nullptr in %s. Please update your Blueprint"), *GetName());
		return;
	}
	FActorSpawnParameters SpawnParams; 
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	UE_LOG(LogTemp, Warning, TEXT("PowerupClass Spawning actor"));
	PowerUpInstance = GetWorld()->SpawnActor<AC_PowerUpActor>(PowerUpClass, GetTransform(), SpawnParams);
}

void AC_PickupActor::NotifyActorBeginOverlap(AActor* OtherActor)
{
	Super::NotifyActorBeginOverlap(OtherActor);
	UE_LOG(LogTemp, Warning, TEXT("AC_PickupActor: NotifyActorBeginOverlap"));

	if (PowerUpInstance) {
		UE_LOG(LogTemp, Warning, TEXT("AC_PickupActor: NotifyActorBeginOverlap: HERE1"));
		PowerUpInstance->ActivatePowerup(OtherActor);
		PowerUpInstance = nullptr;

		if (HasAuthority()) {
			UE_LOG(LogTemp, Warning, TEXT("AC_PickupActor: NotifyActorBeginOverlap: HERE2"));
			//Set timer to respawn
			GetWorldTimerManager().SetTimer(TimerHandle_RespawnTimer, this, &AC_PickupActor::Respawn, CooldownDuration, false);
		}
	}

  // @TODO:  Grant a powerup to player if available

}



