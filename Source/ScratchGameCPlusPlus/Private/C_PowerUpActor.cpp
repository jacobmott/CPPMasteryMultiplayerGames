// Fill out your copyright notice in the Description page of Project Settings.


#include "C_PowerUpActor.h"
#include "Net/UnrealNetwork.h"

// Sets default values
AC_PowerUpActor::AC_PowerUpActor()
{

	PowerupInterval = 0.0f;
	TotalNrOfTicks = 0;

	bIsPowerupActive = false;

	bReplicates = true;

}


void AC_PowerUpActor::OnTickPowerup()
{


	++TicksProcessed;
  if (HasAuthority()) {
		OnPowerupTicked();
	}

	if (TicksProcessed >= TotalNrOfTicks) {

		//Delete timer
    if (HasAuthority()) {
      bIsPowerupActive = false;
      OnRep_PowerupActive();
      GetWorldTimerManager().ClearTimer(TimerHandle_PowerupTick);
    }

		OnExpired();


	}



}

void AC_PowerUpActor::OnRep_PowerupActive()
{
	
	OnPowerupStateChanged(bIsPowerupActive);

}

void AC_PowerUpActor::ActivatePowerup(AActor* OtherActor)
{

	GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Orange, TEXT("ActivatePowerup"));
	if (HasAuthority()) {
		bIsPowerupActive = true;
		OnRep_PowerupActive();
	}

	OnActivated(OtherActor);

  if (PowerupInterval > 0.0f) {
    GetWorldTimerManager().SetTimer(TimerHandle_PowerupTick, this, &AC_PowerUpActor::OnTickPowerup, PowerupInterval, true);
  }
  else {
    OnTickPowerup();
  }

}



void AC_PowerUpActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
  Super::GetLifetimeReplicatedProps(OutLifetimeProps);

  DOREPLIFETIME(AC_PowerUpActor, bIsPowerupActive);

}

