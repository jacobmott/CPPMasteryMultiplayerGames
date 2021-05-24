// Fill out your copyright notice in the Description page of Project Settings.


#include "C_Weapon.h"
#include "Components/SkeletalMeshComponent.h"
#include "DrawDebugHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystem.h"
#include "Components/SkeletalMeshComponent.h"
#include "Particles/ParticleSystemComponent.h"
#include "PhysicalMaterials/PhysicalMaterial.h"
#include "ScratchGameCPlusPlus/ScratchGameCPlusPlus.h"
#include "TimerManager.h"
#include "Net/UnrealNetwork.h"


static int32 DebugWeaponDrawing = 0;
FAutoConsoleVariableRef CVARDebugWeaponDrawing(TEXT("COOP.DebugWeapons"), DebugWeaponDrawing, TEXT("Draw debug lines for Weapons"), ECVF_Cheat);


// Sets default values
AC_Weapon::AC_Weapon()
{

	MeshComp = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("MeshComp"));
	RootComponent = MeshComp;

	MuzzleSocketName = "MuzzleSocket";
	TracerTargetName = "Target";

	BaseDamage = 20.0f;
	RateOfFire = 600;

	SetReplicates(true);

	NetUpdateFrequency = 66.0f;
	MinNetUpdateFrequency = 33.0f;

	BulletSpread = 2.0f;

}

void AC_Weapon::BeginPlay()
{
	Super::BeginPlay();

	TimeBetweenShots = 60 / RateOfFire;

}

void AC_Weapon::Fire() {


  if (GetLocalRole() < ROLE_Authority) {
    ServerFire();
  }


	//Trace the world from pawn eyes to crosshair location
	AActor* MYOwner = GetOwner();
	if (MYOwner) {

		FVector EyeLocation;
		FRotator EyeRotation;
		MYOwner->GetActorEyesViewPoint(EyeLocation, EyeRotation);

		FVector ShotDirection = EyeRotation.Vector();

		// Bullet Spread
		float HalfRad = FMath::DegreesToRadians(BulletSpread);
		ShotDirection = FMath::VRandCone(ShotDirection, HalfRad, HalfRad);
		FVector TraceEnd = EyeLocation + (ShotDirection * 10000);

		FCollisionQueryParams QueryParams;
		QueryParams.AddIgnoredActor(MYOwner);
		QueryParams.AddIgnoredActor(this);
		QueryParams.bTraceComplex = true;
		QueryParams.bReturnPhysicalMaterial = true;


		EPhysicalSurface SurfaceType = SurfaceType_Default;

		//Particle "Target" parameter
		FVector TracerEndPoint = TraceEnd;
		UE_LOG(LogTemp, Warning, TEXT("Called AC_Weapon::Fire"));
		FHitResult Hit;
		if (GetWorld()->LineTraceSingleByChannel(Hit, EyeLocation, TraceEnd, COLLISION_WEAPON, QueryParams)) {
			//Blocking hit process damage
			AActor* HitActor = Hit.GetActor();

			SurfaceType = UPhysicalMaterial::DetermineSurfaceType(Hit.PhysMaterial.Get());
			float ActualDamage = BaseDamage;
      if (SurfaceType == SURFACE_FLESHVUNERABLE) {
				ActualDamage *= 4.0f;
      }

			UGameplayStatics::ApplyPointDamage(HitActor, ActualDamage, ShotDirection, Hit, MYOwner->GetInstigatorController(), this, DamageType);

			PlayImpaceEffects(SurfaceType, Hit.ImpactPoint);

			TracerEndPoint = Hit.ImpactPoint;

		}

		if (DebugWeaponDrawing > 0) {
			DrawDebugLine(GetWorld(), EyeLocation, TraceEnd, FColor::White, false, 1.0f, 0, 1.0f);
		}

		PlayFireEffects(TracerEndPoint);

    //We only set traceto, becuase the client has tracefrom, since its the clients weapons muzzle
    if (GetLocalRole() == ROLE_Authority) {
      HitScanTrace.TraceTo = TracerEndPoint;
			HitScanTrace.SurfaceType = SurfaceType;
    }

    LastFireTime = GetWorld()->TimeSeconds;

	}

}


void AC_Weapon::ServerFire_Implementation()
{
	Fire();
}

bool AC_Weapon::ServerFire_Validate()
{
	return true;
}

void AC_Weapon::StartFire()
{

	float FirstDelay = FMath::Max(LastFireTime + TimeBetweenShots - GetWorld()->TimeSeconds, 0.0f);

	GetWorldTimerManager().SetTimer(TimerHandle_TimeBetweenShots, this, &AC_Weapon::Fire, TimeBetweenShots, true, FirstDelay);

}

void AC_Weapon::StopFire()
{
	GetWorldTimerManager().ClearTimer(TimerHandle_TimeBetweenShots);
}



void AC_Weapon::PlayFireEffects(FVector TracerEndPoint)
{

  if (MuzzleEffect) {
    UGameplayStatics::SpawnEmitterAttached(MuzzleEffect, MeshComp, MuzzleSocketName);
  }


  if (TracerEffect) {

    FVector MuzzleLocation = MeshComp->GetSocketLocation(MuzzleSocketName);
    UParticleSystemComponent* TracerComp = UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), TracerEffect, MuzzleLocation);

    if (TracerComp) {
      TracerComp->SetVectorParameter(TracerTargetName, TracerEndPoint);
    }
  }


	APawn* MyOwner = Cast<APawn>(GetOwner());
	if (MyOwner) {
		APlayerController* PC =  Cast<APlayerController>(MyOwner->GetController());
		if (PC) {
			UE_LOG(LogTemp, Warning, TEXT("CameraShake called!"));
			PC->ClientPlayCameraShake(CameraShake);
		}
	}

}

void AC_Weapon::PlayImpaceEffects(EPhysicalSurface SurfaceType, FVector ImpactPoint)
{
  UParticleSystem* SelectedEffect = nullptr;

  switch (SurfaceType)
  {
  case SURFACE_FLESHDEFAULT:
  case SURFACE_FLESHVUNERABLE:
    SelectedEffect = FleshImpactEffect;
    break;
  default:
    SelectedEffect = DefaultImpactEffect;
    break;
  }

  if (SelectedEffect) {

		FVector MuzzleLocation = MeshComp->GetSocketLocation(MuzzleSocketName);

		FVector ShotDirection = ImpactPoint - MuzzleLocation;
		ShotDirection.Normalize();
    UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), SelectedEffect, ImpactPoint, ShotDirection.Rotation());
  }
}

void AC_Weapon::OnRep_HitScanTrace()
{
	//Play Cosmetic effects

	PlayFireEffects(HitScanTrace.TraceTo);
	PlayImpaceEffects(HitScanTrace.SurfaceType, HitScanTrace.TraceTo);

}


void AC_Weapon::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(AC_Weapon, HitScanTrace, COND_SkipOwner);

}



