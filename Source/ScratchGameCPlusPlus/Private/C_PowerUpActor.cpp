// Fill out your copyright notice in the Description page of Project Settings.


#include "C_PowerUpActor.h"

// Sets default values
AC_PowerUpActor::AC_PowerUpActor()
{

	PowerupInterval = 0.0f;
	TotalNrOfTicks = 0;


}

// Called when the game starts or when spawned
void AC_PowerUpActor::BeginPlay()
{
	Super::BeginPlay();



}

void AC_PowerUpActor::OnTickPowerup()
{

	++TicksProcessed;

	OnPowerupTicked();

	if (TicksProcessed >= TotalNrOfTicks) {
		OnExpired();
		//Delete timer
		GetWorldTimerManager().ClearTimer(TimerHandle_PowerupTick);
	}



}

void AC_PowerUpActor::ActivatePowerup()
{

	OnActivated();

  if (PowerupInterval > 0.0f) {
		GetWorldTimerManager().SetTimer(TimerHandle_PowerupTick, this, &AC_PowerUpActor::OnTickPowerup, PowerupInterval, true);
  }
  else {
    OnTickPowerup();
  }
}

