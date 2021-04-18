// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "C_Weapon.h"
#include "C_GrenadeLauncher.generated.h"

/**
 * 
 */
UCLASS()
class SCRATCHGAMECPLUSPLUS_API AC_GrenadeLauncher : public AC_Weapon
{
	GENERATED_BODY()

protected:

	virtual void Fire() override;

	UPROPERTY(EditDefaultsOnly, Category = "ProjectileWeapon")
	TSubclassOf<AActor> ProjectileClass;

  UPROPERTY(EditDefaultsOnly, Category = "Player");
  TSubclassOf<AActor> ActorClassToCheckForRadialDamageOn;


  //UPROPERTY(ReplicatedUsing = OnRep_Bullet)
 // FHitScanTrace HitScanTrace;

  //UFUNCTION()
  //void OnRep_Bullet()


public:
  void StartFire();

  void StopFire();
	
};
