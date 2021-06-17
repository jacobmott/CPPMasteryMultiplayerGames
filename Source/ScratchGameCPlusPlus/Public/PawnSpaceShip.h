// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "PawnSpaceShip.generated.h"

class UBoxComponent;
class UCameraComponent;
class USpringArmComponent;

UCLASS()
class SCRATCHGAMECPLUSPLUS_API APawnSpaceShip : public APawn
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	APawnSpaceShip();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	//UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components");
	//UBoxComponent* CollisionMesh = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components");
	UStaticMeshComponent* MeshComp = nullptr;

	float Yaw = 0;
	void UpVectorMove(float Value);
	void MoveForward(float Value);
	void YawMoveRotate(float Value);
	void FixRotation();
	bool bWantToYaw;
	bool bMoveForward;
	bool bMoveUp;
	float ForwardDirection;
	float UpDirection;
	bool bFixRotation; 
	FRotator InitalRotation;
	FVector MovementDirection;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Movement");
	float MovementSpeed = 1000.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Rotation");
	float RotationSpeed = 10.0f;

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

	void HandleCollision(FHitResult* Hit);

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

};
