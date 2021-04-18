// Fill out your copyright notice in the Description page of Project Settings.


#include "C_TargetDummy.h"

// Sets default values
AC_TargetDummy::AC_TargetDummy()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AC_TargetDummy::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AC_TargetDummy::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

