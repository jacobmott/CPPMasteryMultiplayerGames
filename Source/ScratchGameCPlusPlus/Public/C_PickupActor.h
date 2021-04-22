// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "C_PickupActor.generated.h"


class USphereComponent;
class UDecalComponent;
class AC_PowerUpActor;

UCLASS()
class SCRATCHGAMECPLUSPLUS_API AC_PickupActor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AC_PickupActor();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	USphereComponent* SphereComp;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	UDecalComponent* DecalComp;

	UPROPERTY(EditInstanceOnly, Category = "PickupActor")
	TSubclassOf<AC_PowerUpActor> PowerUpClass;

	AC_PowerUpActor* PowerUpInstance;


	FTimerHandle TimerHandle_RespawnTimer;
	UPROPERTY(EditInstanceOnly, Category = "PickupActor")
	float CooldownDuration;

  UFUNCTION()
  void Respawn();

public:	


	virtual void NotifyActorBeginOverlap(AActor* OtherActor) override;


};
