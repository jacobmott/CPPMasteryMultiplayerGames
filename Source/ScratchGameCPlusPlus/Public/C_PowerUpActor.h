// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "C_PowerUpActor.generated.h"



UCLASS()
class SCRATCHGAMECPLUSPLUS_API AC_PowerUpActor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AC_PowerUpActor();

protected:

	/* Time Between Powerup Ticks */
	UPROPERTY(EditDefaultsOnly, Category = "Powerups")
	float PowerupInterval;

	/* Total time we apply the powerup effect */
	UPROPERTY(EditDefaultsOnly, Category = "Powerups")
	int32 TotalNrOfTicks;

	//Total number of ticks applied
	int32 TicksProcessed;

	FTimerHandle TimerHandle_PowerupTick;

	UFUNCTION()
	void OnTickPowerup();

	UPROPERTY(ReplicatedUsing=OnRep_PowerupActive)
	bool bIsPowerupActive;

	//Keeps state of the powerup
  UFUNCTION()
  void OnRep_PowerupActive();


	UFUNCTION(BlueprintImplementableEvent, Category = "PowerUps")
	void OnPowerupStateChanged(bool value);


public:	


	void ActivatePowerup(AActor* OtherActor);

	UFUNCTION(BlueprintImplementableEvent, Category = "PowerUps")
	void OnActivated(AActor* ActiveFor);

	UFUNCTION(BlueprintImplementableEvent, Category = "PowerUps")
  void OnExpired();

  UFUNCTION(BlueprintImplementableEvent, Category = "PowerUps")
  void OnPowerupTicked();

	

};
