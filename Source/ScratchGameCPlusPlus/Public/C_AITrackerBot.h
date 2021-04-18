// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "C_HealthComponent.h"
#include "C_AITrackerBot.generated.h"


class UStaticMeshComponent;
class UC_HealthComponent;
class USphereComponent;
class USoundCue;

UCLASS()
class SCRATCHGAMECPLUSPLUS_API AC_AITrackerBot : public APawn
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	AC_AITrackerBot();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(VisibleDefaultsOnly, Category = "Components")
	UStaticMeshComponent* MeshComp;

	UFUNCTION()
	FVector GetNextPathPoint();

	//Next point in navigation path
	FVector NextPathPoint;

	UPROPERTY(VisibleDefaultsOnly, Category = "Components")
  USphereComponent* SphereComp;


	UPROPERTY(EditDefaultsOnly, Category = "TrackerBot")
	float MovementForce;
	UPROPERTY(EditDefaultsOnly, Category = "TrackerBot")
	bool bUseVelocityChange;
	UPROPERTY(EditDefaultsOnly, Category = "TrackerBot")
	float RequiredDistanceToTarget;

	UPROPERTY(VisibleDefaultsOnly, Category = "Components")
	UC_HealthComponent* HealthComp;

	UFUNCTION()
	void HandleTakeDamage(UC_HealthComponent* HealthCompent, float Health, float HealthDelta,
		const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser);

	//DynamicMaterial to pulse on damage
	UMaterialInstanceDynamic* MatInst;

	void SelfDestruct();

	UPROPERTY(EditDefaultsOnly, Category = "TrackerBot")
	UParticleSystem* ExplosionEffect;


	bool bExploded;

	UPROPERTY(EditDefaultsOnly, Category = "TrackerBot")
	float ExplosionRadius;
	UPROPERTY(EditDefaultsOnly, Category = "TrackerBot")
	float ExplosionDamage;

	FTimerHandle TimerHandle_SelfDamage;

	void DamageSelf();

	bool bStartedSelfDestruction;

	UPROPERTY(EditDefaultsOnly, Category = "TrackerBot")
	USoundCue* SelfDestructSound;

	UPROPERTY(EditDefaultsOnly, Category = "TrackerBot")
	USoundCue* ExplodeSound;

	UPROPERTY(EditDefaultsOnly, Category = "TrackerBot")
	float SelfDamageInterval;



  // Find nearby enemies and grow in 'power level' based on the amount.
  void OnCheckNearbyBots();


public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	virtual void NotifyActorBeginOverlap(AActor* OtherActor) override;

};
