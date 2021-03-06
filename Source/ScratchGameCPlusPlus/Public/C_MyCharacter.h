// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "C_MyCharacter.generated.h"

class UCameraComponent;
class USpringArmComponent;
class AC_Weapon;
class UC_HealthComponent;

UCLASS()
class SCRATCHGAMECPLUSPLUS_API AC_MyCharacter : public ACharacter
{
    GENERATED_BODY()

public:
    // Sets default values for this character's properties
    AC_MyCharacter();

protected:
    // Called when the game starts or when spawned
    virtual void BeginPlay() override;

    void MoveForward(float Value);
    void MoveRight(float Value);

    void BeginCrouch();
    void EndCrouch();

    void BeginJump();

    void BeginZoom();
    void EndZoom();

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components");
    UCameraComponent* CameraComp;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components");
    USpringArmComponent* SpringArmComp;

    bool WantsToZoom;

    UPROPERTY(EditDefaultsOnly, Category = "Player");
    float ZoomedFOV;

    float DefaultFOV;

    UPROPERTY(EditDefaultsOnly, Category = "Player", meta = (ClampMin = 0.1, ClampMax = 100));
    float ZoomedInterpSpeed;

    UPROPERTY(Replicated)
    AC_Weapon* CurrentWeapon;

    UPROPERTY(EditDefaultsOnly, Category = "Player");
    TSubclassOf<AC_Weapon> StarterWeaponClass;

    UPROPERTY(VisibleDefaultsOnly, Category = "Player");
    FName WeaponAttachSocketName;


    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components");
    UC_HealthComponent* HealthComponent;


    UFUNCTION()
    void OnHealthChanged(UC_HealthComponent* OwningHealthComp, float Health, float HealthDelta, const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser);

    // Pawn died previously 
    UPROPERTY(Replicated, BlueprintReadOnly, Category = "Player")
    bool bDied;

    APawn* VehicleInRangePawn;

    bool vehiclePossed;

    bool bNotMoving;



public: 
    // Called every frame
    virtual void Tick(float DeltaTime) override;

    // Called to bind functionality to input
    virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

    virtual FVector GetPawnViewLocation() const override;


    UFUNCTION(BlueprintCallable, Category = "Player")
    void StartFire();

    UFUNCTION(BlueprintCallable, Category = "Player")
    void StopFire();

    void EnterVehicle();

    void ExitVehicle();

public:
    void SetVehicleInRange(APawn* VehiclePawn);


};
