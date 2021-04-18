// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "C_ExplosiveBarrel.generated.h"

class UC_HealthComponent;
class UStaticMeshComponent;
class URadialForceComponent;
class UParticleSystem;


UCLASS()
class SCRATCHGAMECPLUSPLUS_API AC_ExplosiveBarrel : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AC_ExplosiveBarrel();

protected:

	UPROPERTY(VisibleAnywhere, Category = "Components")
	UC_HealthComponent* HealthComponent;


  UPROPERTY(VisibleAnywhere, Category = "Components")
	UStaticMeshComponent* MeshComp;	


  UPROPERTY(VisibleAnywhere, Category = "Components")
	URadialForceComponent* RadialForceComponent;


  UPROPERTY(VisibleAnywhere, Category = "Components")
  UParticleSystem* ParticleSystem;

	UPROPERTY(ReplicatedUsing=OnRep_Exploded)
	bool bExploded;

	UFUNCTION()
	void OnRep_Exploded();

	UPROPERTY(EditDefaultsOnly, Category = "FX")
	float ExplosionImpulse;

  UPROPERTY(EditDefaultsOnly, Category = "FX")
	UParticleSystem* ExplodedMaterial;

	UPROPERTY(EditDefaultsOnly, Category = "FX")
	UMaterialInterface* ExplosionEffect;

  UFUNCTION()
  void OnHealthChanged(UC_HealthComponent* HealthComp, float Health, float HealthDelta, 
		const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser);


};
