// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "C_PlayerState.generated.h"

/**
 * 
 */
UCLASS()
class SCRATCHGAMECPLUSPLUS_API AC_PlayerState : public APlayerState
{
	GENERATED_BODY()


public:

	UFUNCTION(BlueprintCallable, Category="PlayerState")
	void AddScore(float ScoreDelta);
	
};
