// Fill out your copyright notice in the Description page of Project Settings.


#include "C_GameStateBase.h"
#include "Net/UnrealNetwork.h"

void AC_GameStateBase::OnRep_WaveState(EWaveState OldState)
{
  WaveStateChanged(WaveState, OldState);
}


void AC_GameStateBase::SetWaveState(EWaveState NewState)
{

  if (HasAuthority()) {
    EWaveState OldState = WaveState;
    WaveState = NewState;
    OnRep_WaveState(OldState);
  }


}

void AC_GameStateBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
  Super::GetLifetimeReplicatedProps(OutLifetimeProps);

  DOREPLIFETIME(AC_GameStateBase, WaveState);


}