// Fill out your copyright notice in the Description page of Project Settings.


#include "C_ExplosiveBarrel.h"
#include "C_HealthComponent.h"
#include "Kismet/GameplayStatics.h"
#include "PhysicsEngine/RadialForceComponent.h"
#include "Net/UnrealNetwork.h"

// Sets default values
AC_ExplosiveBarrel::AC_ExplosiveBarrel()
{

  HealthComponent = CreateDefaultSubobject<UC_HealthComponent>(TEXT("HealthComp"));
  HealthComponent->OnHealthChanged.AddDynamic(this, &AC_ExplosiveBarrel::OnHealthChanged);

  MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComp"));
  MeshComp->SetSimulatePhysics(true);
  MeshComp->SetCollisionObjectType(ECollisionChannel::ECC_PhysicsBody);
  RootComponent = MeshComp;

  RadialForceComponent = CreateDefaultSubobject<URadialForceComponent>(TEXT("RadialForceComp"));
  RadialForceComponent->SetupAttachment(MeshComp);
  RadialForceComponent->Radius = 250.0f;
  RadialForceComponent->bImpulseVelChange = true;
  RadialForceComponent->bAutoActivate = true;
  RadialForceComponent->bIgnoreOwningActor = true;

  ExplosionImpulse = 400.0f;

  SetReplicates(true);
  SetReplicateMovement(true);


}

void AC_ExplosiveBarrel::OnRep_Exploded()
{
  UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ExplodedMaterial, GetActorLocation());
  MeshComp->SetMaterial(0, ExplosionEffect);
}

void AC_ExplosiveBarrel::OnHealthChanged(UC_HealthComponent* HealthComp, float Health, float HealthDelta, const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser)
{

  if (bExploded) {
    return;
  }


  if (Health <= 0.0f) {

    bExploded = true;
    OnRep_Exploded();

    FVector BoostIntensity = FVector::UpVector * ExplosionImpulse;
    MeshComp->AddImpulse(BoostIntensity, NAME_None, true);

    RadialForceComponent->FireImpulse();


  }


}

void AC_ExplosiveBarrel::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
  Super::GetLifetimeReplicatedProps(OutLifetimeProps);

  DOREPLIFETIME(AC_ExplosiveBarrel, bExploded);
}