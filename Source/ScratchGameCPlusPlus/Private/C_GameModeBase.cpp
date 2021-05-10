// Fill out your copyright notice in the Description page of Project Settings.


#include "C_GameModeBase.h"
#include "TimerManager.h"
#include "C_HealthComponent.h"
#include "C_GameStateBase.h"

AC_GameModeBase::AC_GameModeBase()
{
  TimeBetweenWaves = 2.0f;

  WaveCount = 0;


  PrimaryActorTick.bCanEverTick = true;
  PrimaryActorTick.TickInterval = 1.0f;

  GameStateClass = AC_GameStateBase::StaticClass();

}

void AC_GameModeBase::StartWave()
{

  ++WaveCount;

  NrOfBotsToSpawn = 2 * WaveCount;

  GetWorldTimerManager().SetTimer(TimerHandle_BotSpawner, this, &AC_GameModeBase::SpawnBotTimerElapsed, 1.0f, true, 1.0f);

  SetWaveState(EWaveState::WaveInProgress);

}

void AC_GameModeBase::EndWave()
{
  GetWorldTimerManager().ClearTimer(TimerHandle_BotSpawner);


  //Instead of starting the next wave after you have spawned all the bots for the current wave,
  //We want to check that all the bots from the current wave have been killed or have died before we start/prepare the next wave
  //PrepareForNextWave(); 

  SetWaveState(EWaveState::WaitingToComplete);
}

void AC_GameModeBase::PrepareForNextWave()
{


  GetWorldTimerManager().SetTimer(TimerHandle_NextWaveStart, this, &AC_GameModeBase::StartWave, TimeBetweenWaves, false);

  SetWaveState(EWaveState::WaitingToStart);

}


void AC_GameModeBase::CheckWaveState()
{

  bool bIsPreparingForNextWave = GetWorldTimerManager().IsTimerActive(TimerHandle_NextWaveStart);

  if (NrOfBotsToSpawn > 0 || bIsPreparingForNextWave) {
    return;
  }

  bool bIsAnyBotAlive = false;

  for (FConstPawnIterator It = GetWorld()->GetPawnIterator(); It; ++It) {


    APawn* TestPawn = It->Get();

    if (TestPawn == nullptr || TestPawn->IsPlayerControlled()) {
      continue;
    }

    UC_HealthComponent* HealthComp = Cast<UC_HealthComponent>(TestPawn->GetComponentByClass(UC_HealthComponent::StaticClass()));

    if (HealthComp && HealthComp->GetHealth() > 0.0f) {
      bIsAnyBotAlive = true;
      break;
    }


  }

  if (!bIsAnyBotAlive) {
    PrepareForNextWave();
  }

}

void AC_GameModeBase::CheckAnyPlayerAlive()
{

  for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It) {

    APlayerController* PC = It->Get();

    if (PC && PC->GetPawn()) {

      APawn* MyPawn = PC->GetPawn();
      UC_HealthComponent* HealthComp = Cast<UC_HealthComponent>(MyPawn->GetComponentByClass(UC_HealthComponent::StaticClass()));
      if (ensure(HealthComp) && HealthComp->GetHealth() > 0.0f) {
        // A player is still alive
        return;
      }


    }

  }

  //no player alive
  GameOver();

}

void AC_GameModeBase::GameOver()
{
  EndWave();

  // @TODO: Finish up the match, present 'game over' to players

  UE_LOG(LogTemp, Log, TEXT("GAME OVER! Players Died"));


  SetWaveState(EWaveState::GameOver);

}

void AC_GameModeBase::SetWaveState(EWaveState NewState)
{
  AC_GameStateBase* GS = GetGameState<AC_GameStateBase>();
  if (ensureAlways(GS)) {
    GS->SetWaveState(NewState);
  }

}

void AC_GameModeBase::StartPlay()
{

  Super::StartPlay();

  PrepareForNextWave();

}

void AC_GameModeBase::Tick(float DeltaSeconds)
{
  Super::Tick(DeltaSeconds);

  CheckWaveState();
  CheckAnyPlayerAlive();
}

void AC_GameModeBase::SpawnBotTimerElapsed()
{

  SpawnNewBot();

  --NrOfBotsToSpawn;

  if (NrOfBotsToSpawn <= 0) {
    EndWave();
  }



}

