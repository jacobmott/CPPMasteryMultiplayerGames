// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "SpaceShipChar.generated.h"

class UBoxComponent;

UCLASS()
class SCRATCHGAMECPLUSPLUS_API ASpaceShipChar : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ASpaceShipChar();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	//UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components");
	//UBoxComponent* CollisionMesh = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components");
	UStaticMeshComponent* MeshComp = nullptr;


	void UpVectorMove(float Value);
	void MoveForward(float Value);


public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

};
