// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "C_GameModeBase.generated.h"



DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnActorKilledSignature, AActor*, VictimActor, AActor*, KillerActor, AController*, KillerController);

enum class EWaveState: uint8;


/**
 * 
 */
UCLASS()
class SCRATCHGAMECPLUSPLUS_API AC_GameModeBase : public AGameModeBase
{
	GENERATED_BODY()


protected:


  FTimerHandle TimerHandle_BotSpawner;
	FTimerHandle TimerHandle_NextWaveStart;

	//Bots to spawn in current wave
	int32 NrOfBotsToSpawn;

	int32 WaveCount;

	UPROPERTY(EditDefaultsOnly, Category="GameMode")
	float TimeBetweenWaves;


protected:


	//Hook for BP(Blueprint code in editor, blueprint diagram) to spawn a single bot
	UFUNCTION(BlueprintImplementableEvent, Category="GameMode")
	void SpawnNewBot();



	void SpawnBotTimerElapsed();


	//Start spawning bots
	void StartWave();


	//Stop spawning bots
	void EndWave();


	//Set Timer for next start wave
	void PrepareForNextWave();


	void CheckWaveState();

	void CheckAnyPlayerAlive();

	void GameOver();

	void SetWaveState(EWaveState NewState);


	void RestartDeadPlayers();


public:

	AC_GameModeBase();

	virtual void StartPlay() override;

	virtual void Tick(float DeltaSeconds) override;

	UPROPERTY(BlueprintAssignable, Category = "GameMode")
	FOnActorKilledSignature OnActorKilled;
	
};
